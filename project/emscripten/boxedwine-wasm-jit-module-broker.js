/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

(function() {
    'use strict';

    var LOOKUP_LOCAL_COMPILE = 1;
    var LOOKUP_BROKER_HIT = 2;
    var PRELOAD_MODULE_LIMIT = 64;
    var MODULE_CLASS_STANDALONE = 1;
    var MODULE_CLASS_GROUPED = 2;
    var localModules = new Map();
    var mainModules = new Map();
    var mainModulesByOwner = new Map();
    var preloadBlockedWorkersByOwner = new Map();
    var publicationBlockedOwners = new Set();
    var freshGroupedConstructionBlockedOwners = new Set();
    var publicationSequence = 0;
    var localStats = {
        localCompiles: 0,
        brokerHits: 0,
        receivedCacheEntries: 0,
        deliveryFailures: 0,
        purgedModules: 0,
        byOwner: new Map()
    };
    var mainStats = {
        firstPublications: 0,
        groupedPublications: 0,
        duplicatePublications: 0,
        ownershipErrors: 0,
        unknownModuleIds: 0,
        scheduledDeliveries: 0,
        deliveryFailures: 0,
        purgedModules: 0,
        preloadAttempts: 0,
        preloadCandidates: 0,
        preloadSent: 0,
        preloadSkippedHeld: 0,
        preloadFailures: 0,
        preloadOwnerMisses: 0,
        preloadOomBlocks: 0,
        staleIncarnationDrops: 0,
        preloadModuleLimit: PRELOAD_MODULE_LIMIT,
        byOwner: new Map()
    };
    var forcedDeliveryFailureIds = new Set();
    var forcedDeliveryOomIds = new Set();
    var forcedCompileOomIds = new Set();
    var forcedTestThreadStartOwners = new Map();
    var dropNextTestRun = false;
    var delayedTestPublicationIds = new Set();
    var testTrackedPurgeOwners = new Set();
    var testPurgeMessagesByOwner = new Map();
    var workerStatsToken = (typeof ENVIRONMENT_IS_PTHREAD !== 'undefined' && ENVIRONMENT_IS_PTHREAD ? 'worker-' : 'main-') +
        Date.now().toString(36) + '-' + Math.random().toString(36).slice(2);
    var allocatedWorkerStatsTokens = new WeakMap();
    var nextAllocatedWorkerStatsToken = 1;
    var nextStatsRequestId = 1;
    var pendingStatsRequests = new Map();
    var testRunDispatchStats = {
        runMessages: 0,
        preloadedRunMessages: 0,
        preloadModulesBeforeRun: 0
    };
    var proxyMainBootstrapOwnerPending = false;

    function moduleKey(moduleId) {
        return moduleId >>> 0;
    }

    globalThis.bwWasmJitMtGroupInstanceKey = function(moduleId, groupKind, groupIdentity,
            memoryId, memoryIncarnation) {
        if (moduleId) {
            return 'm' + (moduleId >>> 0);
        }
        return (groupKind >>> 0) + '|' + String(groupIdentity) + '|' +
            (memoryId >>> 0).toString(16) + '|' +
            (memoryIncarnation >>> 0).toString(16);
    };

    globalThis.bwWasmJitMtRecordGroupWorkerEvent = function(event) {
        if (typeof Module._wasm_jit_mt_record_group_worker_event === 'function') {
            Module._wasm_jit_mt_record_group_worker_event(event >>> 0);
        }
    };

    function ownerKey(memoryId, memoryIncarnation) {
        return (memoryId >>> 0) + ':' + (memoryIncarnation >>> 0);
    }

    function isWasmOom(error) {
        if (!error || error.name === 'CompileError' ||
                (typeof WebAssembly.CompileError === 'function' &&
                    error instanceof WebAssembly.CompileError)) {
            return false;
        }
        var text = String(error && (error.message || error));
        return /out of memory|\boom\b/i.test(text);
    }

    globalThis.bwWasmJitBrokerIsOom = isWasmOom;

    globalThis.bwWasmJitMtCanConstructFreshGroup = function(memoryId, memoryIncarnation) {
        return !freshGroupedConstructionBlockedOwners.has(
            ownerKey(memoryId, memoryIncarnation));
    };

    globalThis.bwWasmJitMtBlockFreshGroupConstruction = function(memoryId, memoryIncarnation) {
        freshGroupedConstructionBlockedOwners.add(
            ownerKey(memoryId, memoryIncarnation));
    };

    function runtimeGroupConstructorStats() {
        if (!globalThis.bwWasmJitMtRuntimeGroupConstructorStats) {
            globalThis.bwWasmJitMtRuntimeGroupConstructorStats = {
                moduleAttempts: 0,
                instanceAttempts: 0
            };
        }
        return globalThis.bwWasmJitMtRuntimeGroupConstructorStats;
    }

    globalThis.bwWasmJitMtTestRecordInstanceConstructor = function() {
        runtimeGroupConstructorStats().instanceAttempts += 1;
    };

    Module['getWasmJitMtRuntimeBatchStats'] = function() {
        var snapshotSize = 192;
        var snapshot = Module._malloc(snapshotSize);
        if (!snapshot) {
            throw new Error('could not allocate MT WASM runtime batch snapshot');
        }
        try {
            Module._wasm_jit_mt_copy_runtime_batch_stats(snapshot);
            var base = snapshot >>> 2;
            var readU64 = function(byteOffset) {
                var index = base + (byteOffset >> 2);
                var low = HEAPU32[index] >>> 0;
                var high = HEAPU32[index + 1] >>> 0;
                // Diagnostic Numbers above Number.MAX_SAFE_INTEGER lose integer precision.
                return low + high * 0x100000000;
            };
            var groupedModules = readU64(40);
            var groupedFunctions = readU64(48);
            return {
                translatedFileBacked: readU64(0),
                translatedAnonymous: readU64(8),
                pendingEntries: readU64(16),
                openBytes: readU64(24),
                sealedGroups: readU64(32),
                groupedModules: groupedModules,
                groupedFunctions: groupedFunctions,
                constructionAttempts: readU64(56),
                constructionSuccesses: readU64(64),
                maxFunctionsPerGroup: readU64(72),
                countFlushes: readU64(80),
                byteFlushes: readU64(88),
                urgentFlushes: readU64(96),
                processCapFlushes: readU64(104),
                cancelledEntries: readU64(112),
                permanentFailures: readU64(120),
                groupedOomBlocks: readU64(128),
                skippedConstructionAttempts: readU64(136),
                groupInstanceCreations: readU64(144),
                groupInstanceReuses: readU64(152),
                groupedBrokerHits: readU64(160),
                groupedLocalCompiles: readU64(168),
                maxBlocks: HEAPU32[base + 44] >>> 0,
                maxBatchBytes: HEAPU32[base + 45] >>> 0,
                urgentPendingHits: HEAPU32[base + 46] >>> 0,
                maxProcessOpenBytes: HEAPU32[base + 47] >>> 0,
                averageFunctionsPerGroup:
                    groupedModules ? groupedFunctions / groupedModules : 0
            };
        } finally {
            Module._free(snapshot);
        }
    };

    function createOwnerStats(memoryId, memoryIncarnation) {
        return {
            memoryId: memoryId >>> 0,
            memoryIncarnation: memoryIncarnation >>> 0,
            localCompiles: 0,
            brokerHits: 0,
            receivedCacheEntries: 0,
            liveWorkerCacheModules: 0,
            firstPublications: 0,
            duplicatePublications: 0,
            scheduledDeliveries: 0,
            deliveryFailures: 0,
            purgedModules: 0,
            preloadAttempts: 0,
            preloadCandidates: 0,
            preloadSent: 0,
            preloadSkippedHeld: 0,
            preloadFailures: 0,
            preloadOwnerMisses: 0,
            preloadOomBlocks: 0,
            staleIncarnationDrops: 0,
            preloadModuleLimit: PRELOAD_MODULE_LIMIT
        };
    }

    function statsForOwner(map, memoryId, memoryIncarnation) {
        var key = ownerKey(memoryId, memoryIncarnation);
        var stats = map.get(key);
        if (!stats) {
            stats = createOwnerStats(memoryId, memoryIncarnation);
            map.set(key, stats);
        }
        return stats;
    }

    function snapshotLocalStats() {
        var rows = new Map();
        var groupStats = globalThis.bwWasmJitMtGroupWorkerStats || {};
        localStats.byOwner.forEach(function(stats, key) {
            rows.set(key, Object.assign({}, stats));
        });
        localModules.forEach(function(entry) {
            statsForOwner(rows, entry.memoryId, entry.memoryIncarnation).liveWorkerCacheModules += 1;
        });
        return {
            localCompiles: localStats.localCompiles,
            brokerHits: localStats.brokerHits,
            receivedCacheEntries: localStats.receivedCacheEntries,
            deliveryFailures: localStats.deliveryFailures,
            liveWorkerCacheModules: localModules.size,
            purgedModules: localStats.purgedModules,
            groupInstanceCreations: groupStats.groupInstanceCreations || 0,
            groupInstanceReuses: groupStats.groupInstanceReuses || 0,
            groupedBrokerHits: groupStats.groupedBrokerHits || 0,
            groupedLocalCompiles: groupStats.groupedLocalCompiles || 0,
            byOwner: Array.from(rows.values())
        };
    }

    globalThis.bwWasmJitBrokerTestLocalSnapshot = snapshotLocalStats;

    function allocatedWorkers() {
        var workers = [];
        var seen = new Set();
        if (typeof PThread === 'undefined') {
            return workers;
        }
        PThread.runningWorkers.concat(PThread.unusedWorkers).forEach(function(worker) {
            if (worker && !seen.has(worker)) {
                seen.add(worker);
                workers.push(worker);
            }
        });
        return workers;
    }

    function statsTokenForAllocatedWorker(worker) {
        var token = allocatedWorkerStatsTokens.get(worker);
        if (!token) {
            token = 'allocated-worker-' + nextAllocatedWorkerStatsToken++;
            allocatedWorkerStatsTokens.set(worker, token);
        }
        return token;
    }

    function publisherWorker(pthread) {
        if (!pthread || typeof PThread === 'undefined') {
            return null;
        }
        return PThread.pthreads[pthread] || null;
    }

    function postToMain(message) {
        if (typeof postMessage === 'function') {
            postMessage(message);
            return;
        }
        if (typeof worker_threads !== 'undefined' && worker_threads.parentPort) {
            worker_threads.parentPort.postMessage(message);
            return;
        }
        throw new Error('WASM JIT broker cannot reach the main thread');
    }

    function recordHolder(entry, worker) {
        if (!worker) {
            return;
        }
        if (!worker.bwWasmJitBrokerHeldIds) {
            worker.bwWasmJitBrokerHeldIds = new Set();
        }
        worker.bwWasmJitBrokerHeldIds.add(entry.moduleId);
        entry.holders.add(worker);
    }

    function publishOnMain(moduleId, memoryId, memoryIncarnation, module, sourcePthread,
            moduleClass, representedBlockCount) {
        moduleId = moduleKey(moduleId);
        memoryId = memoryId >>> 0;
        memoryIncarnation = memoryIncarnation >>> 0;
        moduleClass = moduleClass === MODULE_CLASS_GROUPED ? MODULE_CLASS_GROUPED : MODULE_CLASS_STANDALONE;
        representedBlockCount = representedBlockCount >>> 0;
        if (!representedBlockCount) {
            representedBlockCount = 1;
        }
        if (!moduleId || !(module instanceof WebAssembly.Module)) {
            return;
        }
        if (typeof Module._wasm_jit_mt_broker_validate_module === 'function') {
            var validation = Module._wasm_jit_mt_broker_validate_module(moduleId, memoryId, memoryIncarnation);
            if (validation === -1) {
                mainStats.ownershipErrors += 1;
                return;
            }
            if (validation === -2) {
                mainStats.staleIncarnationDrops += 1;
                var staleSourceWorker = publisherWorker(sourcePthread);
                if (staleSourceWorker) {
                    try {
                        var stalePost = staleSourceWorker.bwWasmJitBrokerOriginalPostMessage ||
                            staleSourceWorker.postMessage.bind(staleSourceWorker);
                        stalePost({
                            bwWasmJitModuleBroker: {
                                type: 'purge',
                                memoryId: memoryId,
                                memoryIncarnation: memoryIncarnation,
                                moduleIds: [moduleId]
                            }
                        });
                    } catch (error) {
                        mainStats.deliveryFailures += 1;
                    }
                }
                return;
            }
            if (validation === 0) {
                mainStats.unknownModuleIds += 1;
                return;
            }
        }
        var current = mainModules.get(moduleId);
        if (current) {
            if (current.memoryId === memoryId && current.memoryIncarnation === memoryIncarnation) {
                mainStats.duplicatePublications += 1;
                statsForOwner(mainStats.byOwner, memoryId, memoryIncarnation).duplicatePublications += 1;
                current.duplicatePublications += 1;
                current.publishSequence = ++publicationSequence;
                recordHolder(current, publisherWorker(sourcePthread));
            } else {
                mainStats.ownershipErrors += 1;
            }
            return;
        }
        var entry = {
            moduleId: moduleId,
            memoryId: memoryId,
            memoryIncarnation: memoryIncarnation,
            module: module,
            moduleClass: moduleClass,
            representedBlockCount: representedBlockCount,
            duplicatePublications: 0,
            publishSequence: ++publicationSequence,
            holders: new Set()
        };
        mainModules.set(moduleId, entry);
        var key = ownerKey(memoryId, memoryIncarnation);
        var ownerModules = mainModulesByOwner.get(key);
        if (!ownerModules) {
            ownerModules = new Map();
            mainModulesByOwner.set(key, ownerModules);
        }
        ownerModules.set(moduleId, entry);
        recordHolder(entry, publisherWorker(sourcePthread));
        mainStats.firstPublications += 1;
        if (moduleClass === MODULE_CLASS_GROUPED) {
            mainStats.groupedPublications += 1;
        }
        statsForOwner(mainStats.byOwner, memoryId, memoryIncarnation).firstPublications += 1;
    }

    function publish(moduleId, memoryId, memoryIncarnation, module, moduleClass, representedBlockCount) {
        if (delayedTestPublicationIds.has(moduleKey(moduleId))) {
            return true;
        }
        if (typeof ENVIRONMENT_IS_PTHREAD !== 'undefined' && ENVIRONMENT_IS_PTHREAD) {
            try {
                postToMain({
                    cmd: 'callHandler',
                    handler: 'bwWasmJitBrokerPublish',
                    args: [moduleId, memoryId, memoryIncarnation, module,
                        typeof _pthread_self === 'function' ? _pthread_self() : 0,
                        moduleClass, representedBlockCount]
                });
                return true;
            } catch (error) {
                localStats.deliveryFailures += 1;
                statsForOwner(localStats.byOwner, memoryId, memoryIncarnation).deliveryFailures += 1;
                return false;
            }
        } else {
            publishOnMain(moduleId, memoryId, memoryIncarnation, module, 0, moduleClass, representedBlockCount);
            return true;
        }
    }

    function comparePreloadCandidates(left, right) {
        var leftTier = left.moduleClass === MODULE_CLASS_GROUPED ? 0 :
            (left.duplicatePublications > 0 ? 1 : 2);
        var rightTier = right.moduleClass === MODULE_CLASS_GROUPED ? 0 :
            (right.duplicatePublications > 0 ? 1 : 2);
        if (leftTier !== rightTier) {
            return leftTier - rightTier;
        }
        if (leftTier === 0 && left.representedBlockCount !== right.representedBlockCount) {
            return right.representedBlockCount - left.representedBlockCount;
        }
        if (leftTier === 1 && left.duplicatePublications !== right.duplicatePublications) {
            return right.duplicatePublications - left.duplicatePublications;
        }
        return right.publishSequence - left.publishSequence;
    }

    function preloadOwnerIntoWorker(worker, originalPostMessage, memoryId, memoryIncarnation) {
        var key = ownerKey(memoryId, memoryIncarnation);
        var ownerModules = mainModulesByOwner.get(key);
        var ownerStats = statsForOwner(mainStats.byOwner, memoryId, memoryIncarnation);
        mainStats.preloadAttempts += 1;
        ownerStats.preloadAttempts += 1;
        if (!ownerModules || !ownerModules.size) {
            return {sent: 0, error: null};
        }

        if (!worker.bwWasmJitBrokerHeldIds) {
            worker.bwWasmJitBrokerHeldIds = new Set();
        }
        var candidates = Array.from(ownerModules.values());
        mainStats.preloadCandidates += candidates.length;
        ownerStats.preloadCandidates += candidates.length;
        var missing = candidates.filter(function(entry) {
            if (worker.bwWasmJitBrokerHeldIds.has(entry.moduleId)) {
                mainStats.preloadSkippedHeld += 1;
                ownerStats.preloadSkippedHeld += 1;
                return false;
            }
            return true;
        });
        missing.sort(comparePreloadCandidates);

        var sent = 0;
        for (var i = 0; i < missing.length && sent < PRELOAD_MODULE_LIMIT; i++) {
            var entry = missing[i];
            try {
                if (forcedDeliveryOomIds.has(entry.moduleId)) {
                    throw new RangeError('out of memory');
                }
                if (forcedDeliveryFailureIds.has(entry.moduleId)) {
                    throw new Error('forced preload failure');
                }
                originalPostMessage({
                    bwWasmJitModuleBroker: {
                        type: 'module',
                        moduleId: entry.moduleId,
                        memoryId: entry.memoryId,
                        memoryIncarnation: entry.memoryIncarnation,
                        module: entry.module
                    }
                });
            } catch (error) {
                return {sent: sent, error: error};
            }
            worker.bwWasmJitBrokerHeldIds.add(entry.moduleId);
            entry.holders.add(worker);
            sent += 1;
            mainStats.scheduledDeliveries += 1;
            mainStats.preloadSent += 1;
            ownerStats.scheduledDeliveries += 1;
            ownerStats.preloadSent += 1;
        }
        return {sent: sent, error: null};
    }

    globalThis.bwWasmJitBrokerGetOrCompile = function(moduleId, memoryId, memoryIncarnation, bytes, enabled,
            moduleClass, representedBlockCount) {
        moduleId = moduleKey(moduleId);
        memoryId = memoryId >>> 0;
        memoryIncarnation = memoryIncarnation >>> 0;
        moduleClass = moduleClass >>> 0;
        representedBlockCount = representedBlockCount >>> 0;
        var key = ownerKey(memoryId, memoryIncarnation);
        var brokerAllowed = !!enabled && !!moduleId && !!memoryIncarnation;
        if (brokerAllowed) {
            var cached = localModules.get(moduleId);
            if (cached && cached.memoryId === memoryId && cached.memoryIncarnation === memoryIncarnation) {
                localStats.brokerHits += 1;
                statsForOwner(localStats.byOwner, memoryId, memoryIncarnation).brokerHits += 1;
                return {module: cached.module, source: LOOKUP_BROKER_HIT};
            }
            if (cached) {
                brokerAllowed = false;
            }
        }
        var module;
        try {
            if (moduleClass === MODULE_CLASS_GROUPED) {
                runtimeGroupConstructorStats().moduleAttempts += 1;
            }
            if (forcedCompileOomIds.has(moduleId)) {
                throw new RangeError('out of memory');
            }
            module = new WebAssembly.Module(bytes);
        } catch (error) {
            if (isWasmOom(error)) {
                error.bwWasmJitBrokerOom = true;
                if (brokerAllowed) {
                    publicationBlockedOwners.add(key);
                }
            }
            throw error;
        }
        localStats.localCompiles += 1;
        statsForOwner(localStats.byOwner, memoryId, memoryIncarnation).localCompiles += 1;
        if (brokerAllowed && !publicationBlockedOwners.has(key)) {
            localModules.set(moduleId, {
                memoryId: memoryId,
                memoryIncarnation: memoryIncarnation,
                module: module
            });
            publish(moduleId, memoryId, memoryIncarnation, module, moduleClass, representedBlockCount);
        }
        return {module: module, source: LOOKUP_LOCAL_COMPILE};
    };

    globalThis.bwWasmJitBrokerTestSetPublicationDelayed = function(moduleId, delayed) {
        moduleId = moduleKey(moduleId);
        if (delayed) {
            delayedTestPublicationIds.add(moduleId);
        } else {
            delayedTestPublicationIds.delete(moduleId);
        }
    };

    globalThis.bwWasmJitBrokerTestSetCompileOom = function(moduleId, enabled) {
        moduleId = moduleKey(moduleId);
        if (enabled) {
            forcedCompileOomIds.add(moduleId);
        } else {
            forcedCompileOomIds.delete(moduleId);
        }
    };

    globalThis.bwWasmJitBrokerTestGetLocalModule = function(moduleId, memoryId, memoryIncarnation) {
        var entry = localModules.get(moduleKey(moduleId));
        if (entry && entry.memoryId === (memoryId >>> 0) &&
                entry.memoryIncarnation === (memoryIncarnation >>> 0)) {
            return entry;
        }
        return null;
    };

    globalThis.bwWasmJitBrokerTestPartialPurgePublicationBlock = function(
            memoryId, memoryIncarnation, firstModuleId, secondModuleId) {
        memoryId = memoryId >>> 0;
        memoryIncarnation = memoryIncarnation >>> 0;
        firstModuleId = moduleKey(firstModuleId);
        secondModuleId = moduleKey(secondModuleId);
        var key = ownerKey(memoryId, memoryIncarnation);
        localModules.set(firstModuleId, {
            memoryId: memoryId,
            memoryIncarnation: memoryIncarnation,
            module: null
        });
        localModules.set(secondModuleId, {
            memoryId: memoryId,
            memoryIncarnation: memoryIncarnation,
            module: null
        });
        publicationBlockedOwners.add(key);

        receiveBrokerMessage({
            type: 'purge',
            memoryId: memoryId,
            memoryIncarnation: memoryIncarnation,
            moduleIds: [firstModuleId]
        });
        var result = 0;
        if (!localModules.has(firstModuleId)) {
            result |= 1;
        }
        if (localModules.has(secondModuleId)) {
            result |= 2;
        }
        if (publicationBlockedOwners.has(key)) {
            result |= 4;
        }

        receiveBrokerMessage({
            type: 'purge',
            memoryId: memoryId,
            memoryIncarnation: memoryIncarnation,
            moduleIds: [secondModuleId]
        });
        if (!localModules.has(secondModuleId)) {
            result |= 8;
        }
        if (!publicationBlockedOwners.has(key)) {
            result |= 16;
        }
        localModules.delete(firstModuleId);
        localModules.delete(secondModuleId);
        publicationBlockedOwners.delete(key);
        localStats.byOwner.delete(key);
        return result;
    };

    function purgeLocalModules(memoryId, memoryIncarnation, moduleIds) {
        memoryId = memoryId >>> 0;
        memoryIncarnation = memoryIncarnation >>> 0;
        var key = ownerKey(memoryId, memoryIncarnation);
        var exactIds = [];
        moduleIds.forEach(function(moduleId) {
            moduleId = moduleId >>> 0;
            var entry = localModules.get(moduleId);
            if (entry && entry.memoryId === memoryId &&
                    entry.memoryIncarnation === memoryIncarnation) {
                exactIds.push(moduleId);
            }
        });
        localStats.purgedModules += exactIds.length;
        var ownerStats = localStats.byOwner.get(key);
        if (ownerStats) {
            ownerStats.purgedModules += exactIds.length;
        }
        exactIds.forEach(function(moduleId) {
            localModules.delete(moduleId);
        });

        var ownerStillLive = false;
        localModules.forEach(function(entry) {
            if (entry.memoryId === memoryId &&
                    entry.memoryIncarnation === memoryIncarnation) {
                ownerStillLive = true;
            }
        });
        if (!ownerStillLive) {
            localStats.byOwner.delete(key);
            freshGroupedConstructionBlockedOwners.delete(key);
            publicationBlockedOwners.delete(key);
        }
        return exactIds.length;
    }

    function releaseMemoryOnMain(memoryId, memoryIncarnation, moduleIds) {
        memoryId = memoryId >>> 0;
        memoryIncarnation = memoryIncarnation >>> 0;
        var key = ownerKey(memoryId, memoryIncarnation);
        var ownerModules = mainModulesByOwner.get(key);
        var exactIds = [];
        var holders = new Set();
        var seen = new Set();

        moduleIds.forEach(function(moduleId) {
            moduleId = moduleId >>> 0;
            if (!moduleId || seen.has(moduleId)) {
                return;
            }
            seen.add(moduleId);
            var entry = mainModules.get(moduleId);
            if (!entry || entry.memoryId !== memoryId ||
                    entry.memoryIncarnation !== memoryIncarnation) {
                return;
            }
            exactIds.push(moduleId);
            entry.holders.forEach(function(worker) {
                holders.add(worker);
            });
        });

        var ownerStats = mainStats.byOwner.get(key);
        if (exactIds.length) {
            mainStats.purgedModules += exactIds.length;
            if (ownerStats) {
                ownerStats.purgedModules += exactIds.length;
            }
        }
        exactIds.forEach(function(moduleId) {
            mainModules.delete(moduleId);
            if (ownerModules) {
                ownerModules.delete(moduleId);
            }
        });

        holders.forEach(function(worker) {
            if (worker.bwWasmJitBrokerHeldIds) {
                exactIds.forEach(function(moduleId) {
                    worker.bwWasmJitBrokerHeldIds.delete(moduleId);
                });
            }
            try {
                var post = worker.bwWasmJitBrokerOriginalPostMessage || worker.postMessage.bind(worker);
                post({
                    bwWasmJitModuleBroker: {
                        type: 'purge',
                        memoryId: memoryId,
                        memoryIncarnation: memoryIncarnation,
                        moduleIds: exactIds
                    }
                });
            } catch (error) {
                mainStats.deliveryFailures += 1;
                if (ownerStats) {
                    ownerStats.deliveryFailures += 1;
                }
            }
        });

        if (!ownerModules || !ownerModules.size) {
            mainModulesByOwner.delete(key);
            var blockedWorkers = preloadBlockedWorkersByOwner.get(key);
            if (blockedWorkers) {
                blockedWorkers.forEach(function(worker) {
                    if (worker.bwWasmJitBrokerPreloadBlockedOwners) {
                        worker.bwWasmJitBrokerPreloadBlockedOwners.delete(key);
                    }
                });
                preloadBlockedWorkersByOwner.delete(key);
            }
            publicationBlockedOwners.delete(key);
            mainStats.byOwner.delete(key);
        }
    }

    globalThis.bwWasmJitBrokerReleaseMemory = function(memoryId, memoryIncarnation, moduleIds) {
        purgeLocalModules(memoryId, memoryIncarnation, moduleIds);
        if (typeof ENVIRONMENT_IS_PTHREAD !== 'undefined' && ENVIRONMENT_IS_PTHREAD) {
            postToMain({
                cmd: 'callHandler',
                handler: 'bwWasmJitBrokerReleaseMemory',
                args: [memoryId, memoryIncarnation, moduleIds]
            });
        } else {
            releaseMemoryOnMain(memoryId, memoryIncarnation, moduleIds);
        }
    };

    function emptyAggregate() {
        return {
            localCompiles: 0,
            brokerHits: 0,
            groupedPublications: mainStats.groupedPublications,
            groupedBrokerHits: 0,
            groupedLocalCompiles: 0,
            groupInstanceCreations: 0,
            groupInstanceReuses: 0,
            freshGroupedConstructionOomBlocks: 0,
            skippedFreshConstructionAttempts: 0,
            firstPublications: mainStats.firstPublications,
            duplicatePublications: mainStats.duplicatePublications,
            ownershipErrors: mainStats.ownershipErrors,
            unknownModuleIds: mainStats.unknownModuleIds,
            scheduledDeliveries: mainStats.scheduledDeliveries,
            deliveryFailures: mainStats.deliveryFailures,
            receivedCacheEntries: 0,
            liveMainRegistryModules: mainModules.size,
            liveWorkerCacheModules: 0,
            purgedModules: mainStats.purgedModules,
            preloadAttempts: mainStats.preloadAttempts,
            preloadCandidates: mainStats.preloadCandidates,
            preloadSent: mainStats.preloadSent,
            preloadSkippedHeld: mainStats.preloadSkippedHeld,
            preloadFailures: mainStats.preloadFailures,
            preloadOwnerMisses: mainStats.preloadOwnerMisses,
            preloadOomBlocks: mainStats.preloadOomBlocks,
            staleIncarnationDrops: mainStats.staleIncarnationDrops,
            preloadModuleLimit: PRELOAD_MODULE_LIMIT,
            missingWorkerReplies: 0,
            workerStatsComplete: true,
            workerStatsExpected: 0,
            workerStatsReplied: 0,
            byOwner: []
        };
    }

    function addOwnerRow(rows, input) {
        var key = ownerKey(input.memoryId, input.memoryIncarnation);
        var row = rows.get(key);
        if (!row) {
            row = createOwnerStats(input.memoryId, input.memoryIncarnation);
            rows.set(key, row);
        }
        Object.keys(row).forEach(function(field) {
            if (field !== 'memoryId' && field !== 'memoryIncarnation' && field !== 'preloadModuleLimit' && input[field]) {
                row[field] += input[field];
            }
        });
    }

    function addWorkerSnapshot(aggregate, rows, snapshot) {
        aggregate.localCompiles += snapshot.localCompiles;
        aggregate.brokerHits += snapshot.brokerHits;
        aggregate.receivedCacheEntries += snapshot.receivedCacheEntries;
        aggregate.deliveryFailures += snapshot.deliveryFailures;
        aggregate.liveWorkerCacheModules += snapshot.liveWorkerCacheModules;
        aggregate.purgedModules += snapshot.purgedModules;
        aggregate.groupedBrokerHits += snapshot.groupedBrokerHits || 0;
        aggregate.groupedLocalCompiles += snapshot.groupedLocalCompiles || 0;
        aggregate.groupInstanceCreations += snapshot.groupInstanceCreations || 0;
        aggregate.groupInstanceReuses += snapshot.groupInstanceReuses || 0;
        snapshot.byOwner.forEach(function(row) {
            addOwnerRow(rows, row);
        });
    }

    function finishStatsRequest(requestId) {
        var pending = pendingStatsRequests.get(requestId);
        if (!pending) {
            return;
        }
        pendingStatsRequests.delete(requestId);
        if (pending.timer) {
            clearTimeout(pending.timer);
            pending.timer = 0;
        }
        try {
            if (pending.ownerRowCounts) {
                var workerSnapshots = Array.from(pending.replies.entries());
                workerSnapshots.sort(function(left, right) {
                    return left[0] < right[0] ? -1 : (left[0] > right[0] ? 1 : 0);
                });
                var missingTokens = pending.expectedTokens.filter(function(token) {
                    return !pending.replies.has(token);
                });
                pending.resolve({
                    mainRows: mainStats.byOwner.size,
                    expectedWorkers: pending.expected,
                    repliedWorkers: pending.replies.size,
                    missingTokens: missingTokens,
                    workerSnapshots: workerSnapshots.map(function(entry) {
                        return entry[1];
                    }),
                    workerRows: workerSnapshots.map(function(entry) {
                        return entry[1].byOwner.length;
                    }),
                    runMessages: testRunDispatchStats.runMessages,
                    preloadedRunMessages: testRunDispatchStats.preloadedRunMessages,
                    preloadModulesBeforeRun: testRunDispatchStats.preloadModulesBeforeRun
                });
                return;
            }
            var aggregate = emptyAggregate();
            var rows = new Map();
            mainStats.byOwner.forEach(function(row) {
                addOwnerRow(rows, row);
            });
            addWorkerSnapshot(aggregate, rows, snapshotLocalStats());
            pending.replies.forEach(function(snapshot) {
                addWorkerSnapshot(aggregate, rows, snapshot);
            });
            var runtimeBatchStats = Module.getWasmJitMtRuntimeBatchStats();
            aggregate.freshGroupedConstructionOomBlocks =
                runtimeBatchStats.groupedOomBlocks;
            aggregate.skippedFreshConstructionAttempts =
                runtimeBatchStats.skippedConstructionAttempts;
            aggregate.groupInstanceCreations =
                runtimeBatchStats.groupInstanceCreations;
            aggregate.groupInstanceReuses =
                runtimeBatchStats.groupInstanceReuses;
            aggregate.groupedBrokerHits =
                runtimeBatchStats.groupedBrokerHits;
            aggregate.groupedLocalCompiles =
                runtimeBatchStats.groupedLocalCompiles;
            aggregate.missingWorkerReplies = pending.expected - pending.replies.size;
            aggregate.workerStatsExpected = pending.expected;
            aggregate.workerStatsReplied = pending.replies.size;
            aggregate.workerStatsComplete =
                aggregate.missingWorkerReplies === 0;
            aggregate.byOwner = Array.from(rows.values()).sort(function(left, right) {
                if (left.memoryId !== right.memoryId) {
                    return left.memoryId - right.memoryId;
                }
                return left.memoryIncarnation - right.memoryIncarnation;
            });
            pending.resolve(aggregate);
        } catch (error) {
            pending.reject(error);
        }
    }

    function requestBrokerStats() {
        return new Promise(function(resolve, reject) {
            var requestId = nextStatsRequestId++;
            var workers = allocatedWorkers();
            var pending = {
                expected: workers.length,
                replies: new Map(),
                resolve: resolve,
                reject: reject,
                timer: 0
            };
            pendingStatsRequests.set(requestId, pending);
            workers.forEach(function(worker) {
                try {
                    worker.postMessage({
                        bwWasmJitModuleBroker: {
                            type: 'statsRequest',
                            requestId: requestId,
                            token: statsTokenForAllocatedWorker(worker)
                        }
                    });
                } catch (error) {
                    mainStats.deliveryFailures += 1;
                }
            });
            pending.timer = setTimeout(function() {
                finishStatsRequest(requestId);
            }, 500);
            if (!pending.expected) {
                finishStatsRequest(requestId);
            }
        });
    }

    function requestBrokerOwnerRowCounts(publisher, publisherSnapshot) {
        return new Promise(function(resolve, reject) {
            var requestId = nextStatsRequestId++;
            var workers = allocatedWorkers();
            var sourceWorker = publisherWorker(publisher);
            var expectedTokens = workers.map(statsTokenForAllocatedWorker);
            var replies = new Map();
            if (sourceWorker) {
                replies.set(statsTokenForAllocatedWorker(sourceWorker), publisherSnapshot);
            }
            var pending = {
                expected: workers.length,
                expectedTokens: expectedTokens,
                replies: replies,
                resolve: resolve,
                reject: reject,
                timer: 0,
                ownerRowCounts: true
            };
            pendingStatsRequests.set(requestId, pending);
            workers.forEach(function(worker, index) {
                if (worker === sourceWorker) {
                    return;
                }
                try {
                    worker.postMessage({
                        bwWasmJitModuleBroker: {
                            type: 'statsRequest',
                            requestId: requestId,
                            token: expectedTokens[index]
                        }
                    });
                } catch (error) {
                    mainStats.deliveryFailures += 1;
                }
            });
            pending.timer = setTimeout(function() {
                finishStatsRequest(requestId);
            }, 2000);
            if (pending.replies.size >= pending.expected) {
                finishStatsRequest(requestId);
            }
        });
    }

    function takeThreadStartOwner(startArg) {
        var testOwner = forcedTestThreadStartOwners.get(startArg >>> 0);
        if (testOwner) {
            forcedTestThreadStartOwners.delete(startArg >>> 0);
            return testOwner;
        }
        if (typeof Module._wasm_jit_mt_take_thread_start_owner !== 'function') {
            return {result: 0};
        }
        if (!takeThreadStartOwner.scratch) {
            takeThreadStartOwner.scratch = Module._malloc(8);
        }
        var scratch = takeThreadStartOwner.scratch;
        HEAPU32[scratch >> 2] = 0;
        HEAPU32[(scratch + 4) >> 2] = 0;
        var result = Module._wasm_jit_mt_take_thread_start_owner(startArg >>> 0, scratch, scratch + 4);
        return {
            result: result,
            memoryId: HEAPU32[scratch >> 2] >>> 0,
            memoryIncarnation: HEAPU32[(scratch + 4) >> 2] >>> 0
        };
    }

    function wrapWorkerRun(worker) {
        if (!worker || worker.bwWasmJitBrokerRunWrapped) {
            return worker;
        }
        worker.bwWasmJitBrokerRunWrapped = true;
        var originalPostMessage = worker.postMessage.bind(worker);
        worker.bwWasmJitBrokerOriginalPostMessage = originalPostMessage;
        worker.postMessage = function(message) {
            var originalArgs = arguments;
            if (!message || message.cmd !== 'run') {
                return originalPostMessage.apply(null, originalArgs);
            }
            if (dropNextTestRun) {
                dropNextTestRun = false;
                return;
            }
            testRunDispatchStats.runMessages += 1;
            var owner = takeThreadStartOwner(message.arg);
            if (owner.result === 0) {
                if (proxyMainBootstrapOwnerPending) {
                    proxyMainBootstrapOwnerPending = false;
                    return originalPostMessage.apply(null, originalArgs);
                }
                mainStats.preloadOwnerMisses += 1;
                return originalPostMessage.apply(null, originalArgs);
            }
            if (owner.result === -1) {
                return originalPostMessage.apply(null, originalArgs);
            }
            var key = ownerKey(owner.memoryId, owner.memoryIncarnation);
            if (!worker.bwWasmJitBrokerPreloadBlockedOwners) {
                worker.bwWasmJitBrokerPreloadBlockedOwners = new Set();
            }
            if (worker.bwWasmJitBrokerPreloadBlockedOwners.has(key)) {
                return originalPostMessage.apply(null, originalArgs);
            }
            var preload = preloadOwnerIntoWorker(worker, originalPostMessage,
                owner.memoryId, owner.memoryIncarnation);
            if (preload.sent) {
                testRunDispatchStats.preloadedRunMessages += 1;
                testRunDispatchStats.preloadModulesBeforeRun += preload.sent;
            }
            if (preload.error) {
                mainStats.preloadFailures += 1;
                mainStats.deliveryFailures += 1;
                var ownerStats = statsForOwner(mainStats.byOwner,
                    owner.memoryId, owner.memoryIncarnation);
                ownerStats.preloadFailures += 1;
                ownerStats.deliveryFailures += 1;
                if (isWasmOom(preload.error)) {
                    worker.bwWasmJitBrokerPreloadBlockedOwners.add(key);
                    var blockedWorkers = preloadBlockedWorkersByOwner.get(key);
                    if (!blockedWorkers) {
                        blockedWorkers = new Set();
                        preloadBlockedWorkersByOwner.set(key, blockedWorkers);
                    }
                    blockedWorkers.add(worker);
                    mainStats.preloadOomBlocks += 1;
                    ownerStats.preloadOomBlocks += 1;
                }
            }
            return originalPostMessage.apply(null, originalArgs);
        };
        return worker;
    }

    function installWorkerRunInterceptor() {
        if (typeof PThread === 'undefined' || PThread.bwWasmJitBrokerRunInterceptor) {
            return;
        }
        proxyMainBootstrapOwnerPending =
            typeof Module['__emscripten_proxy_main'] === 'function';
        PThread.bwWasmJitBrokerRunInterceptor = true;
        var originalGetNewWorker = PThread.getNewWorker;
        PThread.getNewWorker = function() {
            return wrapWorkerRun(originalGetNewWorker.apply(this, arguments));
        };
        PThread.runningWorkers.concat(PThread.unusedWorkers).forEach(wrapWorkerRun);
    }

    function receiveBrokerMessage(message) {
        if (!message) {
            return;
        }
        if (message.type === 'module') {
            var moduleId = moduleKey(message.moduleId);
            var memoryId = message.memoryId >>> 0;
            var memoryIncarnation = message.memoryIncarnation >>> 0;
            var cached = localModules.get(moduleId);
            if (!cached || cached.memoryId !== memoryId || cached.memoryIncarnation !== memoryIncarnation) {
                localModules.set(moduleId, {
                    memoryId: memoryId,
                    memoryIncarnation: memoryIncarnation,
                    module: message.module
                });
                localStats.receivedCacheEntries += 1;
                statsForOwner(localStats.byOwner, memoryId, memoryIncarnation).receivedCacheEntries += 1;
            }
        } else if (message.type === 'purge') {
            var purgeKey = ownerKey(message.memoryId, message.memoryIncarnation);
            if (testTrackedPurgeOwners.has(purgeKey)) {
                testPurgeMessagesByOwner.set(purgeKey,
                    (testPurgeMessagesByOwner.get(purgeKey) || 0) + 1);
            }
            purgeLocalModules(message.memoryId, message.memoryIncarnation, message.moduleIds);
        } else if (message.type === 'testTrackPurges') {
            testTrackedPurgeOwners.add(ownerKey(message.memoryId, message.memoryIncarnation));
        } else if (message.type === 'testPurgeCountBarrier') {
            var testKey = ownerKey(message.memoryId, message.memoryIncarnation);
            var testPurgeMessages = testPurgeMessagesByOwner.get(testKey) || 0;
            testTrackedPurgeOwners.delete(testKey);
            testPurgeMessagesByOwner.delete(testKey);
            var testCachedModules = 0;
            localModules.forEach(function(entry) {
                if (entry.memoryId === (message.memoryId >>> 0) &&
                        entry.memoryIncarnation === (message.memoryIncarnation >>> 0)) {
                    testCachedModules += 1;
                }
            });
            Atomics.add(HEAP32, message.purgeMessages >> 2, testPurgeMessages);
            Atomics.add(HEAP32, message.cachedModules >> 2, testCachedModules);
            Atomics.add(HEAP32, message.received >> 2, 1);
            Atomics.notify(HEAP32, message.received >> 2);
        } else if (message.type === 'statsRequest') {
            postToMain({
                cmd: 'callHandler',
                handler: 'bwWasmJitBrokerStatsReply',
                args: [message.requestId, message.token || workerStatsToken, snapshotLocalStats()]
            });
        } else if (message.type === 'barrier') {
            if (message.cached && message.moduleId) {
                var barrierModuleId = moduleKey(message.moduleId);
                var barrierMemoryId = message.memoryId >>> 0;
                var barrierMemoryIncarnation = message.memoryIncarnation >>> 0;
                var barrierEntry = localModules.get(barrierModuleId);
                if (barrierEntry && barrierEntry.memoryId === barrierMemoryId &&
                        barrierEntry.memoryIncarnation === barrierMemoryIncarnation) {
                    Atomics.add(HEAP32, message.cached >> 2, 1);
                }
            }
            Atomics.add(HEAP32, message.received >> 2, 1);
            Atomics.notify(HEAP32, message.received >> 2);
        }
    }

    if (typeof addEventListener === 'function') {
        addEventListener('message', function(event) {
            receiveBrokerMessage(event.data && event.data.bwWasmJitModuleBroker);
        });
    } else if (typeof worker_threads !== 'undefined' && worker_threads.parentPort) {
        worker_threads.parentPort.on('message', function(message) {
            receiveBrokerMessage(message && message.bwWasmJitModuleBroker);
        });
    }

    if (typeof ENVIRONMENT_IS_PTHREAD === 'undefined' || !ENVIRONMENT_IS_PTHREAD) {
        Module['bwWasmJitBrokerPublish'] = publishOnMain;
        Module['bwWasmJitBrokerReleaseMemory'] = releaseMemoryOnMain;
        Module['bwWasmJitBrokerDeliveryBarrier'] = function(moduleId, publisher, expected, received) {
            var entry = mainModules.get(moduleKey(moduleId));
            Atomics.store(HEAP32, received >> 2, entry ? 1 : 0);
            Atomics.store(HEAP32, expected >> 2, 1);
            Atomics.notify(HEAP32, received >> 2);
            Atomics.notify(HEAP32, expected >> 2);
        };
        Module['bwWasmJitBrokerPublicationBarrier'] = function(moduleId, preloadSent, receivedCacheEntries, done) {
            if (!mainModules.has(moduleKey(moduleId))) {
                return;
            }
            requestBrokerStats().then(function(snapshot) {
                Atomics.store(HEAP32, preloadSent >> 2, mainStats.preloadSent || 0);
                Atomics.store(HEAP32, receivedCacheEntries >> 2, snapshot.receivedCacheEntries);
                Atomics.store(HEAP32, done >> 2, 1);
                Atomics.notify(HEAP32, done >> 2);
            });
        };
        Module['bwWasmJitBrokerSetTestPreloadCandidate'] = function(templateModuleId, moduleId,
                memoryId, memoryIncarnation, moduleClass, representedBlockCount,
                duplicatePublications, enabled, status) {
            moduleId = moduleKey(moduleId);
            memoryId = memoryId >>> 0;
            memoryIncarnation = memoryIncarnation >>> 0;
            var key = ownerKey(memoryId, memoryIncarnation);
            var ownerModules = mainModulesByOwner.get(key);
            if (!enabled) {
                var current = mainModules.get(moduleId);
                if (current && current.memoryId === memoryId &&
                        current.memoryIncarnation === memoryIncarnation) {
                    mainModules.delete(moduleId);
                }
                if (ownerModules) {
                    ownerModules.delete(moduleId);
                    if (!ownerModules.size) {
                        mainModulesByOwner.delete(key);
                    }
                }
                Atomics.store(HEAP32, status >> 2, 1);
                Atomics.notify(HEAP32, status >> 2);
                return;
            }

            var templateEntry = mainModules.get(moduleKey(templateModuleId));
            if (!templateEntry || mainModules.has(moduleId)) {
                Atomics.store(HEAP32, status >> 2, -1);
                Atomics.notify(HEAP32, status >> 2);
                return;
            }
            var entry = {
                moduleId: moduleId,
                memoryId: memoryId,
                memoryIncarnation: memoryIncarnation,
                module: templateEntry.module,
                moduleClass: moduleClass >>> 0,
                representedBlockCount: representedBlockCount >>> 0,
                duplicatePublications: Math.max(0, duplicatePublications | 0),
                publishSequence: ++publicationSequence,
                holders: new Set()
            };
            mainModules.set(moduleId, entry);
            if (!ownerModules) {
                ownerModules = new Map();
                mainModulesByOwner.set(key, ownerModules);
            }
            ownerModules.set(moduleId, entry);
            Atomics.store(HEAP32, status >> 2, 1);
            Atomics.notify(HEAP32, status >> 2);
        };
        Module['bwWasmJitBrokerSelectPreload'] = function(memoryId, memoryIncarnation, heldIds,
                sentIds, sentCapacity, sentCount, done) {
            var sent = [];
            var originalPostMessage = function(message) {
                var payload = message && message.bwWasmJitModuleBroker;
                if (payload && payload.type === 'module') {
                    sent.push(payload.moduleId >>> 0);
                }
            };
            var worker = {
                postMessage: originalPostMessage,
                bwWasmJitBrokerHeldIds: new Set(heldIds)
            };
            preloadOwnerIntoWorker(worker, originalPostMessage, memoryId, memoryIncarnation);
            var count = Math.min(sent.length, sentCapacity >>> 0);
            for (var i = 0; i < count; i++) {
                HEAPU32[(sentIds >> 2) + i] = sent[i];
            }
            Atomics.store(HEAP32, sentCount >> 2, sent.length);
            Atomics.store(HEAP32, done >> 2, 1);
            Atomics.notify(HEAP32, done >> 2);
        };
        Module['bwWasmJitBrokerCopyMainStats'] = function(snapshot, done) {
            var values = [
                mainStats.firstPublications,
                mainStats.duplicatePublications,
                mainStats.ownershipErrors,
                mainStats.unknownModuleIds,
                mainStats.scheduledDeliveries,
                mainStats.deliveryFailures,
                mainStats.purgedModules,
                mainModules.size,
                mainStats.preloadAttempts,
                mainStats.preloadCandidates,
                mainStats.preloadSent,
                mainStats.preloadSkippedHeld,
                mainStats.preloadFailures,
                mainStats.preloadOwnerMisses,
                mainStats.preloadOomBlocks,
                mainStats.staleIncarnationDrops,
                PRELOAD_MODULE_LIMIT
            ];
            for (var i = 0; i < values.length; i++) {
                HEAPU32[(snapshot >> 2) + i] = values[i];
            }
            Atomics.store(HEAP32, done >> 2, 1);
            Atomics.notify(HEAP32, done >> 2);
        };
        Module['bwWasmJitBrokerCopyHolderCounts'] = function(moduleId, publisher, holders, workers, done) {
            var entry = mainModules.get(moduleKey(moduleId));
            if (entry) {
                var sourceWorker = publisherWorker(publisher);
                allocatedWorkers().forEach(function(worker) {
                    if (worker !== sourceWorker) {
                        worker.postMessage({
                            bwWasmJitModuleBroker: {
                                type: 'testTrackPurges',
                                memoryId: entry.memoryId,
                                memoryIncarnation: entry.memoryIncarnation
                            }
                        });
                    }
                });
            }
            Atomics.store(HEAP32, holders >> 2, entry ? entry.holders.size : 0);
            Atomics.store(HEAP32, workers >> 2, allocatedWorkers().length);
            Atomics.store(HEAP32, done >> 2, 1);
            Atomics.notify(HEAP32, done >> 2);
        };
        Module['bwWasmJitBrokerTrackPurges'] = function(memoryId, memoryIncarnation, publisher, done) {
            var sourceWorker = publisherWorker(publisher);
            allocatedWorkers().forEach(function(worker) {
                if (worker !== sourceWorker) {
                    worker.postMessage({
                        bwWasmJitModuleBroker: {
                            type: 'testTrackPurges',
                            memoryId: memoryId,
                            memoryIncarnation: memoryIncarnation
                        }
                    });
                }
            });
            Atomics.store(HEAP32, done >> 2, 1);
            Atomics.notify(HEAP32, done >> 2);
        };
        Module['bwWasmJitBrokerPurgeCountBarrier'] = function(memoryId, memoryIncarnation,
                publisher, expected, received, purgeMessages, cachedModules) {
            var sourceWorker = publisherWorker(publisher);
            var count = 0;
            allocatedWorkers().forEach(function(worker) {
                if (worker !== sourceWorker) {
                    worker.postMessage({
                        bwWasmJitModuleBroker: {
                            type: 'testPurgeCountBarrier',
                            memoryId: memoryId,
                            memoryIncarnation: memoryIncarnation,
                            purgeMessages: purgeMessages,
                            cachedModules: cachedModules,
                            received: received
                        }
                    });
                    count += 1;
                }
            });
            Atomics.store(HEAP32, expected >> 2, count);
            Atomics.notify(HEAP32, expected >> 2);
        };
        Module['bwWasmJitBrokerPurgeBarrier'] = function(moduleId, memoryId, memoryIncarnation,
                publisher, expected, received, cached) {
            var sourceWorker = publisherWorker(publisher);
            var count = 0;
            allocatedWorkers().forEach(function(worker) {
                if (worker !== sourceWorker) {
                    worker.postMessage({
                        bwWasmJitModuleBroker: {
                            type: 'barrier',
                            moduleId: moduleId,
                            memoryId: memoryId,
                            memoryIncarnation: memoryIncarnation,
                            cached: cached,
                            received: received
                        }
                    });
                    count += 1;
                }
            });
            Atomics.store(HEAP32, expected >> 2, count);
            Atomics.notify(HEAP32, expected >> 2);
        };
        Module['bwWasmJitBrokerCopyQueryDisabled'] = function(queryDisabled, done) {
            var disabled = typeof location !== 'undefined' &&
                new URLSearchParams(location.search).get('wasmModuleBroker') === '0';
            Atomics.store(HEAP32, queryDisabled >> 2, disabled ? 1 : 0);
            Atomics.store(HEAP32, done >> 2, 1);
            Atomics.notify(HEAP32, done >> 2);
        };
        Module['bwWasmJitBrokerSetDeliveryFailure'] = function(moduleId, enabled, done) {
            moduleId = moduleKey(moduleId);
            if (enabled) {
                forcedDeliveryFailureIds.add(moduleId);
            } else {
                forcedDeliveryFailureIds.delete(moduleId);
            }
            Atomics.store(HEAP32, done >> 2, 1);
            Atomics.notify(HEAP32, done >> 2);
        };
        Module['bwWasmJitBrokerSetDeliveryOom'] = function(moduleId, enabled, done) {
            moduleId = moduleKey(moduleId);
            if (enabled) {
                forcedDeliveryOomIds.add(moduleId);
            } else {
                forcedDeliveryOomIds.delete(moduleId);
            }
            Atomics.store(HEAP32, done >> 2, 1);
            Atomics.notify(HEAP32, done >> 2);
        };
        Module['bwWasmJitBrokerSetDropNextRun'] = function(enabled, done) {
            dropNextTestRun = !!enabled;
            Atomics.store(HEAP32, done >> 2, 1);
            Atomics.notify(HEAP32, done >> 2);
        };
        Module['bwWasmJitBrokerCopyPreloadOomScope'] = function(firstModuleId,
                firstMemoryId, firstMemoryIncarnation, secondModuleId,
                secondMemoryId, secondMemoryIncarnation, snapshot, done) {
            var firstRecord = {runs: 0, moduleIds: []};
            var secondRecord = {runs: 0, moduleIds: []};
            var makeWorker = function(record) {
                return wrapWorkerRun({
                    postMessage: function(message) {
                        if (message && message.cmd === 'run') {
                            record.runs += 1;
                        }
                        var payload = message && message.bwWasmJitModuleBroker;
                        if (payload && payload.type === 'module') {
                            record.moduleIds.push(payload.moduleId >>> 0);
                        }
                    }
                });
            };
            var firstWorker = makeWorker(firstRecord);
            var secondWorker = makeWorker(secondRecord);
            var firstArg = 0xfffffff1;
            var secondArg = 0xfffffff2;
            var beforeAttempts = mainStats.preloadAttempts;
            var beforeSent = mainStats.preloadSent;
            var beforeFailures = mainStats.preloadFailures;
            var beforeBlocks = mainStats.preloadOomBlocks;
            var postRun = function(worker, startArg, memoryId, memoryIncarnation) {
                forcedTestThreadStartOwners.set(startArg, {
                    result: 1,
                    memoryId: memoryId >>> 0,
                    memoryIncarnation: memoryIncarnation >>> 0
                });
                worker.postMessage({cmd: 'run', arg: startArg});
            };

            forcedDeliveryOomIds.add(firstModuleId >>> 0);
            postRun(firstWorker, firstArg, firstMemoryId, firstMemoryIncarnation);
            forcedDeliveryOomIds.delete(firstModuleId >>> 0);
            postRun(firstWorker, firstArg, firstMemoryId, firstMemoryIncarnation);
            postRun(secondWorker, secondArg, firstMemoryId, firstMemoryIncarnation);
            postRun(firstWorker, firstArg, secondMemoryId, secondMemoryIncarnation);

            var values = [
                firstWorker !== secondWorker ? 1 : 0,
                firstRecord.runs,
                secondRecord.runs,
                firstRecord.moduleIds.filter(function(id) {
                    return id === (firstModuleId >>> 0);
                }).length,
                secondRecord.moduleIds.filter(function(id) {
                    return id === (firstModuleId >>> 0);
                }).length,
                firstRecord.moduleIds.filter(function(id) {
                    return id === (secondModuleId >>> 0);
                }).length,
                mainStats.preloadAttempts - beforeAttempts,
                mainStats.preloadSent - beforeSent,
                mainStats.preloadFailures - beforeFailures,
                mainStats.preloadOomBlocks - beforeBlocks
            ];
            for (var i = 0; i < values.length; i++) {
                HEAPU32[(snapshot >> 2) + i] = values[i];
            }
            Atomics.store(HEAP32, done >> 2, 1);
            Atomics.notify(HEAP32, done >> 2);
        };
        Module['bwWasmJitBrokerCopyPublicPreloadLimit'] = function(preloadModuleLimit, done) {
            requestBrokerStats().then(function(snapshot) {
                Atomics.store(HEAP32, preloadModuleLimit >> 2, snapshot.preloadModuleLimit);
                Atomics.store(HEAP32, done >> 2, 1);
                Atomics.notify(HEAP32, done >> 2);
            });
        };
        Module['bwWasmJitBrokerCopyOwnerRowCounts'] = function(mainRows, workerRows,
                workerStats, workerCapacity, workerCount, expectedWorkers, repliedWorkers,
                missingWorkers, runMessages, preloadedRunMessages,
                preloadModulesBeforeRun, publisher, publisherSnapshot, done) {
            requestBrokerOwnerRowCounts(publisher, publisherSnapshot).then(function(snapshot) {
                var count = snapshot.workerRows.length;
                Atomics.store(HEAP32, mainRows >> 2, snapshot.mainRows);
                Atomics.store(HEAP32, workerCount >> 2, count);
                Atomics.store(HEAP32, expectedWorkers >> 2, snapshot.expectedWorkers);
                Atomics.store(HEAP32, repliedWorkers >> 2, snapshot.repliedWorkers);
                Atomics.store(HEAP32, missingWorkers >> 2, snapshot.missingTokens.length);
                Atomics.store(HEAP32, runMessages >> 2, snapshot.runMessages);
                Atomics.store(HEAP32, preloadedRunMessages >> 2, snapshot.preloadedRunMessages);
                Atomics.store(HEAP32, preloadModulesBeforeRun >> 2,
                    snapshot.preloadModulesBeforeRun);
                for (var i = 0; i < count && i < (workerCapacity >>> 0); ++i) {
                    Atomics.store(HEAP32, (workerRows >> 2) + i, snapshot.workerRows[i]);
                    var workerSnapshot = snapshot.workerSnapshots[i];
                    var statsOffset = (workerStats >> 2) + i * 6;
                    Atomics.store(HEAP32, statsOffset + 0, workerSnapshot.localCompiles || 0);
                    Atomics.store(HEAP32, statsOffset + 1, workerSnapshot.brokerHits || 0);
                    Atomics.store(HEAP32, statsOffset + 2,
                        workerSnapshot.groupInstanceCreations || 0);
                    Atomics.store(HEAP32, statsOffset + 3,
                        workerSnapshot.groupInstanceReuses || 0);
                    Atomics.store(HEAP32, statsOffset + 4,
                        workerSnapshot.groupedBrokerHits || 0);
                    Atomics.store(HEAP32, statsOffset + 5,
                        workerSnapshot.groupedLocalCompiles || 0);
                }
                if (snapshot.missingTokens.length) {
                    console.error('[WASM JIT module broker] owner-row stats request timed out',
                        {
                            expectedWorkers: snapshot.expectedWorkers,
                            repliedWorkers: snapshot.repliedWorkers,
                            missingTokens: snapshot.missingTokens
                        });
                }
                Atomics.store(HEAP32, done >> 2,
                    snapshot.missingTokens.length ? -2 :
                        (count <= (workerCapacity >>> 0) ? 1 : -1));
                Atomics.notify(HEAP32, done >> 2);
            });
        };
        Module['bwWasmJitBrokerResetScheduleThreadTestStats'] = function(done) {
            testRunDispatchStats.runMessages = 0;
            testRunDispatchStats.preloadedRunMessages = 0;
            testRunDispatchStats.preloadModulesBeforeRun = 0;
            Atomics.store(HEAP32, done >> 2, 1);
            Atomics.notify(HEAP32, done >> 2);
        };
        Module['bwWasmJitBrokerTestBootstrapOwnerMiss'] = function(done) {
            var firstArg = 0xffffffd1;
            var secondArg = 0xffffffd2;
            var forwardedRuns = 0;
            var beforeOwnerMisses = mainStats.preloadOwnerMisses;
            var beforeRunMessages = testRunDispatchStats.runMessages;
            var restoreBootstrapPending = proxyMainBootstrapOwnerPending;
            proxyMainBootstrapOwnerPending = true;
            var worker = wrapWorkerRun({
                postMessage: function(message) {
                    if (message && message.cmd === 'run') {
                        forwardedRuns += 1;
                    }
                }
            });
            forcedTestThreadStartOwners.set(firstArg, {result: 0});
            forcedTestThreadStartOwners.set(secondArg, {result: 0});
            worker.postMessage({cmd: 'run', arg: firstArg});
            worker.postMessage({cmd: 'run', arg: secondArg});
            var ownerMissDelta =
                mainStats.preloadOwnerMisses - beforeOwnerMisses;
            mainStats.preloadOwnerMisses = beforeOwnerMisses;
            testRunDispatchStats.runMessages = beforeRunMessages;
            proxyMainBootstrapOwnerPending = restoreBootstrapPending;
            Atomics.store(HEAP32, done >> 2,
                forwardedRuns === 2 && ownerMissDelta === 1 ? 1 : -1);
            Atomics.notify(HEAP32, done >> 2);
        };
        Module['bwWasmJitBrokerTestStatsSnapshotFailure'] = function(done) {
            if (typeof PThread === 'undefined') {
                Atomics.store(HEAP32, done >>> 2, -1);
                Atomics.notify(HEAP32, done >>> 2);
                return;
            }
            var originalRunningWorkers = PThread.runningWorkers;
            var originalUnusedWorkers = PThread.unusedWorkers;
            var originalCopy = Module._wasm_jit_mt_copy_runtime_batch_stats;
            var requestId = 0;
            var requestToken = '';
            var completed = false;
            var timeout = 0;
            var fakeWorker = {
                postMessage: function(message) {
                    var request = message && message.bwWasmJitModuleBroker;
                    requestId = request ? request.requestId : 0;
                    requestToken = request ? request.token : '';
                    setTimeout(function() {
                        try {
                            Module.bwWasmJitBrokerStatsReply(
                                requestId, requestToken, snapshotLocalStats());
                        } catch (error) {
                            // The pre-fix path throws here and leaves the Promise pending.
                        }
                    }, 0);
                }
            };
            var finish = function(rejected) {
                if (completed) {
                    return;
                }
                completed = true;
                if (timeout) {
                    clearTimeout(timeout);
                }
                PThread.runningWorkers = originalRunningWorkers;
                PThread.unusedWorkers = originalUnusedWorkers;
                var lateCopyCalls = 0;
                Module._wasm_jit_mt_copy_runtime_batch_stats = function() {
                    lateCopyCalls += 1;
                };
                try {
                    Module.bwWasmJitBrokerStatsReply(
                        requestId, requestToken + '-late', snapshotLocalStats());
                } catch (error) {
                }
                Module._wasm_jit_mt_copy_runtime_batch_stats = originalCopy;
                Atomics.store(HEAP32, done >>> 2,
                    rejected && lateCopyCalls === 0 ? 1 : -1);
                Atomics.notify(HEAP32, done >>> 2);
            };
            PThread.runningWorkers = [];
            PThread.unusedWorkers = [fakeWorker];
            Module._wasm_jit_mt_copy_runtime_batch_stats = function() {
                throw new Error('forced asynchronous runtime batch snapshot failure');
            };
            requestBrokerStats().then(function() {
                finish(false);
            }, function(error) {
                finish(String(error).indexOf(
                    'forced asynchronous runtime batch snapshot failure') >= 0);
            });
            timeout = setTimeout(function() {
                finish(false);
            }, 100);
        };
        Module['bwWasmJitBrokerTestIncompleteStats'] = function(
                expectedCreations, expectedReuses, expectedBrokerHits,
                expectedLocalCompiles, done) {
            if (typeof PThread === 'undefined') {
                Atomics.store(HEAP32, done >>> 2, -1);
                Atomics.notify(HEAP32, done >>> 2);
                return;
            }
            var originalRunningWorkers = PThread.runningWorkers;
            var originalUnusedWorkers = PThread.unusedWorkers;
            var finish = function(value) {
                PThread.runningWorkers = originalRunningWorkers;
                PThread.unusedWorkers = originalUnusedWorkers;
                Atomics.store(HEAP32, done >>> 2, value);
                Atomics.notify(HEAP32, done >>> 2);
            };
            PThread.runningWorkers = [];
            PThread.unusedWorkers = [{
                postMessage: function() {
                }
            }];
            requestBrokerStats().then(function(snapshot) {
                var exact = snapshot.missingWorkerReplies === 1 &&
                    snapshot.workerStatsComplete === false &&
                    snapshot.workerStatsExpected === 1 &&
                    snapshot.workerStatsReplied === 0 &&
                    snapshot.groupInstanceCreations === (expectedCreations >>> 0) &&
                    snapshot.groupInstanceReuses === (expectedReuses >>> 0) &&
                    snapshot.groupedBrokerHits === (expectedBrokerHits >>> 0) &&
                    snapshot.groupedLocalCompiles === (expectedLocalCompiles >>> 0);
                finish(exact ? 1 : -1);
            }, function() {
                finish(-1);
            });
        };
        Module['bwWasmJitBrokerStatsReply'] = function(requestId, token, snapshot) {
            var pending = pendingStatsRequests.get(requestId);
            if (!pending || pending.replies.has(token)) {
                return;
            }
            pending.replies.set(token, snapshot);
            if (pending.replies.size >= pending.expected) {
                finishStatsRequest(requestId);
            }
        };
        Module['getWasmJitModuleBrokerStats'] = requestBrokerStats;
        Module['logWasmJitModuleBrokerStats'] = async function() {
            var snapshot = await requestBrokerStats();
            console.table(snapshot.byOwner);
            console.log('[WASM JIT module broker]', snapshot);
            return snapshot;
        };
        Module['preRun'] = Module['preRun'] || [];
        Module['preRun'].push(installWorkerRunInterceptor);
        Module['preRun'].push(function() {
            if (typeof Module._wasm_jit_mt_set_module_broker_enabled === 'function' &&
                    typeof location !== 'undefined' && new URLSearchParams(location.search).get('wasmModuleBroker') === '0') {
                Module._wasm_jit_mt_set_module_broker_enabled(0);
            }
        });
    }
})();
