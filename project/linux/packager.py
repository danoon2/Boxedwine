#!/usr/bin/python
import sys, os, subprocess

def error(s):
    print >> sys.stderr, s
    sys.exit(1)

def getfiletext(fn):
    try:
        f = open(fn, 'r')
        txt = f.read()
    except Exception, e:
        error('Error reading file: %s' % (str(e)))
    f.close
    return txt

try:
    txt = getfiletext('boxedwine.js')
    
    zeeCode = """\
    var zeeWorker = new Worker('zee-worker.js');
        var zeeCallbacks = [];
        zeeWorker.onmessage = function(msg) {
            zeeCallbacks[msg.data.callbackID](msg.data.data);
            console.log("zee'd " + msg.data.filename + ' in ' + msg.data.time + ' ms, ' + msg.data.data.length + '  bytes');
            zeeCallbacks[msg.data.callbackID] = null;
        };
        
        function requestZee(filename, data, callback) {
            zeeWorker.postMessage({
                filename: filename,
                data: new Uint8Array(data), // do not send over the underlying ArrayBuffer
                callbackID: zeeCallbacks.length
            });
            zeeCallbacks.push(callback);
        }
        Module.postRun.push(function() {
            zeeWorker.terminate();
        });
        //--
        function processPackageData(arrayBuffer) {
        Module.finishedDataFileDownloads++;
        assert(arrayBuffer, 'Loading data file failed.');
        
        requestZee("data", arrayBuffer, function(arrayBuffer) {
                postProcessPackageData(arrayBuffer);
            });
        }
        function postProcessPackageData(byteArray){
    """
    txt = txt.replace("function processPackageData(arrayBuffer){Module.finishedDataFileDownloads++;assert(arrayBuffer,\"Loading data file failed.\");assert(arrayBuffer instanceof ArrayBuffer,\"bad input to processPackageData\");var byteArray=new Uint8Array(arrayBuffer);", zeeCode)
    
    outf = open('package.js', 'w')
    outf.write(txt)
    outf.close
except Exception, e:
    error('Error ')

