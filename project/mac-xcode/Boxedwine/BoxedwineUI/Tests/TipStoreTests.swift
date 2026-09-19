// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

#if BOXEDWINE_APP_STORE
import Foundation
import Testing
@testable import BoxedwineLibrary

@MainActor
struct TipStoreTests {
    private let small = TipStore.productIDs[0]

    private final class Client: TipPurchasing {
        var canMakePayments = true
        var products = TipStore.productIDs.map { TipOption(id: $0, name: $0, displayPrice: "€4,99") }
        var loadFails = false
        var purchaseCalls: [String] = []
        var outcome: () async throws -> TipPurchaseResult = { .cancelled }
        let stream: AsyncStream<TipUpdate>
        let continuation: AsyncStream<TipUpdate>.Continuation
        init() {
            (stream, continuation) = AsyncStream.makeStream()
        }
        func loadProducts() async throws -> [TipOption] {
            if loadFails { throw CocoaError(.fileReadUnknown) }
            return products
        }
        func purchase(_ id: String) async throws -> TipPurchaseResult {
            purchaseCalls.append(id)
            return try await outcome()
        }
        func updates() -> AsyncStream<TipUpdate> { stream }
    }

    private actor Finishes {
        var count = 0
        func finish() { count += 1 }
    }

    @Test func pricesComeFromAppleAndUnknownProductsAreExcluded() async {
        let client = Client()
        client.products.reverse()
        client.products.append(TipOption(id: "unrelated", name: "Unrelated", displayPrice: "$99"))
        let store = TipStore(client: client)
        await store.load()
        #expect(store.options.map(\.id) == TipStore.productIDs)
        #expect(store.options.allSatisfy { $0.displayPrice == "€4,99" })
        await store.purchase("unrelated")
        #expect(client.purchaseCalls.isEmpty)
    }

    @Test func missingProductsAndNetworkFailureCanBeRetried() async {
        let client = Client()
        client.products = []
        let store = TipStore(client: client)
        await store.load()
        #expect(store.options.isEmpty && store.errorMessage != nil && !store.loading)
        client.loadFails = true
        await store.load()
        #expect(store.errorMessage?.contains("connection") == true)
        client.loadFails = false
        client.products = [TipOption(id: small, name: "Small Tip", displayPrice: "$2.99")]
        await store.load()
        #expect(store.options.count == 1 && store.errorMessage == nil)
    }

    @Test func paymentRestrictionsAreRecheckedBeforePurchase() async {
        let client = Client()
        let store = TipStore(client: client)
        await store.load()
        client.canMakePayments = false
        await store.purchase(small)
        #expect(client.purchaseCalls.isEmpty && !store.paymentsAllowed)
        await store.load()
        #expect(store.options.isEmpty)
    }

    @Test func cancellationIsQuietAndFailureDoesNotClaimSuccess() async {
        let client = Client()
        let store = TipStore(client: client)
        await store.load()
        await store.purchase(small)
        #expect(store.errorMessage == nil && store.message == nil && store.purchasingID == nil)
        client.outcome = { throw CocoaError(.fileReadUnknown) }
        await store.purchase(small)
        #expect(store.errorMessage != nil && store.message == nil && store.purchasingID == nil)
    }

    @Test func simultaneousClicksStartOnlyOnePurchase() async {
        let client = Client()
        let store = TipStore(client: client)
        await store.load()
        var finish: CheckedContinuation<TipPurchaseResult, Never>?
        let purchase = await withCheckedContinuation { (started: CheckedContinuation<Task<Void, Never>, Never>) in
            client.outcome = {
                await withCheckedContinuation { continuation in
                    finish = continuation
                    started.resume(returning: Task { await store.purchase(self.small) })
                }
            }
            Task { await store.purchase(small) }
        }
        await purchase.value
        #expect(client.purchaseCalls == [small])
        #expect(store.purchasingID == small)
        finish?.resume(returning: .cancelled)
    }

    @Test func pendingApprovalPreventsDuplicatePurchaseAndFinishesLater() async {
        let client = Client()
        client.outcome = { .pending }
        let store = TipStore(client: client)
        await store.load()
        await store.purchase(small)
        await store.purchase(small)
        #expect(client.purchaseCalls == [small])
        #expect(store.message == nil && store.pendingProductIDs == [small])
        let finishes = Finishes()
        await store.receive(.transaction(TipTransaction(id: 1, productID: small, revoked: false,
                                                       finish: { await finishes.finish() })))
        #expect(store.pendingProductIDs.isEmpty && store.message != nil)
        #expect(await finishes.count == 1)
    }

    @Test func duplicateDeliveryFinishesOnceAndTipsAreRepeatable() async {
        let client = Client()
        let finishes = Finishes()
        let transaction = TipTransaction(id: 2, productID: small, revoked: false, finish: { await finishes.finish() })
        client.outcome = { .success(transaction) }
        let store = TipStore(client: client)
        await store.load()
        await store.purchase(small)
        await store.receive(.transaction(transaction))
        #expect(await finishes.count == 1)
        client.outcome = { .success(TipTransaction(id: 3, productID: self.small, revoked: false, finish: { await finishes.finish() })) }
        await store.purchase(small)
        #expect(client.purchaseCalls == [small, small])
        #expect(await finishes.count == 2)
    }

    @Test func unverifiedAndUnrelatedTransactionsNeverThankOrFinish() async {
        let finishes = Finishes()
        let store = TipStore(client: Client())
        await store.receive(.verificationFailed)
        #expect(store.message == nil && store.errorMessage != nil)
        await store.receive(.transaction(TipTransaction(id: 4, productID: "unrelated", revoked: false,
                                                       finish: { await finishes.finish() })))
        #expect(store.message == nil && store.options.isEmpty)
        #expect(await finishes.count == 0)
    }

    @Test func refundsRemoveThankYouWithoutAffectingAppFeatures() async {
        let store = TipStore(client: Client())
        await store.receive(.transaction(TipTransaction(id: 5, productID: small, revoked: false, finish: {})))
        #expect(store.message != nil)
        await store.receive(.transaction(TipTransaction(id: 5, productID: small, revoked: true, finish: {})))
        #expect(store.message == nil)
    }

    @Test func listenerHandlesUnfinishedPurchaseWithoutOpeningSupport() async {
        let client = Client()
        let store = TipStore(client: client)
        await withCheckedContinuation { (finished: CheckedContinuation<Void, Never>) in
            client.continuation.yield(.transaction(TipTransaction(id: 6, productID: small, revoked: false,
                                                                  finish: { finished.resume() })))
        }
        #expect(store.message != nil && store.options.isEmpty)
    }
}
#endif
