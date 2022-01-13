<!-- This is a minimal example of how to drive BoxedWine 					-->
<!-- initialSetup() is the entry point. It is called from Module.preRun() 	-->

let statusElement = document.getElementById('status');
let progressElement = document.getElementById('progress');
let spinnerElement = document.getElementById('spinner');

let ROOT = "/root";
let DEFAULT_APP_DIRECTORY = ROOT + "/files/";
let Config = {
    locateRootBaseUrl: "",
    locateAppBaseUrl: ""
};
//based on code in emularity github project
let flag_r = { isReadable: function() { return true; },
    isWriteable: function() { return false; },
    isTruncating: function() { return false; },
    isAppendable: function() { return false; },
    isSynchronous: function() { return false; },
    isExclusive: function() { return false; },
    pathExistsAction: function() { return 0; },
    pathNotExistsAction: function() { return 1; }
};
        
window.onerror = function() {
    Module.setStatus('Exception thrown, see JavaScript console');
    spinnerElement.style.display = 'none';
    Module.setStatus = function(text) {
      if (text) Module.printErr('[post-exception status] ' + text);
    };
};
function logAndExit(msg) {
    console.log("FATAL ERROR: " + msg);
    throw new Error(msg);
}
var initialSetup = function(){
    Module["addRunDependency"]("setupBoxedWine");

    Config.appDirPrefix = DEFAULT_APP_DIRECTORY;
    Config.rootZipFile = "boxedwine.zip";
    Config.appZipFile = "chomp.zip";
    Config.Program = "CHOMP.EXE";

    let writableStorage = new BrowserFS.FileSystem.InMemory();
    var Buffer = BrowserFS.BFSRequire('buffer').Buffer;
    buildAppFileSystems(function(homeAdapter) {
        var rootListingObject = {};
        rootListingObject[Config.rootZipFile] =  null;
        BrowserFS.FileSystem.XmlHttpRequest.Create({"index":rootListingObject, "baseUrl": Config.locateRootBaseUrl}, function(e2, xmlHttpFs){
            if(e2){
                logAndExit(e2);
            }
            var rootMfs = new BrowserFS.FileSystem.MountableFileSystem();
            rootMfs.mount('/temp', xmlHttpFs);
            rootMfs.readFile('/temp/' + Config.rootZipFile, null, flag_r, function callback(e, contents){
                if(e){
                    logAndExit(e);
                }
                BrowserFS.FileSystem.ZipFS.Create({"zipData":new Buffer(contents)}, function(e3, zipfs){
                    if(e3){
                        logAndExit(e3);
                    }
                    buildBrowserFileSystem(writableStorage, homeAdapter, zipfs);
                });
                rootMfs = null;
            });
        });
    });
}
function buildAppFileSystems(adapterCallback)
{
    var Buffer = BrowserFS.BFSRequire('buffer').Buffer;
    var listingObject = {};
    listingObject[Config.appZipFile] =  null;
    var mfs = new BrowserFS.FileSystem.MountableFileSystem();
    BrowserFS.FileSystem.XmlHttpRequest.Create({"index":listingObject, "baseUrl": Config.locateAppBaseUrl}, function(e2, xmlHttpFs){
        if(e2){
            logAndExit(e2);
        }
        mfs.mount('/temp', xmlHttpFs);
        mfs.readFile('/temp/' + Config.appZipFile, null, flag_r, function callback(e, contents){
            if(e){
                logAndExit(e);
            }
            BrowserFS.FileSystem.ZipFS.Create({"zipData":new Buffer(contents)}, function(e3, additionalZipfs){
                if(e3){
                    logAndExit(e3);
                }
                let homeAdapter = new BrowserFS.FileSystem.FolderAdapter("/", additionalZipfs);
                adapterCallback(homeAdapter);
                mfs = null;
            });
        });
    });
}
function buildBrowserFileSystem(writableStorage, homeAdapter, zipfs)
{
    FS.createPath(FS.root, 'root', FS.createPath);
    FS.createPath("/root", 'base', true, true);
    FS.createPath("/root", 'files', true, true);
    var mainfs = null;

    BrowserFS.FileSystem.OverlayFS.Create({"readable":zipfs,"writable":new BrowserFS.FileSystem.InMemory()}, function(e3, rootOverlay){
        if(e3){
            logAndExit(e3);
        }
        deleteFile(rootOverlay, "/lib/wine/wineboot.exe.so");

        homeAdapter.initialize(function callback(e){
            if(e){
                logAndExit(e);
            }
            BrowserFS.FileSystem.OverlayFS.Create({"readable":homeAdapter,"writable":writableStorage}, function(e2, homeOverlay){
                if(e2){
                    logAndExit(e2);
                }
                postBuildFileSystem(rootOverlay, homeOverlay);
            });
        });
    });
}
function postBuildFileSystem(rootFS, homeFS)
{
    var mfs = new BrowserFS.FileSystem.MountableFileSystem();
    mfs.mount('/root/base', rootFS);
    mfs.mount( Config.appDirPrefix.substring(0, Config.appDirPrefix.length - 1), homeFS);
    var BFS = new BrowserFS.EmscriptenFS();

    BrowserFS.initialize(mfs);
    FS.mount(BFS, {root: '/root'}, '/root');

    var params = getEmulatorParams();
    for(var i=0; i < params.length; i++) {
        Module['arguments'].push(params[i]);
    }
    Module["removeRunDependency"]("setupBoxedWine");
}
function getEmulatorParams() {
    var params = ["-root", "/root/base"];
    params.push("-mount_drive");
    params.push(Config.appDirPrefix);
    params.push("d");
    params.push("-nozip");

    //params.push("-resolution");
    //params.push(Config.resolution);

    params.push("-nosound");
    var subDirectory = Config.appZipFile.substring(0, Config.appZipFile.lastIndexOf("."));
    params.push("-w");
    if(isInSubDirectory(Config.appDirPrefix, subDirectory)){
        params.push("/home/username/.wine/dosdevices/d:/" + subDirectory);
    }else{
        params.push("/home/username/.wine/dosdevices/d:");
    }
    params.push("/bin/wine");
    params.push(Config.Program);
    console.log("Emulator params:" + params);
    return params;
}
function isInSubDirectory(fullPath, programDir) {
    var fileEntry = FS.lookupPath(fullPath, { follow: true });
    if (fileEntry!= null && fileEntry.node.isFolder) {
        var entries = FS.readdir(fullPath).filter(function(param) {
            return param !== "." && param !== ".." && param !== "__MACOSX";
        });
        for(var idx = 0; idx < entries.length; idx++) {
            if(entries[idx] === programDir) {
                return true;
            }
        }
    }
    return false;
}
function deleteFile(fs, pathAndFilename)
{
    try {
        fs.unlinkSync(pathAndFilename);
    }catch(ef) {
        console.log("Unable to delete:" + pathAndFilename + " error:" + ef.message);
    }
}
var Module = {
    preRun: [initialSetup],
    postRun: [],
    arguments: [],
    print: (function() {
      var element = document.getElementById('output');
      if (element) element.value = ''; // clear browser cache
      return function(text) {
        if (arguments.length > 1) text = Array.prototype.slice.call(arguments).join(' ');
        // These replacements are necessary if you render to raw HTML
        //text = text.replace(/&/g, "&amp;");
        //text = text.replace(/</g, "&lt;");
        //text = text.replace(/>/g, "&gt;");
        //text = text.replace('\n', '<br>', 'g');
        console.log(text);
        if (element) {
          element.value += text + "\n";
          element.scrollTop = element.scrollHeight; // focus on bottom
        }
      };
    })(),
    printErr: function(text) {
      if (arguments.length > 1) text = Array.prototype.slice.call(arguments).join(' ');
      console.error(text);
    },
    canvas: (function() {
      var canvas = document.getElementById('canvas');

      // As a default initial behavior, pop up an alert when webgl context is lost. To make your
      // application robust, you may want to override this behavior before shipping!
      // See http://www.khronos.org/registry/webgl/specs/latest/1.0/#5.15.2
      canvas.addEventListener("webglcontextlost", function(e) { alert('WebGL context lost. You will need to reload the page.'); e.preventDefault(); }, false);

      return canvas;
    })(),
    setStatus: function(text) {
      if (!Module.setStatus.last) Module.setStatus.last = { time: Date.now(), text: '' };
      if (text === Module.setStatus.last.text) return;
      var m = text.match(/([^(]+)\((\d+(\.\d+)?)\/(\d+)\)/);
      var now = Date.now();
      if (m && now - Module.setStatus.last.time < 30) return; // if this is a progress update, skip it if too soon
      Module.setStatus.last.time = now;
      Module.setStatus.last.text = text;
      if (m) {
        text = m[1];
        progressElement.value = parseInt(m[2])*100;
        progressElement.max = parseInt(m[4])*100;
        progressElement.hidden = false;
        spinnerElement.hidden = false;
      } else {
        progressElement.value = null;
        progressElement.max = null;
        progressElement.hidden = true;
        if (!text) spinnerElement.hidden = true;
      }
      statusElement.innerHTML = text;
    },
    totalDependencies: 0,
    monitorRunDependencies: function(left) {
      this.totalDependencies = Math.max(this.totalDependencies, left);
      Module.setStatus(left ? 'Preparing... (' + (this.totalDependencies-left) + '/' + this.totalDependencies + ')' : 'All downloads complete.');
    }
};
Module.setStatus('Downloading...');