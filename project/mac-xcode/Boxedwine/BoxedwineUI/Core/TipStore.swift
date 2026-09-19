// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

#if BOXEDWINE_APP_STORE
import AppKit
import Combine
import Foundation
import StoreKit

public struct TipOption: Identifiable, Equatable, Sendable {
    public let id: String
    public let name: String
    public let displayPrice: String
}

/// Only verified StoreKit transactions reach this type. Tips have no entitlement
/// to restore or persist; Apple keeps the payment history.
struct TipTransaction: Sendable {
    let id: UInt64
    let productID: String
    let revoked: Bool
    let finish: @Sendable () async -> Void
}

enum TipPurchaseResult {
    case success(TipTransaction), pending, cancelled
}

enum TipUpdate: Sendable {
    case transaction(TipTransaction), verificationFailed
}

@MainActor
protocol TipPurchasing {
    var canMakePayments: Bool { get }
    func loadProducts() async throws -> [TipOption]
    func purchase(_ id: String) async throws -> TipPurchaseResult
    func updates() -> AsyncStream<TipUpdate>
}

@MainActor
public final class TipStore: ObservableObject {
    public static let productIDs = [
        "org.boxedwine.app.tip.small",
        "org.boxedwine.app.tip.medium",
        "org.boxedwine.app.tip.large"
    ]
    @Published public private(set) var options: [TipOption] = []
    @Published public private(set) var loading = false
    @Published public private(set) var purchasingID: String?
    @Published public private(set) var pendingProductIDs: Set<String> = []
    @Published public private(set) var message: String?
    @Published public private(set) var errorMessage: String?
    @Published public private(set) var paymentsAllowed = true
    private let client: any TipPurchasing
    private var listener: Task<Void, Never>?
    private var handledTransactions: Set<UInt64> = []
    private var thankedTransaction: UInt64?

    public convenience init() { self.init(client: StoreKitTipClient()) }

    init(client: any TipPurchasing) {
        self.client = client
        startListening()
    }

    public func startListening() {
        guard listener == nil else { return }
        // Start at app launch, not when the support window opens. StoreKit also
        // delivers unfinished purchases here after an interrupted app session.
        let updates = client.updates()
        listener = Task { [weak self] in
            for await update in updates {
                guard !Task.isCancelled else { break }
                await self?.receive(update)
            }
        }
    }

    deinit { listener?.cancel() }

    public func load() async {
        guard !loading, purchasingID == nil else { return }
        loading = true
        defer { loading = false }
        errorMessage = nil
        paymentsAllowed = client.canMakePayments
        guard paymentsAllowed else {
            options = []
            errorMessage = "In-app purchases aren’t allowed on this Mac. All Boxedwine features remain available."
            return
        }
        do {
            let products = try await client.loadProducts()
            try Task.checkCancellation()
            // Show only configured tips, in a stable order. Prices and names are
            // localized by Apple, never reconstructed from a hard-coded amount.
            options = Self.productIDs.compactMap { id in products.first { $0.id == id } }
            if options.isEmpty {
                errorMessage = "Tips are currently unavailable. Please try again later."
            }
        } catch is CancellationError {
            // Closing the window while loading is not a purchase failure.
        } catch {
            options = []
            errorMessage = "Couldn’t load tips from the App Store. Check your connection and try again."
            NSLog("Boxedwine: unable to load StoreKit tip products.")
        }
    }

    public func purchase(_ id: String) async {
        guard !loading, purchasingID == nil, !pendingProductIDs.contains(id),
              options.contains(where: { $0.id == id }) else { return }
        paymentsAllowed = client.canMakePayments
        guard paymentsAllowed else {
            errorMessage = "In-app purchases aren’t allowed on this Mac. All Boxedwine features remain available."
            return
        }
        purchasingID = id
        message = nil
        errorMessage = nil
        defer { purchasingID = nil }
        do {
            switch try await client.purchase(id) {
            case .success(let transaction): await receive(.transaction(transaction))
            case .pending: pendingProductIDs.insert(id)
            case .cancelled: break
            }
        } catch is CancellationError {
            // StoreKit's update listener still handles a purchase that completes
            // after this task is cancelled.
        } catch {
            errorMessage = "Couldn’t confirm your tip. Check your App Store purchase history before trying again."
            NSLog("Boxedwine: unable to confirm a StoreKit tip purchase.")
        }
    }

    func receive(_ update: TipUpdate) async {
        switch update {
        case .verificationFailed:
            errorMessage = "Couldn’t verify a tip with the App Store. Check your App Store purchase history before trying again."
            NSLog("Boxedwine: StoreKit tip verification failed.")
        case .transaction(let transaction):
            guard Self.productIDs.contains(transaction.productID) else { return }
            pendingProductIDs.remove(transaction.productID)
            if transaction.revoked {
                if thankedTransaction == transaction.id { message = nil }
                await transaction.finish()
                return
            }
            // Purchase results and the update listener can deliver the same
            // transaction. Claim it before awaiting finish to prevent duplicates.
            guard handledTransactions.insert(transaction.id).inserted else { return }
            thankedTransaction = transaction.id
            errorMessage = nil
            message = "Thank you for supporting Boxedwine!"
            await transaction.finish()
        }
    }
}

@MainActor
private final class StoreKitTipClient: TipPurchasing {
    private var products: [String: Product] = [:]
    var canMakePayments: Bool { AppStore.canMakePayments }

    func loadProducts() async throws -> [TipOption] {
        let loaded = try await Product.products(for: TipStore.productIDs)
            .filter { $0.type == .consumable }
        products = Dictionary(uniqueKeysWithValues: loaded.map { ($0.id, $0) })
        return loaded.map { TipOption(id: $0.id, name: $0.displayName, displayPrice: $0.displayPrice) }
    }

    func purchase(_ id: String) async throws -> TipPurchaseResult {
        guard let product = products[id] else { throw TipError.unavailable }
        let result: Product.PurchaseResult
        if #available(macOS 15.2, *), let window = NSApplication.shared.keyWindow {
            result = try await product.purchase(confirmIn: window)
        } else {
            result = try await product.purchase()
        }
        switch result {
        case .success(let verification):
            guard let transaction = try Self.verified(verification) else { throw TipError.unavailable }
            return .success(transaction)
        case .pending: return .pending
        case .userCancelled: return .cancelled
        @unknown default: throw TipError.unavailable
        }
    }

    func updates() -> AsyncStream<TipUpdate> {
        AsyncStream { continuation in
            let task = Task {
                for await verification in StoreKit.Transaction.updates {
                    guard !Task.isCancelled else { break }
                    do {
                        if let transaction = try Self.verified(verification) {
                            continuation.yield(.transaction(transaction))
                        }
                    } catch { continuation.yield(.verificationFailed) }
                }
                continuation.finish()
            }
            continuation.onTermination = { _ in task.cancel() }
        }
    }

    private static func verified(_ result: VerificationResult<StoreKit.Transaction>) throws -> TipTransaction? {
        // Do not acknowledge unrelated or unverified transactions. StoreKit may
        // redeliver a transaction once it can verify its signature.
        guard TipStore.productIDs.contains(result.unsafePayloadValue.productID) else { return nil }
        guard case .verified(let transaction) = result else { throw TipError.verification }
        guard transaction.productType == .consumable else { return nil }
        return TipTransaction(id: transaction.id, productID: transaction.productID,
                              revoked: transaction.revocationDate != nil,
                              finish: { await transaction.finish() })
    }

    private enum TipError: Error { case unavailable, verification }
}
#endif
