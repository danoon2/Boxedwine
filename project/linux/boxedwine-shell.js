        let ALLOW_PARAM_OVERRIDE_FROM_URL = true;
        let SUPPRESS_WINEBOOT = true; //prevent wine from re-creating .wine directory
        let ROOT = "/root";
        let DLL_DIRECTORY = ROOT + "/lib/wine";
        let STORAGE_DROPBOX = "DROPBOX";
        let STORAGE_LOCAL_STORAGE = "LOCAL_STORAGE";
        let STORAGE_MEMORY = "MEMORY";

        let DROPBOX_APP_KEY = 'put key in here';
        let RECEIVE_URL = "http://put url in here/oauth_receiver.html";

        let DEFAULT_AUTO_RUN = true;
        let DEFAULT_SOUND_ENABLED = true;
        let DEFAULT_ZLIB_ENABLED = false;
        let DEFAULT_RETRIEVE_REMOTE_DLL_FILES = false;
        let DEFAULT_HOME_DIRECTORY = ROOT + "/home/username/files/";
        let DEFAULT_BPP = 32;
        let DEFAULT_ROOT_ZIP_FILE = "boxedwine.zip";
        //params
        let Config = {};
        Config.urlParams = "";
        Config.storageMode = STORAGE_MEMORY;
        Config.isRunningInline = false;
        Config.showUploadDownload = false;

        var isRunning = false;
        var uniqueDirs = {};
        var timer = null;
    	var index = 0;
		var selectedItem;
		var selectedFilename;
		var files = []; //used for constructing tree and for retrieving files when zipping
        var client = null;
        var alreadyBuiltFileSystem = false;

		var ae = document.createElement("a");
		document.body.appendChild(ae);
		ae.style = "display: none";
		var url = null;
      	var statusElement = document.getElementById('status');
      	var progressElement = document.getElementById('progress');
      	var spinnerElement = document.getElementById('spinner');
        var dropzone = document.getElementById("dropzone");


    //recursive copy based on code in emularity github project
    var flag_r = { isReadable: function() { return true; },
        isWriteable: function() { return false; },
        isTruncating: function() { return false; },
        isAppendable: function() { return false; },
        isSynchronous: function() { return false; },
        isExclusive: function() { return false; },
        pathExistsAction: function() { return 0; },
        pathNotExistsAction: function() { return 1; }
    };
        function setConfiguration() {
            Config.dirPrefix = getFileDirectory();
            Config.isAutoRunSet = getAutoRun();
            Config.extraZipFiles = getZipFileList("overlay"); //MANUAL:"dlls.zip;fonts.zip";
            Config.appZipFile = getAppZipFile("app"); //MANUAL:"chomp.zip";
            Config.rootZipFile = getRootZipFile("root"); //MANUAL:"base.zip";
            Config.Program = getExecutable(); //MANUAL:"CHOMP.EXE";
            Config.WorkingDir = getWorkingDirectory(); //MANUAL:"";
            Config.retrieveDlls = getRetrieveDlls();
            Config.isSoundEnabled = getSound();
            Config.isZlibEnabled = getZlibParameter();
            Config.bpp = getBitsPerPixel();
        }
        function allowParameterOverride() {
            if(Config.urlParams.length >0) {
                return true;
            }
            return ALLOW_PARAM_OVERRIDE_FROM_URL;
        }
        function getBitsPerPixel(){

            var bpp =  getParameter("bpp");
            if(!allowParameterOverride()){
                bpp = DEFAULT_BPP;
            }else if(bpp == "8") {
                bpp = 8;
            }else if(bpp == "16") {
                bpp = 16;
            }else if(bpp == "32"){
                bpp = 32;
            }else{
                bpp = DEFAULT_BPP;
            }
            console.log("setting BPP to: "+bpp);
            return bpp;
        }
        function getAutoRun(){

            var auto =  getParameter("auto");
            if(!allowParameterOverride()){
                auto = DEFAULT_AUTO_RUN;
            }else if(auto == "true") {
                auto = true;
            }else if(auto == "false"){
                auto = false;
            }else{
                auto = DEFAULT_AUTO_RUN;
            }
            if(!auto && Config.isRunningInline){
                console.log("parameter mismatch. Auto run can't be false if running inline. Resetting auto run to true");
                auto = true;
            }
            console.log("setting auto run to: "+auto);
            return auto;
        }
        function getRetrieveDlls(){
            var retrieveEnabled =  getParameter("remote");
            if(!allowParameterOverride()){
                retrieveEnabled = DEFAULT_RETRIEVE_REMOTE_DLL_FILES;
            }else if(retrieveEnabled == "true") {
                retrieveEnabled = true;
            }else if(retrieveEnabled == "false"){
                retrieveEnabled = false;
            }else{
                retrieveEnabled = DEFAULT_RETRIEVE_REMOTE_DLL_FILES;
            }
            console.log("setting retrieve remote Dlls to: "+retrieveEnabled);
            return retrieveEnabled;
        }
        function getSound(){
            var soundEnabled =  getParameter("sound");
            if(!allowParameterOverride()){
                soundEnabled = DEFAULT_SOUND_ENABLED;
            }else if(soundEnabled == "true") {
                soundEnabled = true;
            }else if(soundEnabled == "false"){
                soundEnabled = false;
            }else{
                soundEnabled = DEFAULT_SOUND_ENABLED;
            }
            console.log("setting sound to: "+soundEnabled);
            return soundEnabled;
        }
        function getZlibParameter(){
            var zlibEnabled =  getParameter("zlib");
            if(!allowParameterOverride()){
                zlibEnabled = DEFAULT_ZLIB_ENABLED;
            }else if(zlibEnabled == "true") {
                zlibEnabled = true;
            }else if(zlibEnabled == "false"){
                zlibEnabled = false;
            }else{
                zlibEnabled = DEFAULT_ZLIB_ENABLED;
            }
            console.log("setting use of internal boxedwine zip filesystem to: "+zlibEnabled);
            return zlibEnabled;
        }
        function getFileDirectory(){

            var dir =  getParameter("dir");
            if(!allowParameterOverride() || dir===""){
                dir = DEFAULT_HOME_DIRECTORY;
            }else{
                if(!dir.endsWith("/")){
                    dir = dir + "/";
                }
                if(!dir.startsWith(ROOT)) {
                    console.log("file directory must start at /root");
                    dir = DEFAULT_HOME_DIRECTORY;
                }
            }
            console.log("setting file directory to: "+dir);
            return dir;
        }
        function getWorkingDirectory(){

            var dir =  getParameter("work");
            if(!allowParameterOverride() || dir===""){
                dir = "";
            }else{
                if(dir.startsWith('c:/')){
                    dir = "/home/username/.wine/c_drive/" + dir.substring(3);
                }else{
                    dir = Config.dirPrefix.substring(ROOT.length) + dir;
                }
                console.log("setting working directory to: "+dir);
            }
            return dir;
        }
        function getAppZipFile(param){

            var filename =  getParameter(param);
            if(!allowParameterOverride() || filename===""){
                filename = "";
                console.log("not setting " + param + " zip file");
            }else{
                if(!filename.endsWith(".zip")){
                    filename = filename + ".zip";
                }
                console.log("setting " + param + " zip file to: "+filename);
            }
            return filename;
        }
        function getRootZipFile(param){

            var filename =  getParameter(param);
            if(!allowParameterOverride() || filename===""){
                filename = DEFAULT_ROOT_ZIP_FILE;
            }else{
                if(!filename.endsWith(".zip")){
                    filename = filename + ".zip";
                }
            }
            console.log("setting " + param + " zip file to: "+filename);
            return filename;
        }
        function getZipFileList(param){
            var zipFiles = [];
            var filenames =  getParameter(param);
            if(!allowParameterOverride() || filename===""){
                console.log("not setting " + param + " zip file(s)");
            }else{
                if(filenames.length > 0) {
                    var zipFilenames = filenames.split(';');
                    for(var i=0; i < zipFilenames.length;i++) {
                        var filename = zipFilenames[i];
                        if(!filename.endsWith(".zip")){
                            filename = filename + ".zip";
                        }
                        zipFiles.push(filename);
                    }
                    console.log("setting " + param + " zip file(s) to: "+zipFiles);
                }
            }
            return zipFiles;
        }
        function auth_callback(error) {
            if (error) {
                alert('Authentication error: ' + error);
                return;
            }
            if (client.isAuthenticated()) {
                if(alreadyBuiltFileSystem){
                    return;
                }
                alreadyBuiltFileSystem = true;
                document.getElementById('startbtn').disabled = true;
                document.getElementById('startbtn').textContent = "Syncing...";
                console.log("authenticated!");
                buildFileSystem(new BrowserFS.FileSystem.InMemory(), true);
            } else {
                alert('unable to authenticate');
            }
        }
        function dropboxLogin()
        {
            client = new Dropbox.Client({key: DROPBOX_APP_KEY});
            client.authDriver(new Dropbox.AuthDriver.Popup({receiverUrl: RECEIVE_URL}));
            client.authenticate(auth_callback);
            document.getElementById('startbtn').textContent = "Start";
        }

        function initFileSystem()
        {
            console.log("Use Storage mode: "+Config.storageMode);
            if(Config.storageMode === STORAGE_LOCAL_STORAGE){
                var writableStorage;
                if(BrowserFS.FileSystem.LocalStorage.isAvailable){
                    writableStorage = new BrowserFS.FileSystem.LocalStorage();
                }else{
                    writableStorage = new BrowserFS.FileSystem.InMemory();
                    console.log("Switching to In Memory store as LocalStorage is not available");
                }
                buildFileSystem(writableStorage, false);
            }else if(Config.storageMode === STORAGE_DROPBOX){
                client.authenticate({interactive:false}, auth_callback);
            }else{
                buildFileSystem(new BrowserFS.FileSystem.InMemory(), false);
            }
        }

        function buildFileSystem(writableStorage, isDropBox)
        {
            spinnerElement.style.display = '';
            spinnerElement.hidden = false;
            var Buffer = BrowserFS.BFSRequire('buffer').Buffer;

            buildExtraFileSystems(Buffer, function(extraFSs) {
                buildAppFileSystems(function(homeAdapter) {
                    var rootMfs = new BrowserFS.FileSystem.MountableFileSystem();

                    var rootListingObject = {};
                    rootListingObject[Config.rootZipFile] =  null;

                    BrowserFS.FileSystem.XmlHttpRequest.Create({"index":rootListingObject, "baseUrl":""}, function(e2, xmlHttpFs){
                        if(e2){
                            console.log(e2);
                        }
                        rootMfs.mount('/temp', xmlHttpFs);
                        rootMfs.readFile('/temp/' + Config.rootZipFile, null, flag_r, function callback(e, contents){
                            if(e){
                                console.log(e);
                            }
                            if(Config.isZlibEnabled) {
                                Module.FS_createDataFile("/", Config.rootZipFile, contents, true, true);
                                contents = null;
                                buildBrowserFileSystem(writableStorage, isDropBox, homeAdapter, extraFSs);
                            }else{
                                BrowserFS.FileSystem.ZipFS.Create({"zipData":new Buffer(contents)}, function(e3, zipfs){
                                    if(e3){
                                        console.log(e3);
                                    }
                                    buildBrowserFileSystem(writableStorage, isDropBox, homeAdapter, extraFSs, zipfs);
                                });
                            }
                            rootMfs = null;
                        });
                    });
                });
            });
        }
        function buildAppFileSystems(adapterCallback)
        {
            var Buffer = BrowserFS.BFSRequire('buffer').Buffer;
            if(Config.appZipFile.length > 0){
                var listingObject = {};
                listingObject[Config.appZipFile] =  null;
                var mfs = new BrowserFS.FileSystem.MountableFileSystem();
                BrowserFS.FileSystem.XmlHttpRequest.Create({"index":listingObject, "baseUrl":""}, function(e2, xmlHttpFs){
                    if(e2){
                        console.log(e2);
                    }
                    mfs.mount('/temp', xmlHttpFs);
                    mfs.readFile('/temp/' + Config.appZipFile, null, flag_r, function callback(e, contents){
                        if(e){
                            console.log(e);
                        }
                        BrowserFS.FileSystem.ZipFS.Create({"zipData":new Buffer(contents)}, function(e3, additionalZipfs){
                            if(e3){
                                console.log(e3);
                            }
                            homeAdapter = new BrowserFS.FileSystem.FolderAdapter("/", additionalZipfs);
                            adapterCallback(homeAdapter);
                            mfs = null;
                        });
                    });
                });
            }else{
                homeAdapter = new BrowserFS.FileSystem.FolderAdapter("/", new BrowserFS.FileSystem.InMemory());
                adapterCallback(homeAdapter);
            }
        }
        function buildExtraFileSystems(Buffer, fsCallback)
        {
            var extraFSs = [];
            if(Config.extraZipFiles.length > 0){
                for(let i = 0; i < Config.extraZipFiles.length; i++) {
                    var listingObject = {};
                    listingObject[Config.extraZipFiles[i]] =  null;
                    var mfs = new BrowserFS.FileSystem.MountableFileSystem();
                    BrowserFS.FileSystem.XmlHttpRequest.Create({"index":listingObject, "baseUrl":""}, function(e2, xmlHttpFs){
                        if(e2){
                            console.log(e2);
                        }
                        mfs.mount('/temp', xmlHttpFs);
                        mfs.readFile('/temp/' + Config.extraZipFiles[i], null, flag_r, function(e, contents){
                            if(e){
                                console.log(e);
                            }
                            BrowserFS.FileSystem.ZipFS.Create({"zipData":new Buffer(contents)}, function(e3, zipfs){
                                if(e3){
                                    console.log(e3);
                                }
                                extraFSs.push(zipfs);
                                if(extraFSs.length == Config.extraZipFiles.length) {
                                    fsCallback(extraFSs);
                                }
                                mfs = null;
                            });
                        });
                    });
                }
            }else{
                fsCallback(extraFSs);
            }
        }

        function buildDllFileSystem(fsDllCallback)
        {
            if(!Config.retrieveDlls) {
                return fsDllCallback(null);
            }
            BrowserFS.FileSystem.XmlHttpRequest.Create({"index":"dlls.json"}, function(e2, xmlHttpFs){
                if(e2){
                    console.log(e2);
                }
                var adapter = new BrowserFS.FileSystem.FolderAdapter("/dlls", xmlHttpFs);
                adapter.initialize(function dllcallback(de){
                    if(de){
                        console.log(de);
                    }
                    BrowserFS.FileSystem.OverlayFS.Create({"readable":adapter,"writable":new BrowserFS.FileSystem.InMemory()}, function(oe, overlay){
                        if(oe){
                            console.log(oe);
                        }
                        fsDllCallback(overlay);
                    });
                });
            });
        }
        function buildBrowserFileSystem(writableStorage, isDropBox, homeAdapter, extraFSs, zipfs)
        {
            FS.createFolder(FS.root, 'root', true, true);
            var mainfs = null;
            if(Config.isZlibEnabled) {
                mainfs = new BrowserFS.FileSystem.InMemory();
                createFolders(mainfs, Config.dirPrefix.substring(ROOT.length).split("/"));
                createFolders(mainfs, DLL_DIRECTORY.substring(ROOT.length).split("/"));
            }else{
                mainfs = zipfs;
            }
            //i do not understand why i need the overlay when Config.isZlibEnabled = true
            BrowserFS.FileSystem.OverlayFS.Create({"readable":mainfs,"writable":new BrowserFS.FileSystem.InMemory()}, function(e3, rootOverlay){
                if(e3){
                    console.log(e3);
                }
                if(!Config.isZlibEnabled && SUPPRESS_WINEBOOT) {
                   deleteFile(rootOverlay, "/lib/wine/wineboot.exe.so");
                }

                buildDllFileSystem(function(dllFS) {
                    homeAdapter.initialize(function callback(e){
                        if(e){
                            console.log(e);
                        }
                        BrowserFS.FileSystem.OverlayFS.Create({"readable":homeAdapter,"writable":writableStorage}, function(e2, homeOverlay){
                            if(e2){
                                console.log(e2);
                            }
                            if(isDropBox) {
                                var mirrorFS = new BrowserFS.FileSystem.AsyncMirror(homeOverlay, new BrowserFS.FileSystem.Dropbox(client));
                                mirrorFS.initialize(function callback(e4){
                                    if(e4){
                                        console.log(e4);
                                    }
                                    postBuildFileSystem(rootOverlay, mirrorFS, extraFSs, dllFS);
                                });
                            }else{
                                postBuildFileSystem(rootOverlay, homeOverlay, extraFSs, dllFS);
                            }
                        });
                    });
                });
            });
        }
        function postBuildFileSystem(rootFS, homeFS, extraFSs, dllFS)
        {
            var dirPrefixWithoutEndSlash = Config.dirPrefix.substring(0, Config.dirPrefix.length - 1);
            var mfs = new BrowserFS.FileSystem.MountableFileSystem();
            if(Config.isZlibEnabled) {
                mfs.mount('/root', rootFS);
            }else{
                mfs.mount('/', rootFS);
            }
            mfs.mount(dirPrefixWithoutEndSlash, homeFS);
            if(dllFS != null) {
                mfs.mount(DLL_DIRECTORY, dllFS);
            }
            var BFS = new BrowserFS.EmscriptenFS();

            BrowserFS.initialize(mfs);
            FS.mount(BFS, {root: '/root'}, '/root');
            for(let i = 0; i < extraFSs.length; i++) {
                recursiveCopy(extraFSs[i], Config.extraZipFiles[i], '/');
            }
            extraFSs = null;

            if(Config.showUploadDownload){
                document.getElementById('uploadbtn').style.display = "";
                document.getElementById('downloadbtn').style.display = "";
            }
            spinnerElement.style.display = 'none';
            if(Config.storageMode === STORAGE_DROPBOX){
                startEmulator();
            }else{
                toggleConsole();
                if(Config.isAutoRunSet){
                    start();
                }else{
                    var startBtn = document.getElementById('startbtn');
                    startBtn.disabled = false;
                    startBtn.style.display = "";
                    var soundToggle = document.getElementById('soundToggle');
                    if(Config.isSoundEnabled){
                        soundToggle.checked = true;
                    }
                    document.getElementById('sound-checkbox').style.display = "";
                }
            }
        }
        function deleteFile(fs, pathAndFilename)
        {
            try {
                fs.unlinkSync("/root" + pathAndFilename);
            }catch(ef) {
                console.log("Unable to delete:" + "/root" + pathAndFilename + " error:" + ef.message);
            }
        }
        function createFolders(mainfs, folders)
        {
            var directory = "";
            for(var k = 0; k < folders.length; k++){
                if(folders[k].length > 0) {
                    directory = directory + "/" + folders[k];
                    mainfs.mkdirSync(directory);
                }
            }
        }
        function recursiveCopy(fs, zipFilename, filename) {

            var prefix = "/" + zipFilename.substring(0, zipFilename.length - 4);
            var path = BrowserFS.BFSRequire('path');
            copyDirectory(fs, filename, prefix);
            function copyDirectory(fs, filename, prefix) {
                createFolderIfNecessary(filename, prefix);
                fs.readdirSync(filename).forEach(function(item) {
                    var file = path.resolve(filename, item);
                    if(!(file.startsWith("/__MACOSX") || file.endsWith(".DS_Store"))) {
                        if (fs.statSync(file).isDirectory()) {
                            copyDirectory(fs, file, prefix);
                        } else {
                            createFileIfNecessary(fs, file, prefix);
                        }
                    }
                });
            }
        }
        function createFileIfNecessary(fs, fullPath, prefix)
        {
            var file = fullPath;
            if(fullPath.startsWith(prefix)) {
                fullPath = fullPath.substring(prefix.length);
            }
            var parent =  extractFirstPartOfPath(fullPath);
            if(parent.length > 0){
                var filename = extractLastPartOfPath(fullPath);
                try {
                    var contents = fs.readFileSync(file, null, flag_r);
                    Module.FS_createDataFile("root" + parent, filename, contents, true, true);
                }catch(ef) {
                    if(ef.message === "File exists"){
                        try {
                        Module.FS_unlink("root" + parent + "/" + filename);
                        var contents = fs.readFileSync(file, null, flag_r);
                        Module.FS_createDataFile("root" + parent, filename, contents, true, true);
                        }catch(ef) {
                            console.log("file replace error:" + ef.message + " for: " + parent + "/" + filename);
                        }
                    }else {
                        console.log("file creation error:" + ef.message + " for: " + parent + "/" + filename);
                    }
                }
            }
        }
        //todo use stat!
        function createFolderIfNecessary(fullPath, prefix)
        {

            if(fullPath.startsWith(prefix)) {
                fullPath = fullPath.substring(prefix.length);
            }
            var parent = extractFirstPartOfPath(fullPath);
            var dir = extractLastPartOfPath(fullPath);
            if(parent.length > 0){
                try{
                    FS.lookupPath("/root" + parent + "/" + dir, { follow: true });
                }catch(ef){
                    if(ef.message == "No such file or directory") {
                        try{
                            FS.createFolder("/root/" + parent, dir, true, true);
                        }catch(cef) {
                            console.log("Directory creation error:" + cef.message + " for: " + parent + "/" + dir);
                        }
                    }else if(ef.message != "File exists"){
                        console.log("Directory creation error:" + ef.message+" for: " + parent + "/" +  dir);
                    }
                }
            }
        }
        function start(){
            if(isRunning){
                return;
            }
            if(Config.isRunningInline) {
                document.getElementById('inline-runbtn').style.display = 'none';
                document.getElementById('inline').style.display = "";
            }
            if(Config.storageMode === STORAGE_DROPBOX){
                if(client == null || !client.isAuthenticated()){
                    dropboxLogin();
                }else{
                    initFileSystem();
                }
            }else{
                startEmulator();
            }
        }
        function startEmulator()
        {
            isRunning = true;

            document.getElementById('startbtn').style.display = 'none';
            document.getElementById('sound-checkbox').style.display = 'none';

            var params = getEmulatorParams();
            Module["arguments"] = params;

            document.getElementById('startbtn').textContent = "Running...";
            Module["removeRunDependency"]("setupBoxedWine");
        }
        function loadScreen() {
            if(Config.isAutoRunSet || Config.isRunningInline) {
                var canvas = document.getElementById("canvas");
                var ctx = canvas.getContext("2d");
                ctx.font = "40px Arial";
                ctx.textAlign = "center";
                ctx.fillText("Loading...", canvas.width/2.5, canvas.height/2);
            }
        }
        var initialSetup = function(){
            console.log("running initial setup");
            setConfiguration();
            //loadScreen();

            Module["addRunDependency"]("setupBoxedWine");
            if(Config.storageMode === STORAGE_DROPBOX){
                startBtn.textContent = "Login";
                startBtn.disabled = false;
                startBtn.style.display = "";
            }else{
                initFileSystem();
            }
        }
        function getExecutable()
        {
            var prog =  getParameter("p");
            if(!allowParameterOverride() || prog===""){
                prog = "taskmgr.exe";
                console.log("not setting program to execute");
            }else{
                if(prog.startsWith("%22") && prog.endsWith("%22")){
                    prog = prog.substring(3, prog.length - 3);
                }else if(prog.startsWith('%27') && prog.endsWith('%27')){
                    prog = prog.substring(3, prog.length - 3);
                }
                prog = prog.split('%20').join(' ');
                console.log("setting program to execute to: "+prog);
            }
            return prog;
        }
        var errorCallback = function(e){
            console.log(e);
        }
        var dirCount = 0;
        function getEntriesAsPromise(item, exeFiles, allFiles) {
            return new Promise((resolve, reject) => {
                if(item.isDirectory){
                    dirCount = dirCount + 1;
                    let reader = item.createReader();
                    let doBatch = () => {
                        reader.readEntries(entries => {
                            if (entries.length > 0) {
                                entries.forEach(function(entry){
                                    getEntriesAsPromise(entry, exeFiles, allFiles);
                                });
                                doBatch();
                            } else {
                                dirCount = dirCount - 1;
                                if(dirCount == 0){
                                    if(!Config.isAutoRunSet && !isRunning){
                                        loadExeModal(exeFiles, allFiles);
                                    }
                                }
                                resolve();
                            }
                        }, reject);
                    };
                    doBatch();
                }else{
                    let fullPath = item.fullPath;
                    let uppercase = fullPath.toUpperCase();
                    allFiles.push(fullPath);
                    if(uppercase.endsWith(".EXE") || uppercase.endsWith(".BAT")){
                        exeFiles.push(fullPath);
                    }
                    item.file(function(item){uploadFile(item, fullPath, allFiles);}, errorCallback);
                }
            });
        }
        function loadExeModal(exeFiles, allFiles)
        {
            document.getElementById('modalLinkExe').click();
            var message = document.getElementById('message');
            message.innerHTML = "<p>Uploading files...</p>";
            timer = setInterval(function(){readyCheck(exeFiles, allFiles);}, 100);
        }
        function populateModalExe(exeFiles)
        {
            var root = document.getElementById('items');
            root.innerHTML = '';
            let listElement = document.createElement("lu");
            for(let i = 0; i < exeFiles.length; i++) {
                let fullPath = exeFiles[i];
                let element = document.createElement("li");
                element.addEventListener("click", function(event){execute(fullPath);}, false);
                element.innerHTML = fullPath;
                listElement.appendChild(element);
            }
            root.appendChild(listElement);
        }
        function execute(filename)
        {
            var root = document.getElementById('items');
            document.getElementById('openModalExeClick').click();

            var file = extractLastPartOfPath(filename);
            var path = extractFirstPartOfPath(filename);

            Config.WorkingDir = Config.dirPrefix.substring(ROOT.length) + path.substring(1);
            Config.Program = file;

            startEmulator();
        }
        function readyCheck(exeFiles, allFiles)
        {
            if(allFiles.length==0){
                clearInterval(timer);
                var message = document.getElementById('message');
                message.innerHTML = '';
                populateModalExe(exeFiles);
            }
        }
        dropzone.addEventListener("dragover", function(event){
            event.preventDefault();
        }, false);
        dropzone.addEventListener("drop", function(event){
            event.preventDefault();
            //if only i know something about async
            let items = event.dataTransfer.items;
            let exeFiles = [];
            let allFiles = [];
            for(let i =0; i < items.length; i++){
                getEntriesAsPromise(items[i].webkitGetAsEntry(), exeFiles, allFiles);
            }
        }, false);
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
        function getEmulatorParams() {
            var params = ["-root", "/root"];
            if(Config.isZlibEnabled) {
                params.push("-zip");
                params.push(Config.rootZipFile);
            }
            if(!Config.isSoundEnabled){
                params.push("-nosound");
            }
            if(Config.bpp != DEFAULT_BPP){
                params.push("-bpp");
                params.push("" + Config.bpp);
            }
            if(Config.WorkingDir.length > 0){
                params.push("-w");
                params.push(Config.WorkingDir);
            }else if(Config.appZipFile.length > 0 && Config.Program.length > 0 && Config.Program.substring(0 ,1) != "/"){
                var subDirectory = Config.appZipFile.substring(0, Config.appZipFile.lastIndexOf("."));
                var programDir = Config.dirPrefix.substring(ROOT.length);
                params.push("-w");
                if(isInSubDirectory(Config.dirPrefix, subDirectory)){
                    params.push(programDir + subDirectory);
                }else{
                    params.push(programDir);
                }
            }
            if(Config.Program.length > 0){
                params.push("/bin/wine");
                if (Config.Program.endsWith('.bat')) {
                    params.push("cmd");
                    params.push("/c");
                }
                params.push(Config.Program);
            }
            console.log("Emulator params:" + params);
            return params;
        }
        var params = [];
      var Module = {
        logReadFiles : false, //enable if you want to prune with Reduce utility
        preRun: [initialSetup],
        arguments: params,
        postRun: [],
        print: (function() {
          var element = document.getElementById('output');
          if (element) element.value = ''; // clear browser cache
          return function(text) {
            text = Array.prototype.slice.call(arguments).join(' ');
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
          text = Array.prototype.slice.call(arguments).join(' ');
          if (0) { // XXX disabled for safety typeof dump == 'function') {
            dump(text + '\n'); // fast, straight to the real console
          } else {
            console.error(text);
          }
        },
        canvas: (function() {
          var canvas = document.getElementById('canvas');

          // As a default initial behavior, pop up an alert when webgl context is lost. To make your
          // application robust, you may want to override this behavior before shipping!
          // See http://www.khronos.org/registry/webgl/specs/latest/1.0/#5.15.2
          canvas.addEventListener("webglcontextlost", function(e) { alert('WebGL context lost. You will need to reload the page.'); e.preventDefault(); }, false);
          canvas.width  = 800;
          canvas.height = 600;
          return canvas;
        })(),
        setStatus: function(text) {
          if (!Module.setStatus.last) Module.setStatus.last = { time: Date.now(), text: '' };
          if (text === Module.setStatus.text) return;
          var m = text.match(/([^(]+)\((\d+(\.\d+)?)\/(\d+)\)/);
          var now = Date.now();
          if (m && now - Date.now() < 30) return; // if this is a progress update, skip it if too soon
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
          Module.setStatus(left ? 'Preparing... (' + (this.totalDependencies-left) + '/' + this.totalDependencies + ')' : '');
        }
      };
      Module.setStatus('Downloading...');
      window.onerror = function() {
        Module.setStatus('Exception thrown, see JavaScript console');
        spinnerElement.style.display = 'none';
        Module.setStatus = function(text) {
          if (text) Module.printErr('[post-exception status] ' + text);
        };
      };

        function isHomeDirectory(str){
            if(str.length >= 10){
                if(str.substring(0,10) === '/root/home'){
                    return true;
                }
            }
            return false;
        }
        function startWithFiles(files){
            for (let i = 0; i < files.length; i++) {
                uploadFile(files[i]);
            }
        }
        function uploadFile(file, fullPath, allFiles)
        {
            let filename = null;
            if(fullPath){
                filename = fullPath;
            }else{
                filename = file.webkitRelativePath.length == 0 ? file.name : file.webkitRelativePath;
                filename = "/" + filename;
            }
            var filereader = new FileReader();
            filereader.file_name = file.name;
            filereader.onload = function(){readFile(this.result, filename, allFiles)};
            filereader.readAsArrayBuffer(file);
        }
        function extractLastPartOfPath(str){
                return str.substring(str.lastIndexOf("/")+1,str.length);
        }
        function extractFirstPartOfPath(str){
                return str.substring(0,str.lastIndexOf("/"));
        }
        function extractFilenameExtension(str){
                return str.substring(str.lastIndexOf(".")+1,str.length);
        }
        function extractFilenameWithoutExtension(str){
                return str.substring(0, str.lastIndexOf("."));
        }
        function readFile(data, name, allFiles){
            var fileExt = extractFilenameExtension(name);
            if(fileExt.toLowerCase() === 'zip'){
                var filenameNoExt = extractFilenameWithoutExtension(name);
                if(!createFolder(Config.dirPrefix, filenameNoExt)){//If dir exists and user says no to replace existing dir, then stop here
                    return;
                }
                var zipDirPrefix = Config.dirPrefix + filenameNoExt;
                var zip = new JSZip(data);
                for(var entry in zip.files)
                {
                    var data = zip.file(entry);
                    if(data != null){
                        var buf = data.asUint8Array();
                        var parent = zipDirPrefix + "/" + extractFirstPartOfPath(entry) ;
                        var filename = extractLastPartOfPath(entry);
                        createFile(parent, filename, buf);
                    }else{ //directory
                        var fullPath = entry.substring(0,entry.length-1);
                        var parent = extractFirstPartOfPath(fullPath);
                        var dir = extractLastPartOfPath(fullPath);
                        if(parent.length == 0){
                            parent = zipDirPrefix;
                        }else{
                            parent = zipDirPrefix + "/" + parent;
                        }
                        createFolder(parent, dir);
                    }
                }
            }else{
                var done = false;
                var startIndex = 0;
                var base = Config.dirPrefix;
                var filename = extractLastPartOfPath(name);
                while(!done){
                    var dirIndex = name.indexOf("/", startIndex);
                    if(dirIndex == -1){
                        done =true;
                    }else{
                        var dirName = name.substring(startIndex, dirIndex);
                        var key = base + dirName;
                        if(dirName.length > 0) {
                            if(uniqueDirs[key] == null && dirName.length > 0){
                                createFolder(base, dirName);
                                uniqueDirs[key] = "";
                            }
                            base = base + dirName + "/";
                        }
                        startIndex = dirIndex + 1;
                    }
                }
                createFile(base.substring(0,base.length-1), filename, new Uint8Array(data));
                if(allFiles){
                    allFiles.pop(name);
                }
            }
        }
function calcBackupFilename()
{
    var d = new Date();
    var str =d.toISOString();
    str = ".backup." + str.split(":").join(".");
    return str;
}
function createFolder(parent, dir)
{
    var created = true;
    try{
        Module.FS_createFolder(parent, dir, true, true);
        //console.log(entry + " is a dir parent="+parent+" dir="+dir);
        //console.log("Directory created :" + parent + "/" +  dir);
    }catch(ef){
      if(ef.message === "File exists"){
        console.log("Directory already exists! :" + parent + dir);
        var replace = confirm("Directory already exists: " + parent + dir+" continue?");
        if(replace){
            try{
                //yeah, like that would work! FS.rmdir(parent + dir);
                FS.rename(parent + dir,parent + dir + calcBackupFilename());
                Module.FS_createFolder(parent, dir, true, true);
                console.log("Directory replaced: " + parent + dir);
            }catch(eef){
                console.log("eef="+eef);
                created = false;
                alert("unable to create folder: "+ parent + dir);
            }
        }else{
            created = false;
        }
      }else{
        console.log("ef="+ef);
      }
    }
    return created;
}
function createFile(dir, name, buf)
{
    try{
        Module.FS_createDataFile(dir, name, buf, true, true);
        //console.log("File created :" + dir + "/" + name);
    }catch(e){
      if(e.message === "File exists"){
        console.log("File already exists!: " + dir + name);
        var replace = confirm("File already exists: " + dir + name+" replace?");
        if(replace){
            try{
                FS.unlink(dir + name);
                Module.FS_createDataFile(dir, name, buf, true, true);
                console.log("File replaced: " + dir + name);
            }catch(ee){
                console.log("ee="+ee);
                alert("unable to create file: "+ dir + name);
            }
        }
      }else{
        console.log("e="+e);
      }
    }
}
function toggleConsole() {
    var el = document.getElementById('showConsole');
    var console = document.getElementById('output');
    if(el.checked){
        console.style.display = '';
    }else{
        console.style.display = 'none';
    }
}
function toggleSound() {
    var el = document.getElementById('soundToggle');
    Config.isSoundEnabled = el.checked;
}
function toggleDirectory(item){
	var itemWidget =document.getElementById(item);
	if(itemWidget!=null){
		if(itemWidget.style.display=='none'){//show
			itemWidget.style.display="";
			document.getElementById(item+'-expand').style.display="none";
			document.getElementById(item+'-contract').style.display="";
		}else{//hide
			itemWidget.style.display="none";
			document.getElementById(item+'-expand').style.display="";
			document.getElementById(item+'-contract').style.display="none";
		}
	}
}
function getParameter(inputKey){
    var retVal="";
    var replacementParameters = Config.urlParams;
    var url = replacementParameters.length > 0 ? "?" + replacementParameters : window.location.href;
    var index = url.indexOf("?")+1;
    if(index > 0){
        var paramStr = url.substring(index);
        var params = paramStr.split("&");
        for(var x=0;x<params.length;x++){
            var param = params[x];
            var kv = param.split("=");
            var key = kv[0];
            if(key === inputKey){
                retVal = kv[1];
                break;
            }
        }
    }
    var hashIndex = retVal.lastIndexOf('#');
    if(hashIndex > 0 ) {
        retVal = retVal.substring(0, hashIndex);
    }
    return retVal;
}
function select(index, dir, filename){
	if(selectedItem != null){
		selectedItem.style.backgroundColor = "";
	}
	selectedItem = document.getElementById(index + '-data');
	selectedItem.style.backgroundColor="#94c2c5";
	var fullpath = dir;
	if(filename != null){
		fullpath = fullpath + filename;
	}
	document.getElementById('selectedItem').value = fullpath
	selectedFilename = filename
}
function endsWith(str, suffix){
    return str.indexOf(suffix, str.length - suffix.length) !== -1;
}
function extract(){
	if(url != null){
		window.URL.revokeObjectURL(url);
	}
	var file = document.getElementById('selectedItem').value;
	if(file != null && file.length > 0 && files.length > 1) {
        if(endsWith(file,"/")){
            file = file.substring(0,file.length - 1);
        }
		var isDirectory = false;
		var outputFilename;
		if(selectedFilename !=null){
			outputFilename = selectedFilename;
		}else{
			isDirectory = true;
			outputFilename = file.substring(file.lastIndexOf('/') + 1) + ".zip";
		}

		var blob = getFile(file, isDirectory);
		url = window.URL.createObjectURL(blob);
		ae.href = url;
		ae.download = outputFilename;
		ae.click();
	}
}
function done(){
	if(url != null){
		window.URL.revokeObjectURL(url);
	}
}
function leaf(entry){
    index++;
	var text = "<tr><td ><span id=\"" + index + "-data\" onclick=\"select(" + index + ",'" + entry.dir + "','" + entry.filename + "')\">" + entry.filename + "</span></td></tr>";
	return text;
}
function branch(entries){
	var item = entries[index];
    index++;
	var dir = item.dir;
    var dirName = dir.substring(0, dir.length - 1);
    dirName = dirName.substring(dirName.lastIndexOf("/")+1,dirName.length);
	var text = "<tr>";
    text = text + "<td>";
	text = text + "<span id=\"" + index + "-expand\"><a onclick=\"toggleDirectory('" + index + "')\"><strong>+</strong></a></span>";
    text = text + "<span id=\"" + index + "-contract\" style=\"display:none;\"><a onclick=\"toggleDirectory('" + index + "')\"><strong>-</strong></a></span>";
    text = text + "<span id=\"" + index + "-data\" onclick=\"select(" + index + ",'" + dir + "', null)\">[" + dirName + "]</span>";
    text = text + "<div id='" + index + "' style=\"display:none;\">";
    text = text + "<table>";
    while(index < entries.length){
    	var nextItem = entries[index];
    	if(nextItem.dir === item.dir){
    		text = text + leaf(nextItem);
    	}else if(parentDir(nextItem.dir) === item.dir){
    		text = text + branch(entries, index);
    	}else{
    		break;
    	}
    }
    text = text + "</table>";
    text = text + "</div>";
	text = text + "</td>";
	text = text + "</tr>";
	return text;
}
function parentDir(childDir)
{
    if(endsWith(childDir,"/")){
        childDir = childDir.substring(0,childDir.length - 1);
    }
    var parentDir= childDir.substring(0, childDir.lastIndexOf('/') + 1);
    return parentDir;
}
function buildTree()
{
    document.getElementById('modalLink').click();
    var root = document.getElementById('tree');
    //reset
    document.getElementById('selectedItem').value = "";
    selectedFilename = null;
	files = [];
    root.innerHTML = "";
    index = 0;

    var currentDir = Config.dirPrefix;
	readFiles(currentDir, files);

	//now build tree
	var contents = "<table>";
	contents = contents + branch(files);
	contents = contents + "</table>";
	document.getElementById('loadStatus').style.display="none";

	root.innerHTML = contents;
	toggleDirectory('1');
}

function readFiles(currentDir, files)
{
    console.log("adding directory: " + currentDir);
    files.push({dir : currentDir, filename : ""});
    var entries = FS.readdir(currentDir).filter(function(param) {
        return param !== "." && param !== "..";
    });
    entries.forEach(function(entry) {
        var fileEntry = FS.lookupPath(currentDir + entry, { follow: true });
        if (fileEntry.node.isFolder) {
            readFiles(currentDir + entry + "/", files);
        }else{
            console.log("adding file: " + currentDir + entry);
            files.push({dir : currentDir, filename : entry});
        }
    });
}

function startsWith(str, prefix)
{
    return str.slice(0, prefix.length) == prefix;
}
function getFile(file, isDirectory)
{
	if(isDirectory){//zip up directory
		var zip = new JSZip();
		files.forEach(function(eachFile) {
            if(startsWith(eachFile.dir, file)){
                if(eachFile.filename !== ""){
                      var fileLocation = eachFile.dir + eachFile.filename;
                      var data = FS.readFile(fileLocation, { encoding: 'binary' });
                      zip.file(fileLocation.substring(file.length), data);
                }else{
                        zip.file(eachFile.dir.substring(file.length), null, {dir: true});
                }
            }
		});
		return zip.generate({type:"blob", compression:"DEFLATE"});
	}else{
		var data = FS.readFile(file, { encoding: 'binary' });
		return new Blob([data], {type: "octet/stream"});
	}
}
