// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

#if BOXEDWINE_APP_STORE
import SwiftUI

struct SupportView: View {
    @ObservedObject var tips: TipStore

    var body: some View {
        VStack(spacing: 20) {
            Image(systemName: "heart.circle.fill")
                .font(.system(size: 48)).foregroundStyle(.tint).accessibilityHidden(true)
            VStack(spacing: 8) {
                Text("Support Boxedwine").font(.title2.bold()).accessibilityAddTraits(.isHeader)
                Text("Boxedwine is free and open source. Optional tips help cover development and project expenses.")
                Text("All features are available without tipping.").foregroundStyle(.secondary)
            }
            if tips.loading {
                ProgressView("Loading tips…").controlSize(.small)
            } else if !tips.options.isEmpty {
                VStack(spacing: 10) {
                    ForEach(tips.options) { option in
                        Button {
                            Task { await tips.purchase(option.id) }
                        } label: {
                            HStack {
                                Text(option.name)
                                Spacer()
                                if tips.purchasingID == option.id {
                                    ProgressView().controlSize(.small)
                                } else {
                                    Text(option.displayPrice).fontWeight(.semibold)
                                }
                            }.padding(.horizontal, 8).padding(.vertical, 6)
                        }
                        .disabled(tips.purchasingID != nil || tips.pendingProductIDs.contains(option.id) || !tips.paymentsAllowed)
                        .accessibilityLabel("\(option.name), \(option.displayPrice), one-time tip")
                    }
                }
                Text("One-time tips, paid through the App Store. No subscription.")
                    .font(.footnote).foregroundStyle(.secondary)
            }
            if tips.purchasingID != nil {
                Text("Complete your purchase in the App Store confirmation window.")
                    .font(.callout).foregroundStyle(.secondary)
            }
            if !tips.pendingProductIDs.isEmpty {
                Label("Your tip is awaiting approval. There’s no need to purchase it again.", systemImage: "clock")
                    .font(.callout).foregroundStyle(.secondary)
            }
            if let message = tips.message {
                Label(message, systemImage: "checkmark.circle.fill").foregroundStyle(.green)
            }
            if let error = tips.errorMessage {
                Text(error).font(.callout).foregroundStyle(.secondary)
            }
            if tips.purchasingID == nil && !tips.loading && (tips.errorMessage != nil || tips.options.count < TipStore.productIDs.count) {
                Button("Reload Tips") { Task { await tips.load() } }
            }
        }
        .multilineTextAlignment(.center)
        .padding(28)
        .frame(width: 420)
        .fixedSize(horizontal: false, vertical: true)
        .task { await tips.load() }
    }
}
#endif
