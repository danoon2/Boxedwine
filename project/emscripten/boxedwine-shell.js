        let ALLOW_PARAM_OVERRIDE_FROM_URL = true;
        let ROOT = "/root";
        let STORAGE_INDEXED_DB = "INDEXED_DB";
        let STORAGE_MEMORY = "MEMORY";

        let DEFAULT_AUTO_RUN = true;
        let DEFAULT_LOAD_DESKTOP = false;
        let DEFAULT_SOUND_ENABLED = true;
        let DEFAULT_DISABLE_HIDE_CURSOR = false;
        let DEFAULT_APP_DIRECTORY = "/home/username/.wine/dosdevices/c:/files";
        let DEFAULT_BPP = 32;
        let DEFAULT_FRAME_SKIP = "0";
        let DEFAULT_ROOT_ZIP_FILE = "boxedwine.zip";
        //params
        let Config = {};
        Config.locateRootBaseUrl = ""; // ie "assets/"
        Config.locateAppBaseUrl = "";
        Config.locateOverlayBaseUrl = "";
        Config.urlParams = "";
        Config.storageMode = STORAGE_INDEXED_DB;
        Config.persist_d_drive = true;
        Config.showUploadDownload = false;
        Config.showSaveJITCache = true;
        Config.WorkingDir = "";
        Config.loadDesktop = false;
        Config.appSubfolder = "";
				
        var isRunning = false;
        var ExeFileTimer = null;

      	var statusElement = document.getElementById('status');
      	var progressElement = document.getElementById('progress');
      	var spinnerElement = document.getElementById('spinner');
        var dropzone = document.getElementById("dropzone");

        function setConfiguration() {
            Config.appDirPrefix = DEFAULT_APP_DIRECTORY;
            Config.isAutoRunSet = getAutoRun();
            Config.loadDesktop = getLoadDesktop();
            Config.rootZipFile = getRootZipFile("root"); //MANUAL:"base.zip";
            Config.extraZipFiles = getZipFileList("overlay"); //MANUAL:"dlls.zip;fonts.zip";
            Config.appZipFile = getAppZipFile("app"); //MANUAL:"chomp.zip";
            Config.appPayload = getPayload("app-payload"); 
            Config.extraPayload = getPayload("overlay-payload"); 
            Config.Program = getExecutable(); //MANUAL:"CHOMP.EXE";
            Config.isSoundEnabled = getSound();
            Config.recordJITCache = getJitRecord();
            Config.disableHideCursor = getDisableHideCursor();
            Config.bpp = getBitsPerPixel();
			Config.cpu = getCPU();
			Config.envProp = getEnvProp();
			Config.emEnvProps = getEmscriptenEnvProps();
			Config.frameSkip = getFrameSkip();
			Config.resolution = getResolution();
			Config.ddrawOverridePath = getDDrawOverridePath();
			Config.payloadZipFile = "app.zip";
			Config.d_drive = "/d_drive";
        }
        function allowParameterOverride() {
            if(Config.urlParams.length >0) {
                return true;
            }
            return ALLOW_PARAM_OVERRIDE_FROM_URL;
        }
        function getEmscriptenEnvProps() {
            var props = getParameter("em-env").trim();
            let allProps = [];
	        //allProps.push({key: 'LIBGL_NPOT', value: 2});
	        //allProps.push({key: 'LIBGL_DEFAULT_WRAP', value: 0});
	        //allProps.push({key: 'LIBGL_MIPMAP', value: 3});	        
            if(allowParameterOverride()){
                if(props.length > 6) {
                	if( (props.startsWith("%22") && props.endsWith("%22") )
                		|| (props.startsWith('%27') && props.endsWith('%27'))){
                    	props = props.substring(3, props.length - 3);
	                	props = props.split('%20').join(' ');
            			props.trim().split(";").forEach(function(item){
            				let kv = item.split(":");
            				if (kv.length == 2) {
    	    					let key = kv[0].trim();
    	        				let value = kv[1].trim();
    	        				let existingIndex = allProps.findIndex(v => v.key === key);
    	        				if (existingIndex > -1) {
    	        				    allProps.splice(existingIndex, 1);
								}
	            				allProps.push({key: key, value: value});
            				}
            			});
                	}else{
	                	console.log("EMSCRIPTEN ENV props parameter must be in quoted string");
                	}
                }
            }
            if(allProps.length > 0) {
                console.log("setting EMSCRIPTEN ENV props:");
            	allProps.forEach(function(prop){
            		console.log(prop.key + " = " + prop.value);
            	});
            }
            return allProps;
        }
        function getDDrawOverridePath() {
            var property = getParameter("ddrawOverride").trim();
            if(allowParameterOverride() && property.length > 0){
                if( (property.startsWith("%22") && property.endsWith("%22") )
                	|| (property.startsWith('%27') && property.endsWith('%27'))){
                    return property.substring(3, property.length - 3);
                }else{
	                console.log("ddrawOverride path must be in quoted string");
                }
            }
            return null;
        }
        function getEnvProp() {
            var property = getParameter("env").trim();
            if(allowParameterOverride()){
                if(property.length > 6) {
                	if( (property.startsWith("%22") && property.endsWith("%22") )
                		|| (property.startsWith('%27') && property.endsWith('%27'))){
                    	let kv = property.substring(3, property.length - 3).split(':');
                    	return '"' + kv[0].trim() + "=" + kv[1].trim() + '"';
                	}else{
	                	console.log("ENV property must be in quoted string");
                	}
                }
            }
            return '';
        }
        function getCPU() {
            var cpu = getParameter("cpu");
            if(!allowParameterOverride()){
                cpu = "";
            }else if(cpu == "p2") {
            }else if(cpu == "p3") {
            }else{
                cpu = "";
            }
            if(cpu.length > 0) {
            	console.log("setting CPU to: "+cpu);
            }
            return cpu;
        }
        function getResolution() {
            var resolution = getParameter("resolution");
            if(!allowParameterOverride()){
                resolution = null;
            }else{
            	if (resolution != null) {
            		if (resolution.indexOf('x') > -1) {
            			let resNumbers = resolution.split('x');
            			if (!(resNumbers.length == 2 && isNumber(resNumbers[0]) && isNumber(resNumbers[1]))) {
            				resolution = null;
            			}            				
            		} else {
            			resolution = null;
            		}
            	}
            }
            if (resolution == null) {
            	console.log("not setting Resolution");
            } else {
            	console.log("setting Resolution to: "+resolution);
            }
            return resolution;
        }
        function isNumber(num) {
        	const result = Number(num);
        	return !isNaN(result) && result > 0 && result < 2000;
        }
        function getFrameSkip() {
            var frameskip =  getParameter("skipFrameFPS");
            if(!allowParameterOverride()){
                frameskip = DEFAULT_FRAME_SKIP;
            }else if(frameskip == ""){
                frameskip = DEFAULT_FRAME_SKIP;
            }else if(Number(frameskip) < 0 || Number(frameskip) > 50){
                frameskip = DEFAULT_FRAME_SKIP;
            }
            console.log("setting skipFrameFPS to: "+frameskip);
            return frameskip;
        }
        function getBitsPerPixel() {
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
        function getAutoRun() {
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
            console.log("setting auto run to: "+auto);
            return auto;
        }        
        function getLoadDesktop() {
            var loadDesktop =  getParameter("desktop");
            if(!allowParameterOverride()){
                loadDesktop = DEFAULT_LOAD_DESKTOP;
            }else if(loadDesktop == "true") {
                loadDesktop = true;
            }else if(loadDesktop == "false"){
                loadDesktop = false;
            }else{
                loadDesktop = DEFAULT_LOAD_DESKTOP;
            }
            console.log("setting load Desktop to: "+loadDesktop);
            return loadDesktop;
        }
        function getPayload(param) {
            var payload =  getParameter(param);
            if(!allowParameterOverride()){
                payload = "";
            }
            return payload;
        }
        function getSound() {
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

        // ?jit-record=true switches the WASM JIT into relocatable codegen so
        // compiled blocks can be exported with the Save JIT Cache button. Off
        // by default because relocatable blocks give up the embedded-pointer
        // dispatch fast paths (~12% throughput). Replay (a server cache zip
        // being present) activates the same mode implicitly.
        function getJitRecord() {
            var record = getParameter("jit-record");
            if(!allowParameterOverride()){
                record = false;
            }else if(record == "true") {
                record = true;
                console.log("setting jit-record to: " + record);
            }else{
                record = false;
            }
            return record;
        }

        function getDisableHideCursor() {
            var disableHideCursor = getParameter("disableHideCursor");
            if (!allowParameterOverride()) {
                disableHideCursor = DEFAULT_DISABLE_HIDE_CURSOR;
            } else if (disableHideCursor == "true") {
                disableHideCursor = true;
                console.log("setting disableHideCursor to: " + disableHideCursor);
            } else if (disableHideCursor == "false") {
                disableHideCursor = false;
            } else {
                disableHideCursor = DEFAULT_DISABLE_HIDE_CURSOR;
            }            
            return disableHideCursor;
        }

        function getExecutable() {
            var prog =  getParameter("p");
            if(!allowParameterOverride() || prog===""){
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
        function getAppZipFile(param) {

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
        function getRootZipFile(param) {

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
        function getZipFileList(param) {
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
                }
            }
            if(zipFiles.length > 0) {
            	console.log("setting " + param + " zip file(s) to: "+zipFiles);
            }
            return zipFiles;
        }
        // ---------------------------------------------------------------
        // WASM JIT persistent-cache key convention: 'v3-<eip>-<blockHash>'
        // where both values are 8-digit lowercase hex. Must stay in sync
        // with the inline key construction in the EM_JS bodies in
        // source/emulation/cpu/wasm/jitWasmCodeGen.cpp.
        // ---------------------------------------------------------------
        function boxedwineWasmJitHex32(value) {
            return ('00000000' + ((value >>> 0).toString(16))).slice(-8);
        }
        function boxedwineWasmJitCacheKey(eip, blockHash) {
            return 'v3-' + boxedwineWasmJitHex32(eip) + '-' + boxedwineWasmJitHex32(blockHash);
        }
        function parseWasmJitCacheKey(filename) {
            var slash = Math.max(filename.lastIndexOf('/'), filename.lastIndexOf('\\'));
            var base = slash >= 0 ? filename.slice(slash + 1) : filename;
            var stem = base.toLowerCase().endsWith('.wasm') ? base.slice(0, -5) : base;
            var parts = stem.split('-');
            if (parts.length !== 3 || parts[0] !== 'v3' || parts[1].length !== 8 || parts[2].length !== 8) return null;
            var eip = parseInt(parts[1], 16) >>> 0;
            var blockHash = parseInt(parts[2], 16) >>> 0;
            if (isNaN(eip) || isNaN(blockHash) || blockHash === 0) return null;
            return { key: boxedwineWasmJitCacheKey(eip, blockHash), eip: eip, blockHash: blockHash };
        }
        function getBase64Data(base64Data) {
            let bytes = atob(base64Data);
        	let contentLength = bytes.length;
    		var contents = new Uint8Array(contentLength);
			for (var i = 0; i < contentLength; i++) {
        		contents[i] = bytes.charCodeAt(i);
    		}
    		return contents;
        }
        function loadFile(pathPrefix, filename, callback) {
			fetch(pathPrefix + filename, { method: 'GET' }).then(function(response) {
      			if (response.status === 200) {
					response.arrayBuffer().then(function(buffer) {
						let arr = new Uint8Array(buffer);
						callback(arr);
    				});
      			} else {
      				console.log('Unable to load:' + filename + ' error:' + response.status);
      			}
			});
		}
        function buildAppFileSystem(callback) {
            if(Config.appPayload.length > 0){
            	let uint8Array = getBase64Data(Config.appPayload);
            	createFile("/", Config.payloadZipFile, uint8Array);
                callback();
            }else if(Config.appZipFile.length > 0){
            	loadFile(Config.locateAppBaseUrl, Config.appZipFile, (uint8Array) => {
            		if (Config.Program.length > 0) {            	
            			let zipEntries = getZipEntries(uint8Array);
            		    let folder = Config.appZipFile.toLowerCase().endsWith('.zip') ?
            		    	 Config.appZipFile.substring(0, Config.appZipFile.length - 4) : Config.appZipFile;
            			let executablePathAndFilename = folder + "/" + Config.Program;
            			let exeFileList = zipEntries.filter(e => !e.directory && e.filename === executablePathAndFilename);
            			if (exeFileList.length == 1) {
            				Config.appSubfolder = folder;
            			}
            		}
            		createFile("/", Config.appZipFile, uint8Array);
            		callback();
            	});
            }else{
                callback();
            }
        }
        function buildExtraFileSystems(callback) {
	        let extraFSs = [];
            if(Config.extraPayload.length > 0){
            	let uint8Array = getBase64Data(Config.extraPayload);
            	createFile("/", "overlay.zip", uint8Array);
            	callback();
            }else if(Config.extraZipFiles.length > 0){
                for(let i = 0; i < Config.extraZipFiles.length; i++) {
                    loadFile(Config.locateOverlayBaseUrl, Config.extraZipFiles[i], (uint8Array) => {
                    	createFile("/", Config.extraZipFiles[i], uint8Array);
                    	extraFSs.push(Config.extraZipFiles[i]);
                    	if(extraFSs.length == Config.extraZipFiles.length) {
            				callback();
                        }
            		});
                }
            }else{
                callback();
            }
        }
        function initBrowserFilesystem(callback) {
    		console.log("Use Storage mode: "+Config.storageMode);
			FS.mkdir(ROOT);
			FS.mkdir(Config.d_drive);
			if (Config.storageMode == STORAGE_INDEXED_DB) {
	  			FS.mount(IDBFS, {autoPersist: true}, ROOT);
	  			if (Config.persist_d_drive) {
	  				FS.mount(IDBFS, {autoPersist: true}, Config.d_drive);
	  			}
  				FS.syncfs(true, function (err) {
  					if (err) {
  						console.log('unable to sync folder: ' + ROOT);
  					} else {
  						callback();
  					}
				});
			} else {
				callback();
			}
		}
        function buildBrowserFileSystem() {
            if(Config.showUploadDownload){
                document.getElementById('uploadbtn').style.display = "";
                document.getElementById('downloadbtn').style.display = "";
            }
            // The Save JIT Cache button is handled by updateSaveJitButton(),
            // called from initWasmJitCache once the persistence mode is known.
            spinnerElement.style.display = 'none';
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
        function closeGetFilesModal() { //called by boxedwine.html
		}
        function start() { //called by boxedwine.html
        	if(isRunning){
                return;
            }
            startEmulator();
        }
        function startEmulator() {
            isRunning = true;

            document.getElementById('startbtn').style.display = 'none';
            document.getElementById('sound-checkbox').style.display = 'none';

            var params = getEmulatorParams();
            for(var i=0; i < params.length; i++) {
                Module['arguments'].push(params[i]);
            }

            document.getElementById('startbtn').textContent = "Running...";
            Module["removeRunDependency"]("setupBoxedWine");
        }
        var initialSetup = function(){
            console.log("running initial setup");
            setConfiguration();
            if (Config.emEnvProps.length > 0) {
            	Config.emEnvProps.forEach(function(prop){
            		ENV[prop.key] = prop.value;
            	});
            }

            // Create /tmp-jit-modules so the JIT cache import path is valid.
            try { FS.mkdir('/tmp-jit-modules'); } catch(e) {}

            // Preload the JIT module cache (clear stale IndexedDB blocks, then
            // fetch the per-app server zip) before the emulator starts. This
            // dependency is resolved inside initWasmJitCache's callback.
            Module["addRunDependency"]("wasm-jit-cache");
            initWasmJitCache(function() {
                Module["removeRunDependency"]("wasm-jit-cache");
            });

            Module["addRunDependency"]("setupBoxedWine");
            initBrowserFilesystem(() => {
            	spinnerElement.style.display = '';
            	spinnerElement.hidden = false;
            
	        	buildExtraFileSystems(() => {
    	        	buildAppFileSystem(() => {
    	            	loadFile(Config.locateRootBaseUrl, Config.rootZipFile, (rootZipfileBytes) => {
    	            	    createFile("/", Config.rootZipFile, rootZipfileBytes);
                        	buildBrowserFileSystem();
						});
                	});
	        	});
	        });
        }
        function getEntriesAsPromise(item, exeFiles, allFiles, firstCall) {
            return new Promise((resolve, reject) => {
                if(firstCall){
                    if(!Config.isAutoRunSet && !isRunning){
                        loadExeModal(exeFiles, allFiles);
                    }
                }
                if(item.isDirectory){
                    let reader = item.createReader();
                    let doBatch = () => {
                        reader.readEntries(entries => {
                            if (entries.length > 0) {
                                entries.forEach(function(entry){
                                    getEntriesAsPromise(entry, exeFiles, allFiles, false);
                                });
                                doBatch();
                            } else {
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
                    item.file(function(item){uploadFile(item, fullPath, allFiles);}, e => console.log(e));
                }
            });
        }
        function loadExeModal(exeFiles, allFiles) {
            document.getElementById('modalLinkExe').click();
            var message = document.getElementById('message');
            message.innerHTML = "<p>Uploading files...</p>";
            ExeFileTimer = setInterval(function(){readyCheck(exeFiles, allFiles);}, 100);
        }
        function populateModalExe(exeFiles) {
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
        function execute(filename) {
            var root = document.getElementById('items');
            document.getElementById('openModalExeClick').click();

            var file = filename.substring(filename.lastIndexOf("/")+1, filename.length);
            var path = filename.substring(0, filename.lastIndexOf("/"));

            Config.WorkingDir = "/home/username/.wine/dosdevices/d:/" + path.substring(1);
            Config.Program = file;

            startEmulator();
        }
        function readyCheck(exeFiles, allFiles) {
            if(allFiles.length==0){
                clearInterval(ExeFileTimer);
                var message = document.getElementById('message');
                if (exeFiles.length == 0) {
	                message.innerHTML = 'No executable files found';                
                } else {
	                message.innerHTML = '';
                }
                populateModalExe(exeFiles);
            }
        }
        dropzone.addEventListener("dragover", function(event){
            event.preventDefault();
        }, false);
        dropzone.addEventListener("drop", function(event){
            event.preventDefault();
            let items = event.dataTransfer.items;
            let exeFiles = [];
            let allFiles = [];
            for(let i =0; i < items.length; i++){
                getEntriesAsPromise(items[i].webkitGetAsEntry(), exeFiles, allFiles, true);
            }
        }, false);
        function getEmulatorParams() {        
            let params = ["-root", ROOT];
            params.push("-zip");
    		params.push(Config.rootZipFile);
    		
            if(Config.extraZipFiles.length > 0){
                for(let i = 0; i < Config.extraZipFiles.length; i++) {
		            params.push("-zip");
    				params.push(Config.extraZipFiles[i]);
                }
            }    		
            if(Config.extraPayload.length > 0){
		        params.push("-zip");
    			params.push("overlay.zip");
            }
            
            if (Config.appZipFile.length > 0) { // -mount $appZipFile "/home/username/files/"
    			params.push("-mount");
    			params.push(Config.appZipFile);
    			params.push(Config.appDirPrefix);          
			} else if (Config.appPayload.length > 0){ // -mount "app.zip" "/home/username/files/" 			
    			params.push("-mount");
    			params.push(Config.payloadZipFile);
    			params.push(Config.appDirPrefix);          			
            }
                        
            params.push("-mount_drive"); // -mount_drive "/d_drive" d
            params.push(Config.d_drive);
            params.push("d");
            
            if (Config.resolution != null) {
            	params.push("-resolution");
            	params.push(Config.resolution);
            }
            if (Config.ddrawOverridePath != null) {
            	params.push("-ddrawOverride");
            	params.push(Config.ddrawOverridePath);
            }
            if (Config.frameSkip != "0") {
            	params.push("-skipFrameFPS");
            	params.push(Config.frameSkip);
			}            
            
            if(!Config.isSoundEnabled){
                params.push("-nosound");
            }
            if (Config.disableHideCursor) {
                params.push("-disableHideCursor");
            }
            if(Config.bpp != DEFAULT_BPP){
                params.push("-bpp");
                params.push("" + Config.bpp);
            }
            if(Config.cpu.length > 0){
                params.push("-" + Config.cpu);
            }
            if(Config.envProp.length > 0){
                params.push("-env");
                params.push(Config.envProp);
            }

			if (!Config.loadDesktop) {
            	if(Config.WorkingDir.length > 0){
                	params.push("-w");
                	params.push(Config.WorkingDir);
            	}else if(Config.appPayload.length > 0 && Config.Program.length > 0 && Config.Program.substring(0 ,1) != "/"){
                	params.push("-w");
                	params.push(Config.appDirPrefix);
            	}else if(Config.appZipFile.length > 0 && Config.Program.length > 0 && Config.Program.substring(0 ,1) != "/"){
                	params.push("-w");
                	if (Config.appSubfolder.length > 0) {
                		params.push(Config.appDirPrefix + "/" + Config.appSubfolder);                
                	} else {
                		params.push(Config.appDirPrefix);
                	}
            	}
            }
        	params.push("/bin/wine");
            if(Config.Program.length > 0 && !Config.loadDesktop){
                if (Config.Program.endsWith('.bat')) {
                    params.push("cmd");
                    params.push("/c");
                }
                params.push(Config.Program);
            }else{
	            params.push("explorer");
    	        params.push("/desktop=shell");
            }
            console.log("Emulator params:" + params);
            return params;
        }
      var Module = {
        preRun: [initialSetup],
        arguments: [],
        postRun: [],
        // Called by Emscripten after runtimeInitialized=true, just before main().
        // By this point all run-dependencies (including wasm-jit-cache) have been
        // resolved, so the persistence-mode decision is final: latch it into C++
        // here, before the first block is compiled.  For MT builds we must also
        // prime the C++ g_wasmCacheByKey map here, on the main thread where
        // Module.wasmJitCache is accessible, before any worker starts JIT work.
        // EM_JS functions cannot appear in EXPORTED_FUNCTIONS (they are JavaScript
        // imports into WASM, not WASM exports), so this logic lives directly here
        // rather than behind a Module._wasm_jit_mt_preload_from_js_cache call.
        onRuntimeInitialized: function() {
            if (Module.wasmJitPersistenceWanted &&
                    typeof Module._wasm_jit_set_persistence_active === 'function') {
                Module._wasm_jit_set_persistence_active();
            }
            if (typeof Module._wasm_jit_mt_register !== 'function') return;
            if (!Module.wasmJitCache || Module.wasmJitCache.size === 0) {
                console.log('[WASM JIT MT] no cached blocks to preload');
                return;
            }
            var count = 0;
            Module.wasmJitCache.forEach(function(data, key) {
                var cacheKey = parseWasmJitCacheKey(String(key));
                if (!cacheKey) return;
                var size = data.length;
                var ptr = Module._malloc(size);
                if (!ptr) return;
                Module.HEAPU8.set(data, ptr);
                Module._wasm_jit_mt_register(cacheKey.eip, cacheKey.blockHash, ptr, size);
                Module._free(ptr);
                count++;
            });
            console.log('[WASM JIT MT] preloaded ' + count + ' blocks into C++ cache');
        },
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
      // Suppress browser alert() dialogs — they block the page and prevent
      // the developer console from being opened to inspect log output.
      // SDL_ShowSimpleMessageBox (and any kpanic path) goes through alert();
      // redirect it to console.error so messages are still visible.
      window.alert = function(msg) { console.error('[alert suppressed]', msg); };
      window.onerror = function(msg, file, line, column, error) {
        Module.setStatus('Exception thrown, see JavaScript console');
        console.log(msg, file, line, column, error);
        spinnerElement.style.display = 'none';
        Module.setStatus = function(text) {
          if (text) Module.printErr('[post-exception status] ' + text);
        };
      };
        function startWithFiles(files) {
            for (let i = 0; i < files.length; i++) {
                uploadFile(files[i]);
            }
        }
        function uploadFile(file, fullPath, allFiles) {
            let filename = null;
            if(fullPath){
                filename = fullPath.startsWith("/") ? fullPath.substring(1) : fullPath;
            }else{
                filename = file.webkitRelativePath.length == 0 ? file.name : file.webkitRelativePath;
            }
            var filereader = new FileReader();
            filereader.file_name = file.name;
            filereader.onload = function(){readFile(this.result, filename, allFiles)};
            filereader.readAsArrayBuffer(file);
        }
        function readFile(data, name, allFiles) {
        	let filename = name.substring(name.lastIndexOf("/")+1,name.length);
            if(name.toLowerCase().endsWith('zip')){
				createFile(Config.d_drive, filename, new Uint8Array(data));
            }else{
                var done = false;
                var startIndex = 0;
                var base = Config.d_drive + "/";
                while(!done){
                    var dirIndex = name.indexOf("/", startIndex);
                    if(dirIndex == -1){
                        done =true;
                    }else{
                        var dirName = name.substring(startIndex, dirIndex);
                        if(dirName.length > 0) {
                            createFolder(base, dirName);
                            base = base + dirName + "/";
                        }
                        startIndex = dirIndex + 1;
                    }
                }
                createFile(base.substring(0,base.length-1), filename, new Uint8Array(data));
            }
            if(allFiles){
                allFiles.pop();
            }
        }
function createFolder(parent, dir) {
    try {
        FS.createPath(parent, dir, true, true);
    	console.log("Directory created :" + parent +  dir);
    } catch(ef) {
    	console.log("Unable to create folder:" + parent + dir + " error:" + ef);
    }
}
function createFile(dir, name, buf) {
    try {
        FS.createDataFile(dir, name, buf, true, true);
        console.log("File created:" + dir + "/" + name);
    } catch(e) {
        console.log("Unable to create file:" + dir + "/" + name + "  error:" + e);
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

// ---------------------------------------------------------------------------
// WASM JIT module persistence
// ---------------------------------------------------------------------------

// Open IndexedDB, clear any blocks left over from a previous session or a
// different app, then load the app-specific server zip into the fresh store.
// Clearing on startup prevents stale EIP→binary mappings from a different app
// or an older build from being instantiated by the JIT and causing crashes.
// The server zip (named after Config.appZipFile) is the authoritative per-app
// cache; it is re-imported each run and supplemented by blocks compiled during
// the session, so the store never accumulates cross-app debris.
// Calls callback() when done (with or without error) so the run-dependency
// can be removed and the emulator can start.
function initWasmJitCache(callback) {
    Module.wasmJitCache = new Map();
    Module.wasmJitCompiledCache = new Map();
    Module.wasmJitGroupModules = new Map();
    Module.wasmJitGroupInstances = new Map();
    Module.wasmJitGroupEntryMap = new Map();
    Module.wasmJitInstalledCache = new Map();
    Module.wasmJitInstalledByTableIndex = new Map();
    Module.wasmJitGroupedManifest = null;
    Module.wasmJitPersistenceWanted = false;
    // Persistence is a runtime mode: the JIT compiles relocatable (exportable)
    // blocks only when this session is a replay (the server zip provided flat
    // blocks or merged groups) or a recording (?jit-record=true). Decided
    // here, once, before the emulator starts; Module.onRuntimeInitialized
    // latches it into C++ via the exported _wasm_jit_set_persistence_active
    // before main() runs.
    var done = function() {
        var replay = Module.wasmJitCache.size > 0 ||
                     Module.wasmJitGroupEntryMap.size > 0;
        Module.wasmJitPersistenceWanted = replay || Config.recordJITCache === true;
        if (Module.wasmJitPersistenceWanted) {
            console.log('[WASM JIT] persistence mode on (' +
                (replay ? 'replay' : 'record') + ')');
        }
        updateSaveJitButton();
        callback();
    };
    var req = indexedDB.open('boxedwine-wasm-jit', 1);
    req.onupgradeneeded = function(e) {
        e.target.result.createObjectStore('blocks');
    };
    req.onerror = function() {
        console.warn('[WASM JIT] IndexedDB unavailable, JIT cache disabled');
        done();
    };
    req.onsuccess = function(e) {
        Module.wasmJitDb = e.target.result;
        // Clear stale entries before loading the server zip.
        var tx = Module.wasmJitDb.transaction('blocks', 'readwrite');
        tx.objectStore('blocks').clear();
        tx.oncomplete = function() {
            fetchServerJitCache(done);
        };
        tx.onerror = function() {
            fetchServerJitCache(done);
        };
    };
}

// Show the Save JIT Cache button only in record mode (?jit-record=true) on a
// build that supports persistence (it exports _wasm_jit_set_persistence_active)
// and with Config.showSaveJITCache enabled. Replay sessions compile exportable
// blocks too, but re-saving what the server zip already provides is not a
// workflow — recording is the explicit way to produce a cache zip.
function updateSaveJitButton() {
    if (Config.showSaveJITCache && Config.recordJITCache === true &&
            typeof Module._wasm_jit_set_persistence_active !== 'undefined') {
        document.getElementById('savejitbtn').style.display = "";
    }
}

// Returns the JIT cache zip filename for the current app.
// If Config.appZipFile is set (e.g. "ski32.zip"), the name is derived from it
// (e.g. "ski32-jit-modules.zip") so each app has its own cache file.
// Falls back to "wasm-jit-modules.zip" when no app zip is configured.
function getJitCacheZipFilename() {
    if (Config.appZipFile && Config.appZipFile.length > 0) {
        var base = Config.appZipFile.toLowerCase().endsWith('.zip')
            ? Config.appZipFile.slice(0, -4)
            : Config.appZipFile;
        return base + '-jit-modules.zip';
    }
    return 'wasm-jit-modules.zip';
}

// ---------------------------------------------------------------------------
// Compression helpers — raw DEFLATE via the Compression Streams API.
// deflate-raw is the algorithm ZIP uses for compression method 8; the header-
// less format means ZIP's own length/CRC fields are the sole integrity check.
// Both helpers use new Response(stream).arrayBuffer() to drain the readable
// side of the transform stream into a single ArrayBuffer in one await.
// ---------------------------------------------------------------------------

async function deflateRaw(data) {
    var cs = new CompressionStream('deflate-raw');
    var writer = cs.writable.getWriter();
    writer.write(data);
    writer.close();
    return new Uint8Array(await new Response(cs.readable).arrayBuffer());
}

async function inflateRaw(data) {
    var ds = new DecompressionStream('deflate-raw');
    var writer = ds.writable.getWriter();
    writer.write(data);
    writer.close();
    return new Uint8Array(await new Response(ds.readable).arrayBuffer());
}

// Attempt to fetch the JIT cache zip from the server.  If found, its entries
// are imported into IndexedDB and the in-memory cache.
function fetchServerJitCache(callback) {
    var baseUrl  = (Config.appZipFile && Config.appZipFile.length > 0)
                   ? Config.locateAppBaseUrl : Config.locateRootBaseUrl;
    fetch(baseUrl + getJitCacheZipFilename())
        .then(function(resp) {
            if (!resp.ok) { callback(); return null; }
            return resp.arrayBuffer();
        })
        .then(async function(buf) {
            if (!buf) return;
            await importJitModulesFromBuffer(new Uint8Array(buf));
            callback();
        })
        .catch(function() {
            callback();
        });
}

// Parse a JIT cache zip buffer and load its contents:
//   v3-*.wasm                            flat block modules -> wasmJitCache
//   groups/*.wasm                        merged group modules (pipeline output)
//   boxedwine-jit-grouped-manifest.json  group entry map + direct-call stats +
//                                        profile-guided split hints
// Handles both compression method 0 (STORED) and method 8 (DEFLATE, produced
// by saveJitModules and the offline pipeline). Decompression of all entries
// runs in parallel via Promise.all, then the results are committed to the
// in-memory caches and IndexedDB in a single transaction.
async function importJitModulesFromBuffer(bytes) {
    try {
        var view  = new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength);
        var dec   = new TextDecoder();
        var pos   = 0;
        var entries = [];
        var manifests = [];
        var groupModules = [];

        // Phase 1: scan local file headers synchronously — no decompression yet.
        while (pos + 30 <= bytes.length) {
            if (view.getUint32(pos, true) !== 0x04034B50) break; // local file header sig

            var method    = view.getUint16(pos + 8,  true);
            var cSize     = view.getUint32(pos + 18, true);
            var uSize     = view.getUint32(pos + 22, true);
            var fnLen     = view.getUint16(pos + 26, true);
            var exLen     = view.getUint16(pos + 28, true);
            var dataStart = pos + 30 + fnLen + exLen;

            if ((method === 0 || method === 8) && uSize > 0 && fnLen >= 8 &&
                    dataStart + cSize <= bytes.length) {
                var fn  = dec.decode(bytes.subarray(pos + 30, pos + 30 + fnLen));
                var cacheKey = parseWasmJitCacheKey(fn);
                if (cacheKey) {
                    entries.push({
                        key:       cacheKey.key,
                        eip:       cacheKey.eip,
                        blockHash: cacheKey.blockHash,
                        method:    method,
                        data:      bytes.slice(dataStart, dataStart + cSize)
                    });
                } else if (fn === 'boxedwine-jit-grouped-manifest.json') {
                    manifests.push({
                        method: method,
                        data: bytes.slice(dataStart, dataStart + cSize)
                    });
                } else if (fn.indexOf('groups/') === 0 && fn.toLowerCase().endsWith('.wasm')) {
                    groupModules.push({
                        path: fn,
                        method: method,
                        data: bytes.slice(dataStart, dataStart + cSize)
                    });
                }
            }
            pos = dataStart + cSize;
        }

        // Phase 2: decompress all entries in parallel.
        var results = await Promise.all(entries.map(async function(e) {
            try {
                var data = e.method === 8 ? await inflateRaw(e.data) : e.data;
                return { key: e.key, eip: e.eip, blockHash: e.blockHash, data: data };
            } catch(ex) {
                console.warn('[WASM JIT] decompression failed for key=' +
                             e.key + ':', ex);
                return null;
            }
        }));
        var valid = results.filter(function(r) { return r !== null; });
        var manifestResults = await Promise.all(manifests.map(async function(e) {
            try {
                var data = e.method === 8 ? await inflateRaw(e.data) : e.data;
                return JSON.parse(new TextDecoder().decode(data));
            } catch(ex) {
                console.warn('[WASM JIT] grouped manifest import failed:', ex);
                return null;
            }
        }));

        // Phase 3 (ST only): precompile all modules while startup is still
        // blocked on the wasm-jit-cache run dependency, so cache hits skip the
        // synchronous WebAssembly.Module compile. The byte cache remains the
        // source of truth for export/IDB; compiled modules are memory-only.
        // MT builds consume raw bytes from the C++ side (workers cannot reach
        // these main-thread Maps), so precompiling here would be wasted work —
        // and merged group modules are unusable in MT for the same reason.
        var isMT = typeof Module._wasm_jit_mt_register === 'function';
        var compiled = isMT ? [] : await Promise.all(valid.map(async function(r) {
            try {
                return { key: r.key, module: await WebAssembly.compile(r.data) };
            } catch(ex) {
                console.warn('[WASM JIT] precompile failed for key=' + r.key + ':', ex);
                return null;
            }
        }));
        var groupResults = isMT ? [] : await Promise.all(groupModules.map(async function(e) {
            try {
                var data = e.method === 8 ? await inflateRaw(e.data) : e.data;
                return { path: e.path, module: await WebAssembly.compile(data) };
            } catch(ex) {
                console.warn('[WASM JIT] grouped module precompile failed for path=' + e.path + ':', ex);
                return null;
            }
        }));
        if (isMT && groupModules.length > 0) {
            console.warn('[WASM JIT] grouped cache modules are single-threaded only; ' +
                         'for multi-threaded builds record with the MT build and run the ' +
                         'cache pipeline with --flat (caches are per-build either way)');
        }

        var groupedManifest = manifestResults.find(function(m) { return m && m.format === 'boxedwine-wasm-jit-grouped-cache'; }) || null;
        Module.wasmJitGroupedManifest = groupedManifest;
        Module.wasmJitProfileSplitTargets = new Map();
        Module.wasmJitProfileSplitTargetSources = new Map();
        Module.wasmJitProfileSplitStats = null;
        groupResults.forEach(function(r) {
            if (r) Module.wasmJitGroupModules.set(r.path, r.module);
        });
        if (groupedManifest && groupedManifest.groups && !isMT) {
            groupedManifest.groups.forEach(function(group) {
                var path = group.path;
                (group.entries || []).forEach(function(entry) {
                    Module.wasmJitGroupEntryMap.set(entry.key, {
                        groupPath: entry.path || path,
                        exportName: entry.exportName || entry.key
                    });
                });
            });
        }
        if (groupedManifest && groupedManifest.profileGuidedSplits &&
                Array.isArray(groupedManifest.profileGuidedSplits.targets)) {
            groupedManifest.profileGuidedSplits.targets.forEach(function(split) {
                if (!split || split.blockStart == null || split.target == null) return;
                var blockStartKey = boxedwineWasmJitHex32(split.blockStart >>> 0);
                var targetKey = boxedwineWasmJitHex32(split.target >>> 0);
                Module.wasmJitProfileSplitTargets.set(blockStartKey, targetKey);
                var sources = Module.wasmJitProfileSplitTargetSources.get(targetKey);
                if (!sources) {
                    sources = [];
                    Module.wasmJitProfileSplitTargetSources.set(targetKey, sources);
                }
                if (sources.indexOf(blockStartKey) < 0) sources.push(blockStartKey);
            });
        }

        // Remember which eip/hash identities the cache covers so lookup misses
        // can be classified (unknown EIP vs. same EIP with a different hash).
        if (!Module.wasmJitCacheEips) Module.wasmJitCacheEips = new Set();
        if (!Module.wasmJitCacheEipHashes) Module.wasmJitCacheEipHashes = new Map();
        function rememberCacheIdentity(eip, blockHash) {
            var eipKey = boxedwineWasmJitHex32(eip);
            var hashKey = boxedwineWasmJitHex32(blockHash);
            var hashes = Module.wasmJitCacheEipHashes.get(eipKey);
            if (!hashes) {
                hashes = [];
                Module.wasmJitCacheEipHashes.set(eipKey, hashes);
            }
            if (hashes.length < 4 && hashes.indexOf(hashKey) < 0) hashes.push(hashKey);
            Module.wasmJitCacheEips.add(eipKey);
        }
        if (groupedManifest && groupedManifest.groups) {
            groupedManifest.groups.forEach(function(group) {
                (group.entries || []).forEach(function(entry) {
                    var cacheKey = parseWasmJitCacheKey(entry.key);
                    if (cacheKey) rememberCacheIdentity(cacheKey.eip, cacheKey.blockHash);
                });
            });
        }
        valid.forEach(function(r) {
            rememberCacheIdentity(r.eip, r.blockHash);
            Module.wasmJitCache.set(r.key, r.data);
        });
        compiled.forEach(function(r) {
            if (r) Module.wasmJitCompiledCache.set(r.key, r.module);
        });
        if (Module.wasmJitDb && valid.length > 0) {
            try {
                var tx    = Module.wasmJitDb.transaction('blocks', 'readwrite');
                var store = tx.objectStore('blocks');
                valid.forEach(function(r) { store.put(r.data, r.key); });
            } catch(e) {}
        }
        if (groupedManifest) {
            var stats = groupedManifest.stats || {};
            var graph = stats.graphEdges || {};
            var imports = stats.imports || {};
            console.log('[WASM JIT] imported ' + valid.length +
                        ' blocks from grouped server cache groups=' + (stats.groups || (groupedManifest.groups ? groupedManifest.groups.length : 0)) +
                        ' groupModules=' + Module.wasmJitGroupModules.size +
                        ' precompiled=' + Module.wasmJitCompiledCache.size +
                        ' profileSplits=' + Module.wasmJitProfileSplitTargets.size +
                        ' resolvedEdges=' + (graph.resolved || 0) +
                        ' intraGroupEdges=' + (graph.inGroup || 0) +
                        ' functionImports=' + (imports.functionImportsBefore || 0) + '->' + (imports.functionImportsAfterGroupedEstimate || 0));
        } else {
            console.log('[WASM JIT] imported ' + valid.length +
                        ' blocks from server cache precompiled=' + Module.wasmJitCompiledCache.size);
        }
    } catch(e) {
        console.warn('[WASM JIT] server cache import failed:', e);
    }
}

// Export all cached JIT modules as a DEFLATE-compressed ZIP and trigger a
// browser download.  Each entry uses compression method 8 (deflate-raw) via
// the Compression Streams API, typically halving file size versus STORED.
// All entries are compressed in parallel with Promise.all before the ZIP
// structure is assembled so the compressed sizes are known before headers
// are written (avoiding the need for a data-descriptor extension).
// The ZIP is built entirely in JavaScript; minizip is not involved.
async function saveJitModules() {
    // For MT builds, sync g_wasmCacheByKey → Module.wasmJitCache first so worker-
    // compiled blocks (which only the C++ map saw) are included in the ZIP.
    if (typeof Module._wasm_jit_mt_prepare_export === 'function') {
        Module._wasm_jit_mt_prepare_export();
    }
    if (!Module.wasmJitCache || Module.wasmJitCache.size === 0) {
        console.log('[WASM JIT] no cached blocks to export');
        return;
    }
    try {
        // CRC-32/ISO-HDLC — computed on uncompressed data, as ZIP requires.
        var crcTable = new Uint32Array(256);
        for (var n = 0; n < 256; n++) {
            var c = n >>> 0;
            for (var k = 0; k < 8; k++) c = c & 1 ? 0xEDB88320 ^ (c >>> 1) : c >>> 1;
            crcTable[n] = c;
        }
        function crc32(data) {
            var c = 0xFFFFFFFF;
            for (var i = 0; i < data.length; i++) c = (c >>> 8) ^ crcTable[(c ^ data[i]) & 0xFF];
            return (c ^ 0xFFFFFFFF) >>> 0;
        }

        // Compress all entries in parallel so we know each compressed size
        // before writing any local file header.
        var cacheEntries = Array.from(Module.wasmJitCache.entries());
        var exportedCacheKeys = new Set();
        var compressed = await Promise.all(cacheEntries.map(async function(kv) {
            var key = String(kv[0]), data = kv[1];
            if (!parseWasmJitCacheKey(key)) return null;
            exportedCacheKeys.add(key);
            var compData = await deflateRaw(data);
            return { name: key + '.wasm', data: data, compData: compData, crc: crc32(data) };
        }));
        compressed = compressed.filter(function(e) { return e !== null; });

        // Per-block exit metadata + runtime constants, consumed by the offline
        // cache pipeline (boxedwine-wasm-jit-cache-pipeline.mjs) to build the
        // successor graph for grouping/direct-call rewriting. Only blocks that
        // made it into the zip are listed.
        if (Module.wasmJitBlockMeta && Module.wasmJitBlockMeta.size > 0) {
            var manifestEntries = Array.from(Module.wasmJitBlockMeta.values())
                .filter(function(entry) { return entry && exportedCacheKeys.has(entry.key); })
                .sort(function(a, b) { return a.key < b.key ? -1 : (a.key > b.key ? 1 : 0); });
            var manifest = {
                version: 1,
                generatedAt: new Date().toISOString(),
                entryCount: manifestEntries.length,
                runtime: Module.wasmJitRuntimeConstants || null,
                entries: manifestEntries
            };
            var manifestData = new TextEncoder().encode(JSON.stringify(manifest, null, 2));
            compressed.push({
                name: 'boxedwine-jit-manifest.json',
                data: manifestData,
                compData: await deflateRaw(manifestData),
                crc: crc32(manifestData)
            });
        }

        // Interior-transition profile sidecar; the pipeline parses it (or a
        // pasted console log) into profile-guided split hints.
        if (Module.wasmJitInteriorProfileLines && Module.wasmJitInteriorProfileLines.length > 0) {
            var profileData = new TextEncoder().encode(Module.wasmJitInteriorProfileLines.join('\n') + '\n');
            compressed.push({
                name: 'boxedwine-jit-profile.txt',
                data: profileData,
                compData: await deflateRaw(profileData),
                crc: crc32(profileData)
            });
        }

        var parts   = [];
        var cdParts = [];
        var offset  = 0;

        compressed.forEach(function(e) {
            var name      = e.name;
            var nameBytes = new TextEncoder().encode(name);
            var nl        = nameBytes.length;
            var cSize     = e.compData.length;
            var uSize     = e.data.length;

            // Local file header (30 bytes fixed + filename).
            var lh = new DataView(new ArrayBuffer(30 + nl));
            lh.setUint32(0,  0x04034B50, true); // local file header signature
            lh.setUint16(4,  20,         true); // version needed to extract (2.0)
            lh.setUint16(8,  8,          true); // compression method: DEFLATE
            lh.setUint32(14, e.crc,      true); // crc-32 of uncompressed data
            lh.setUint32(18, cSize,      true); // compressed size
            lh.setUint32(22, uSize,      true); // uncompressed size
            lh.setUint16(26, nl,         true); // filename length
            new Uint8Array(lh.buffer, 30).set(nameBytes);

            // Central directory file header (46 bytes fixed + filename).
            var cd = new DataView(new ArrayBuffer(46 + nl));
            cd.setUint32(0,  0x02014B50, true); // central directory signature
            cd.setUint16(4,  20,         true); // version made by
            cd.setUint16(6,  20,         true); // version needed
            cd.setUint16(10, 8,          true); // compression method: DEFLATE
            cd.setUint32(16, e.crc,      true); // crc-32
            cd.setUint32(20, cSize,      true); // compressed size
            cd.setUint32(24, uSize,      true); // uncompressed size
            cd.setUint16(28, nl,         true); // filename length
            cd.setUint32(42, offset,     true); // offset of local file header
            new Uint8Array(cd.buffer, 46).set(nameBytes);

            parts.push(new Uint8Array(lh.buffer));
            parts.push(e.compData);
            cdParts.push(new Uint8Array(cd.buffer));
            offset += 30 + nl + cSize;
        });

        // Central directory followed by end-of-central-directory record.
        var cdOffset = offset;
        var cdSize   = cdParts.reduce(function(s, cd) { return s + cd.length; }, 0);
        cdParts.forEach(function(cd) { parts.push(cd); });

        var count = compressed.length;
        var eocd  = new DataView(new ArrayBuffer(22));
        eocd.setUint32(0,  0x06054B50, true); // end of central directory signature
        eocd.setUint16(8,  count,      true); // total entries on this disk
        eocd.setUint16(10, count,      true); // total entries
        eocd.setUint32(12, cdSize,     true); // size of central directory
        eocd.setUint32(16, cdOffset,   true); // offset of start of central directory
        parts.push(new Uint8Array(eocd.buffer));

        var blob = new Blob(parts, { type: 'application/zip' });
        var url  = URL.createObjectURL(blob);
        var a    = document.createElement('a');
        a.href   = url;
        var zipFilename = getJitCacheZipFilename();
        a.download = zipFilename;
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
        URL.revokeObjectURL(url);
        console.log('[WASM JIT] exported ' + count + ' blocks to ' + zipFilename);
    } catch(e) {
        console.error('[WASM JIT] export failed:', e);
    }
}
function toggleDirectory(item) {
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
function getParameter(inputKey) {
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
var index = 0;
var files = [];
var selectedItem;
var selectedFilename;
function select(index, dir, filename) {
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
function endsWith(str, suffix) {
    return str.indexOf(suffix, str.length - suffix.length) !== -1;
}
function extract() {
	let file = document.getElementById('selectedItem').value;
	if(file != null && file.length > 0 && files.length > 1) {
        if(endsWith(file,"/")){
			return;
        }
        let data = FS.readFile(file, { encoding: 'binary' });
		let blob =  new Blob([data], {type: "octet/stream"});
		
      	let link = document.createElement('a');
      	link.setAttribute('download', selectedFilename);
      	link.setAttribute('href', window.URL.createObjectURL(blob));
      	document.body.appendChild(link);
      	link.click();
      	document.body.removeChild(link);
	}
}
function leaf(entry) {
    index++;
	var text = "<tr><td ><span id=\"" + index + "-data\" onclick=\"select(" + index + ",'" + entry.dir + "','" + entry.filename + "')\">" + entry.filename + "</span></td></tr>";
	return text;
}
function branch(entries) {
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
function parentDir(childDir) {
    if(endsWith(childDir,"/")){
        childDir = childDir.substring(0,childDir.length - 1);
    }
    return childDir.substring(0, childDir.lastIndexOf('/') + 1);
}
function buildGetFilesModal() {
	buildGetFilesModalForFolder(Config.d_drive);
}
function buildGetFilesModalForFolder(folder) {
    document.getElementById('modalLink').click();
    let root = document.getElementById('tree');
    //reset
    document.getElementById('selectedItem').value = "";
    selectedFilename = null;
	files = [];
    root.innerHTML = "";
    index = 0;
	readFiles(folder + "/", files);

	let contents = "<table>" + branch(files) + "</table>";
	document.getElementById('loadStatus').style.display="none";
	root.innerHTML = contents;
	toggleDirectory('1');
}
function readFiles(currentDir, files) {
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


/** code from https://github.com/Rob--W/zipinfo.js MIT license
 **/
function getZipEntries(data) {
  var view = new DataView(data.buffer, data.byteOffset, data.length);
  var entriesLeft = 0;
  var offset = 0;
  var endoffset = data.length;
  // Find EOCD (0xFFFF is the maximum size of an optional trailing comment).
  for (var i = data.length - 22, ii = Math.max(0, i - 0xFFFF); i >= ii; --i) {
    if (data[i] === 0x50 && data[i + 1] === 0x4b &&
      data[i + 2] === 0x05 && data[i + 3] === 0x06) {
        endoffset = i;
        offset = view.getUint32(i + 16, true);
        entriesLeft = view.getUint16(i + 8, true);
        break;
      }
  }
  var entries = [{
    directory: true,
    filename: '/',
    uncompressedSize: 0,
    centralDirectoryStart: offset,
  }];
  if (offset >= data.length || offset <= 0) {
    // EOCD not found or malformed. Try to recover if possible (the result is
    // most likely going to be incomplete or bogus, but we can try...).
    offset = -1;
    entriesLeft = 0xFFFF;
    while (++offset < data.length && data[offset] !== 0x50 &&
      data[offset + 1] !== 0x4b && data[offset + 2] !== 0x01 &&
        data[offset + 3] !== 0x02);
  }
  endoffset -= 46;  // 46 = minimum size of an entry in the central directory.
  while (--entriesLeft >= 0 && offset < endoffset) {
    if (view.getUint32(offset) != 0x504b0102) {
      break;
    }
    var bitFlag = view.getUint16(offset + 8, true);
    var uncompressedSize = view.getUint32(offset + 24, true);
    var fileNameLength = view.getUint16(offset + 28, true);
    var extraFieldLength = view.getUint16(offset + 30, true);
    var fileCommentLength = view.getUint16(offset + 32, true);
    var filename = data.subarray(offset + 46, offset + 46 + fileNameLength);
    var utfLabel = (bitFlag & 0x800) ? 'utf-8' : 'ascii';
    filename = new TextDecoder(utfLabel).decode(filename);
    entries.push({
      directory: filename.endsWith('/'),
      filename: filename,
      uncompressedSize: uncompressedSize,
    });
    offset += 46 + fileNameLength + extraFieldLength + fileCommentLength;
  }
  return entries;
};
