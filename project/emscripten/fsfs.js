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

        var FSFS = {
            DIR_MODE: 16384 | 511,
            FILE_MODE: 32768 | 511,
            queuePersist: function(mount) {
                if (mount.fsfsPopulating) {
                    return;
                }
                function onPersistComplete(error) {
                    if (error && mount.opts && typeof mount.opts.onPersistError === "function") {
                        mount.opts.onPersistError(error);
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
                try {
                    if (populate) {
                        mount.fsfsPopulating = true;
                    }
                    var local = await FSFS.getLocalSet(mount);
                    var remote = await FSFS.getRemoteSet(mount);
                    var src = populate ? remote : local;
                    var dst = populate ? local : remote;
                    await FSFS.reconcile(mount, src, dst);
                    mount.fsfsPopulating = false;
                    callback(null);
                } catch (e) {
                    mount.fsfsPopulating = false;
                    callback(e);
                }
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
                    entries[path] = {timestamp: makeDate(stat.mtime), mode: stat.mode};
                }
                return {type: "local", entries: entries};
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
                    if (handle.kind === "file") {
                        timestamp = new Date((await handle.getFile()).lastModified);
                    }
                    entries[joinPath(mount.mountpoint, path)] = {
                        timestamp: timestamp,
                        mode: handle.kind === "file" ? FSFS.FILE_MODE : FSFS.DIR_MODE
                    };
                }
                return {type: "remote", entries: entries, handles: handles};
            },
            loadLocalEntry: function(path) {
                var lookup = FS.lookupPath(path);
                var node = lookup.node;
                var stat = FS.stat(path);

                if (FS.isDir(stat.mode)) {
                    return {timestamp: makeDate(stat.mtime), mode: stat.mode};
                } else if (FS.isFile(stat.mode)) {
                    node.contents = MEMFS.getFileDataAsTypedArray(node);
                    return {timestamp: makeDate(stat.mtime), mode: stat.mode, contents: node.contents};
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
                        timestamp: new Date(file.lastModified)
                    };
                } else if (handle.kind === "directory") {
                    return {
                        mode: FSFS.DIR_MODE,
                        timestamp: new Date()
                    };
                }
                throw new Error("unknown kind: " + handle.kind);
            },
            storeRemoteEntry: async function(handles, path, entry) {
                var parentDirHandle = handles.get(dirname(path));
                var handle = FS.isFile(entry.mode) ?
                    await parentDirHandle.getFileHandle(basename(path), {create: true}) :
                    await parentDirHandle.getDirectoryHandle(basename(path), {create: true});
                if (handle.kind === "file") {
                    var writable = await handle.createWritable({keepExistingData: false});
                    await writable.write(entry.contents);
                    await writable.close();
                }
                handles.set(path, handle);
            },
            removeRemoteEntry: async function(handles, path) {
                var parentDirHandle = handles.get(dirname(path));
                await parentDirHandle.removeEntry(basename(path), {recursive: true});
                handles.delete(path);
            },
            reconcile: async function(mount, src, dst) {
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
                    return;
                }

                var handles = src.type === "remote" ? src.handles : dst.handles;
                for (var createPath of create) {
                    var relPath = relativeFromMount(mount.mountpoint, createPath);
                    if (dst.type === "local") {
                        var handle = handles.get(relPath);
                        var remoteEntry = await FSFS.loadRemoteEntry(handle);
                        FSFS.storeLocalEntry(createPath, remoteEntry);
                    } else {
                        var localEntry = FSFS.loadLocalEntry(createPath);
                        await FSFS.storeRemoteEntry(handles, relPath, localEntry);
                    }
                }

                for (var removePath of remove) {
                    if (dst.type === "local") {
                        FSFS.removeLocalEntry(removePath);
                    } else {
                        await FSFS.removeRemoteEntry(handles, relativeFromMount(mount.mountpoint, removePath));
                    }
                }
            }
        };

        global.FSFS = FSFS;
        FS.filesystems.FSFS = FSFS;
        return FSFS;
    }

    global.installFSFS = installFSFS;
})(globalThis);
