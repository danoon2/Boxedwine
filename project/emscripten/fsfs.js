/*
 * Copyright 2022 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 *
 * Adapted from emscripten-core/emscripten#16804 for Boxedwine.
 */

(function(global) {
    function installFSFS() {
        if (global.FSFS) {
            return global.FSFS;
        }
        if (typeof FS === "undefined" || typeof MEMFS === "undefined") {
            throw new Error("FSFS requires Emscripten FS and MEMFS");
        }

        function joinPath(parent, name) {
            return parent.endsWith("/") ? parent + name : parent + "/" + name;
        }
        function dirname(path) {
            var index = path.lastIndexOf("/");
            if (index <= 0) {
                return ".";
            }
            return path.substring(0, index);
        }
        function basename(path) {
            var index = path.lastIndexOf("/");
            return index >= 0 ? path.substring(index + 1) : path;
        }
        function relativeFromMount(mountpoint, path) {
            var result = path.substring(mountpoint.length);
            if (result.startsWith("/")) {
                result = result.substring(1);
            }
            return result || ".";
        }
        async function getFsHandles(dirHandle) {
            var result = new Map();
            result.set(".", dirHandle);

            async function collect(currentHandle, currentPath) {
                for await (var entry of currentHandle.values()) {
                    var childPath = currentPath ? currentPath + "/" + entry.name : entry.name;
                    result.set(childPath, entry);
                    if (entry.kind === "directory") {
                        await collect(entry, childPath);
                    }
                }
            }

            await collect(dirHandle, "");
            return result;
        }
        function makeDate(value) {
            if (value instanceof Date) {
                return value;
            }
            return new Date(value || Date.now());
        }
        function entryTime(entry) {
            return makeDate(entry && entry.timestamp).getTime();
        }
        function entrySize(entry) {
            return entry && entry.size ? entry.size : 0;
        }
        function cloneEntry(entry) {
            return {
                timestamp: new Date(entryTime(entry)),
                mode: entry.mode,
                size: entrySize(entry)
            };
        }
        // The baseline records, per synced path, the guest (local) and host
        // (remote) metadata as of the last sync. Each side is compared only
        // against its own baseline, since guest and host timestamps and modes
        // never match each other after a write.
        function makeBaselineEntry(localEntry, remoteEntry) {
            return {local: cloneEntry(localEntry), remote: cloneEntry(remoteEntry)};
        }
        function cloneBaseline(baseline) {
            var result = Object.create(null);
            Object.keys(baseline).forEach(function(path) {
                result[path] = makeBaselineEntry(baseline[path].local, baseline[path].remote);
            });
            return result;
        }
        function sameKind(a, b) {
            return FS.isFile(a.mode) === FS.isFile(b.mode) && FS.isDir(a.mode) === FS.isDir(b.mode);
        }
        function entriesSame(a, b) {
            if (!a || !b) {
                return !a && !b;
            }
            if (!sameKind(a, b)) {
                return false;
            }
            if (FS.isFile(a.mode)) {
                return entryTime(a) === entryTime(b) && entrySize(a) === entrySize(b);
            }
            return true;
        }
        function makeReconcileResult() {
            return {
                created: 0,
                updated: 0,
                removed: 0,
                conflicts: []
            };
        }
        function addConflict(result, path, reason) {
            result.conflicts.push({path: path, reason: reason});
        }
        // Guest mutations made by FSFS itself must not queue a persist back to
        // the host. Only the synchronous local store/remove calls are covered, so
        // guest writes made while FSFS awaits host I/O are still persisted.
        function applyLocal(mount, fn) {
            mount.fsfsApplyingLocal = true;
            try {
                return fn();
            } finally {
                mount.fsfsApplyingLocal = false;
            }
        }
        function isLocalPathOpen(path) {
            if (!FS.streams) {
                return false;
            }
            var node;
            try {
                node = FS.lookupPath(path).node;
            } catch (e) {
                return false;
            }
            for (var i = 0; i < FS.streams.length; i++) {
                var stream = FS.streams[i];
                if (stream && stream.node === node) {
                    return true;
                }
            }
            return false;
        }

        var FSFS = {
            DIR_MODE: 16384 | 511,
            FILE_MODE: 32768 | 511,
            queueSync: function(mount, work) {
                var previous = mount.fsfsSyncQueue || Promise.resolve();
                var current = previous.catch(function() {}).then(work);
                mount.fsfsSyncQueue = current;
                return current;
            },
            notifyConflicts: function(mount, source, result) {
                if (result && result.conflicts.length && mount.opts &&
                        typeof mount.opts.onConflict === "function") {
                    mount.opts.onConflict(source, result.conflicts);
                }
            },
            queuePersist: function(mount) {
                if (mount.fsfsApplyingLocal) {
                    return;
                }
                function onPersistComplete(error, result) {
                    if (error && mount.opts && typeof mount.opts.onPersistError === "function") {
                        mount.opts.onPersistError(error);
                    }
                    if (!error) {
                        FSFS.notifyConflicts(mount, "persist", result);
                    }
                    if (mount.fsfsPersistState === "again") {
                        startPersist();
                    } else {
                        mount.fsfsPersistState = 0;
                        if (!error && mount.opts && typeof mount.opts.onPersistComplete === "function") {
                            mount.opts.onPersistComplete();
                        }
                    }
                }
                function startPersist() {
                    mount.fsfsPersistState = "fsfs";
                    FSFS.syncfs(mount, false, onPersistComplete);
                }

                if (!mount.fsfsPersistState) {
                    mount.fsfsPersistState = setTimeout(startPersist, 0);
                } else if (mount.fsfsPersistState === "fsfs") {
                    mount.fsfsPersistState = "again";
                }
            },
            mount: function(mount) {
                if (!mount.opts.dirHandle) {
                    throw new Error("opts.dirHandle is required");
                }

                var mounted = MEMFS.mount.apply(null, arguments);
                if (mount.opts.autoPersist) {
                    mount.fsfsPersistState = 0;
                    mount.fsfsSyncQueue = Promise.resolve();
                    mount.fsfsBaseline = Object.create(null);
                    var memfsNodeOps = mounted.node_ops;
                    mounted.node_ops = Object.assign({}, mounted.node_ops);
                    mounted.node_ops.mknod = function(parent, name, mode, dev) {
                        var node = memfsNodeOps.mknod(parent, name, mode, dev);
                        node.node_ops = mounted.node_ops;
                        node.fsfs_mount = mounted.mount;
                        node.memfs_stream_ops = node.stream_ops;
                        node.stream_ops = Object.assign({}, node.stream_ops);
                        node.stream_ops.write = function(stream, buffer, offset, length, position, canOwn) {
                            stream.node.isModified = true;
                            return node.memfs_stream_ops.write(stream, buffer, offset, length, position, canOwn);
                        };
                        node.stream_ops.close = function(stream) {
                            var n = stream.node;
                            if (n.isModified) {
                                FSFS.queuePersist(n.fsfs_mount);
                                n.isModified = false;
                            }
                            if (n.memfs_stream_ops.close) {
                                return n.memfs_stream_ops.close(stream);
                            }
                        };
                        FSFS.queuePersist(mounted.mount);
                        return node;
                    };
                    mounted.node_ops.rmdir = function() {
                        FSFS.queuePersist(mounted.mount);
                        return memfsNodeOps.rmdir.apply(null, arguments);
                    };
                    mounted.node_ops.symlink = function() {
                        FSFS.queuePersist(mounted.mount);
                        return memfsNodeOps.symlink.apply(null, arguments);
                    };
                    mounted.node_ops.unlink = function() {
                        FSFS.queuePersist(mounted.mount);
                        return memfsNodeOps.unlink.apply(null, arguments);
                    };
                    mounted.node_ops.rename = function() {
                        FSFS.queuePersist(mounted.mount);
                        return memfsNodeOps.rename.apply(null, arguments);
                    };
                }
                return mounted;
            },
            syncfs: async function(mount, populate, callback) {
                FSFS.queueSync(mount, function() {
                    return FSFS.doSyncfs(mount, populate);
                }).then(function(result) {
                    callback(null, result);
                }).catch(function(e) {
                    callback(e);
                });
            },
            doSyncfs: async function(mount, populate) {
                var local = await FSFS.getLocalSet(mount);
                var remote = await FSFS.getRemoteSet(mount);
                var result;
                if (populate) {
                    result = await FSFS.reconcile(mount, remote, local);
                    mount.fsfsBaseline = FSFS.makeBaseline(FSFS.getLocalSet(mount), remote);
                } else if (mount.fsfsBaseline) {
                    result = await FSFS.reconcileLocalChanges(mount, local, remote);
                } else {
                    result = await FSFS.reconcile(mount, local, remote);
                    mount.fsfsBaseline = FSFS.makeBaseline(local, await FSFS.getRemoteSet(mount));
                }
                return result;
            },
            makeBaseline: function(local, remote) {
                var baseline = Object.create(null);
                Object.keys(remote.entries).forEach(function(path) {
                    if (local.entries[path]) {
                        baseline[path] = makeBaselineEntry(local.entries[path], remote.entries[path]);
                    }
                });
                return baseline;
            },
            refreshFromHost: function(mount, callback) {
                FSFS.queueSync(mount, async function() {
                    var local = await FSFS.getLocalSet(mount);
                    var remote = await FSFS.getRemoteSet(mount);
                    var result = await FSFS.reconcileRemoteChanges(mount, local, remote);
                    FSFS.notifyConflicts(mount, "refresh", result);
                    return result;
                }).then(function(result) {
                    callback(null, result);
                }).catch(function(e) {
                    callback(e);
                });
            },
            getLocalSet: function(mount) {
                var entries = Object.create(null);
                var check = FS.readdir(mount.mountpoint)
                    .filter(function(entry) { return entry !== "." && entry !== ".."; })
                    .map(function(entry) { return joinPath(mount.mountpoint, entry); });

                while (check.length) {
                    var path = check.pop();
                    var stat = FS.stat(path);
                    if (FS.isDir(stat.mode)) {
                        check.push.apply(check, FS.readdir(path)
                            .filter(function(entry) { return entry !== "." && entry !== ".."; })
                            .map(function(entry) { return joinPath(path, entry); }));
                    }
                    entries[path] = FSFS.statToEntry(stat);
                }
                return {type: "local", entries: entries};
            },
            statToEntry: function(stat) {
                return {
                    timestamp: makeDate(stat.mtime),
                    mode: stat.mode,
                    size: FS.isFile(stat.mode) ? stat.size : 0
                };
            },
            getRemoteSet: async function(mount) {
                var entries = Object.create(null);
                var handles = await getFsHandles(mount.opts.dirHandle);
                for (var item of handles.entries()) {
                    var path = item[0];
                    var handle = item[1];
                    if (path === ".") {
                        continue;
                    }
                    var timestamp = new Date();
                    var size = 0;
                    if (handle.kind === "file") {
                        var file = await handle.getFile();
                        timestamp = new Date(file.lastModified);
                        size = file.size;
                    }
                    entries[joinPath(mount.mountpoint, path)] = {
                        timestamp: timestamp,
                        mode: handle.kind === "file" ? FSFS.FILE_MODE : FSFS.DIR_MODE,
                        size: size
                    };
                }
                return {type: "remote", entries: entries, handles: handles};
            },
            loadLocalEntry: function(path) {
                var lookup = FS.lookupPath(path);
                var node = lookup.node;
                var stat = FS.stat(path);

                if (FS.isDir(stat.mode)) {
                    return {timestamp: makeDate(stat.mtime), mode: stat.mode, size: 0};
                } else if (FS.isFile(stat.mode)) {
                    node.contents = MEMFS.getFileDataAsTypedArray(node);
                    return {
                        timestamp: makeDate(stat.mtime),
                        mode: stat.mode,
                        size: node.contents.length,
                        contents: node.contents
                    };
                }
                throw new Error("node type not supported");
            },
            storeLocalEntry: function(path, entry) {
                if (FS.isDir(entry.mode)) {
                    FS.mkdirTree(path, entry.mode);
                } else if (FS.isFile(entry.mode)) {
                    FS.writeFile(path, entry.contents, {canOwn: true});
                } else {
                    throw new Error("node type not supported");
                }
                FS.chmod(path, entry.mode);
                FS.utime(path, entry.timestamp, entry.timestamp);
            },
            removeLocalEntry: function(path) {
                var stat = FS.stat(path);
                if (FS.isDir(stat.mode)) {
                    FS.rmdir(path);
                } else if (FS.isFile(stat.mode)) {
                    FS.unlink(path);
                }
            },
            loadRemoteEntry: async function(handle) {
                if (handle.kind === "file") {
                    var file = await handle.getFile();
                    return {
                        contents: new Uint8Array(await file.arrayBuffer()),
                        mode: FSFS.FILE_MODE,
                        timestamp: new Date(file.lastModified),
                        size: file.size
                    };
                } else if (handle.kind === "directory") {
                    return {
                        mode: FSFS.DIR_MODE,
                        timestamp: new Date(),
                        size: 0
                    };
                }
                throw new Error("unknown kind: " + handle.kind);
            },
            storeRemoteEntry: async function(handles, path, entry) {
                var parentDirHandle = handles.get(dirname(path));
                var handle = FS.isFile(entry.mode) ?
                    await parentDirHandle.getFileHandle(basename(path), {create: true}) :
                    await parentDirHandle.getDirectoryHandle(basename(path), {create: true});
                handles.set(path, handle);
                if (handle.kind === "file") {
                    var writable = await handle.createWritable({keepExistingData: false});
                    await writable.write(entry.contents);
                    await writable.close();
                    var stored = await handle.getFile();
                    return {timestamp: new Date(stored.lastModified), mode: FSFS.FILE_MODE, size: stored.size};
                }
                return {timestamp: new Date(), mode: FSFS.DIR_MODE, size: 0};
            },
            removeRemoteEntry: async function(handles, path) {
                var parentDirHandle = handles.get(dirname(path));
                await parentDirHandle.removeEntry(basename(path), {recursive: true});
                handles.delete(path);
            },
            reconcile: async function(mount, src, dst) {
                var result = makeReconcileResult();
                var create = [];
                Object.keys(src.entries).forEach(function(key) {
                    var entry = src.entries[key];
                    var existing = dst.entries[key];
                    if (!existing || (FS.isFile(entry.mode) && entry.timestamp.getTime() > existing.timestamp.getTime())) {
                        create.push(key);
                    }
                });
                create.sort();

                var remove = [];
                Object.keys(dst.entries).forEach(function(key) {
                    if (!src.entries[key]) {
                        remove.push(key);
                    }
                });
                remove.sort().reverse();

                if (!create.length && !remove.length) {
                    return result;
                }

                var handles = src.type === "remote" ? src.handles : dst.handles;
                for (var createPath of create) {
                    var relPath = relativeFromMount(mount.mountpoint, createPath);
                    if (dst.type === "local") {
                        var handle = handles.get(relPath);
                        var remoteEntry = await FSFS.loadRemoteEntry(handle);
                        applyLocal(mount, function() {
                            FSFS.storeLocalEntry(createPath, remoteEntry);
                        });
                    } else {
                        var localEntry = FSFS.loadLocalEntry(createPath);
                        await FSFS.storeRemoteEntry(handles, relPath, localEntry);
                    }
                    if (dst.entries[createPath]) {
                        result.updated++;
                    } else {
                        result.created++;
                    }
                }

                for (var removePath of remove) {
                    if (dst.type === "local") {
                        applyLocal(mount, function() {
                            FSFS.removeLocalEntry(removePath);
                        });
                    } else {
                        await FSFS.removeRemoteEntry(handles, relativeFromMount(mount.mountpoint, removePath));
                    }
                    result.removed++;
                }
                return result;
            },
            reconcileRemoteChanges: async function(mount, local, remote) {
                var result = makeReconcileResult();
                var baseline = mount.fsfsBaseline || Object.create(null);
                var nextBaseline = cloneBaseline(baseline);
                var remotePaths = Object.keys(remote.entries).sort();

                for (var i = 0; i < remotePaths.length; i++) {
                    var remotePath = remotePaths[i];
                    var remoteEntry = remote.entries[remotePath];
                    var localEntry = local.entries[remotePath];
                    var base = baseline[remotePath];
                    var remoteChanged = !base || !entriesSame(remoteEntry, base.remote);
                    var localChanged = localEntry && (!base || !entriesSame(localEntry, base.local));
                    var relPath = relativeFromMount(mount.mountpoint, remotePath);

                    if (!localEntry) {
                        if (base && remoteChanged) {
                            addConflict(result, remotePath, "host changed while guest removed it");
                            continue;
                        }
                        if (!base) {
                            var newHandle = remote.handles.get(relPath);
                            var newEntry = await FSFS.loadRemoteEntry(newHandle);
                            applyLocal(mount, function() {
                                FSFS.storeLocalEntry(remotePath, newEntry);
                            });
                            nextBaseline[remotePath] = makeBaselineEntry(FSFS.statToEntry(FS.stat(remotePath)), remoteEntry);
                            result.created++;
                        }
                    } else if (remoteChanged) {
                        if (localChanged || isLocalPathOpen(remotePath)) {
                            addConflict(result, remotePath, localChanged ?
                                "host and guest both changed it" : "guest has it open");
                            continue;
                        }
                        var changedHandle = remote.handles.get(relPath);
                        var changedEntry = await FSFS.loadRemoteEntry(changedHandle);
                        applyLocal(mount, function() {
                            FSFS.storeLocalEntry(remotePath, changedEntry);
                        });
                        nextBaseline[remotePath] = makeBaselineEntry(FSFS.statToEntry(FS.stat(remotePath)), remoteEntry);
                        result.updated++;
                    }
                }

                var localPaths = Object.keys(local.entries).sort().reverse();
                for (var j = 0; j < localPaths.length; j++) {
                    var localPath = localPaths[j];
                    if (remote.entries[localPath]) {
                        continue;
                    }
                    var localBase = baseline[localPath];
                    if (!localBase) {
                        continue;
                    }
                    var currentLocal = local.entries[localPath];
                    if (!entriesSame(currentLocal, localBase.local) || isLocalPathOpen(localPath)) {
                        addConflict(result, localPath, "host removed it while guest changed or opened it");
                        continue;
                    }
                    applyLocal(mount, function() {
                        FSFS.removeLocalEntry(localPath);
                    });
                    delete nextBaseline[localPath];
                    result.removed++;
                }

                mount.fsfsBaseline = nextBaseline;
                return result;
            },
            reconcileLocalChanges: async function(mount, local, remote) {
                var result = makeReconcileResult();
                var baseline = mount.fsfsBaseline || Object.create(null);
                var nextBaseline = cloneBaseline(baseline);
                var localPaths = Object.keys(local.entries).sort();

                for (var i = 0; i < localPaths.length; i++) {
                    var localPath = localPaths[i];
                    var localEntry = local.entries[localPath];
                    var remoteEntry = remote.entries[localPath];
                    var base = baseline[localPath];
                    var localChanged = !base || !entriesSame(localEntry, base.local);
                    var remoteChanged = remoteEntry && (!base || !entriesSame(remoteEntry, base.remote));
                    var relPath = relativeFromMount(mount.mountpoint, localPath);

                    if (!localChanged) {
                        continue;
                    }
                    if (!remoteEntry && base) {
                        addConflict(result, localPath, "guest changed it after host removed it");
                        continue;
                    }
                    if (remoteChanged && !entriesSame(localEntry, remoteEntry)) {
                        addConflict(result, localPath, "guest and host both changed it");
                        continue;
                    }
                    var entryToStore = FSFS.loadLocalEntry(localPath);
                    var storedEntry = await FSFS.storeRemoteEntry(remote.handles, relPath, entryToStore);
                    nextBaseline[localPath] = makeBaselineEntry(entryToStore, storedEntry);
                    if (remoteEntry) {
                        result.updated++;
                    } else {
                        result.created++;
                    }
                }

                var remotePaths = Object.keys(remote.entries).sort().reverse();
                for (var j = 0; j < remotePaths.length; j++) {
                    var remotePath = remotePaths[j];
                    if (local.entries[remotePath]) {
                        continue;
                    }
                    var remoteBase = baseline[remotePath];
                    if (!remoteBase) {
                        continue;
                    }
                    var currentRemote = remote.entries[remotePath];
                    if (!entriesSame(currentRemote, remoteBase.remote)) {
                        addConflict(result, remotePath, "guest removed it while host changed it");
                        continue;
                    }
                    await FSFS.removeRemoteEntry(remote.handles, relativeFromMount(mount.mountpoint, remotePath));
                    delete nextBaseline[remotePath];
                    result.removed++;
                }

                mount.fsfsBaseline = nextBaseline;
                return result;
            }
        };

        global.FSFS = FSFS;
        FS.filesystems.FSFS = FSFS;
        return FSFS;
    }

    global.installFSFS = installFSFS;
})(globalThis);
