var rootDirectory = null;
onmessage = async (e) => {
  // retrieve message sent to work from main script
  const message = e.data;
  let that = this;
  if (message.action =="set") {
    setOPFSKV(message.key, message.value, 0);
  } else if(message.action =="get") {
    getOPFSKV(message.key, 0);
  } else if(message.action =="del") {
    delOPFSKV(message.key, 0);
  }
};
function delOPFSKV(key) {
    let work = function(key, retryCount) {
        getParentDirectoryHandle(key, "browserfs").then(parentDirHandle => {
            parentDirHandle.removeEntry(btoa(key)).then(() => {
                console.log("OPFS File deleted:" + key);
            }).catch(e => {
                if (retryCount < 5) {
                    setTimeout(() => work(key, retryCount + 1), 10);
                } else {
                    console.log(e);
                }
            });
        });
    };
    work(key, 0);
}

function getOPFSKV(key, retryCount) {
    getFileHandle(key, "browserfs").then( fileHandle => {
        if (fileHandle != null) {
            fileHandle.createSyncAccessHandle().then(accessHandle => {
                const size = accessHandle.getSize();
                const dataView = new Int8Array(size);
                accessHandle.read(dataView);
                accessHandle.close();
                postMessage({key: key, contents: dataView});
            }).catch(e => {
                if (retryCount < 3) {
                    setTimeout(() => getOPFSKV(key, retryCount + 1));
                } else {
                    console.log('getOPFSKV error: ' + e + " key:" + key);
                    postMessage({key: key, contents: null});
                }
            });
        }
    }).catch(e => {
        console.log('getOPFSKV getFileHandle error: ' + e + " key:" + key);
        postMessage({key: key, contents: null});
    });
}
function setOPFSKV(key, value, retryCount) {
    getSyncFileHandleCreateIfNecessary(key, "browserfs").then(accessHandle => {
        accessHandle.write(value);
        accessHandle.flush();
        accessHandle.close();
    }).catch(e => {
        if (retryCount < 3) {
            setTimeout(() => setOPFSKV(key, value, retryCount + 1));
        } else {
            console.log('setOPFSKV error: ' + e + " key:" + key);
        }
    });
}
function getFileHandle(filename, directory) {
    let encodedFilename = btoa(filename);
    let blockFolder = encodedFilename.substring(encodedFilename.length - 3, encodedFilename.length - 1);
    return navigator.storage.getDirectory().then(rootDir => {
        return rootDir.getDirectoryHandle(directory)
            .then(dirHandle => dirHandle.getDirectoryHandle(blockFolder)
                .then(blockDirHandle => blockDirHandle.getFileHandle(encodedFilename))
            ).catch(e => {
                //console.log('getFileHandle error: ' + e);
                postMessage({key: filename, contents: null});
            });
    });
}
function getParentDirectoryHandle(filename, directory) {
    let encodedFilename = btoa(filename);
    let blockFolder = encodedFilename.substring(encodedFilename.length - 3, encodedFilename.length - 1);
    return navigator.storage.getDirectory().then(rootDir => {
        return rootDir.getDirectoryHandle(directory, { create: true })
            .then(dirHandle => dirHandle.getDirectoryHandle(blockFolder, { create: true }))
    });
}

function getSyncFileHandleCreateIfNecessary(filename, directory) {
    return getParentDirectoryHandle(filename, directory)
        .then(blockDirHandle =>
            blockDirHandle.getFileHandle(btoa(filename), { create: true })
                .then(fileHandle => fileHandle.createSyncAccessHandle()));
}