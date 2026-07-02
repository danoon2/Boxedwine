/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

        let ALLOW_PARAM_OVERRIDE_FROM_URL = true;
        let ROOT = "/root";
        let STORAGE_INDEXED_DB = "INDEXED_DB";
        let STORAGE_MEMORY = "MEMORY";
        let STORAGE_HOST_FOLDER = "HOST_FOLDER";
        let HOST_FOLDER_HANDLE_DB = "boxedwine-host-folder-handles";
        let HOST_FOLDER_HANDLE_DB_VERSION = 1;
        let HOST_FOLDER_HANDLE_STORE = "handles";
        let HOST_FOLDER_HANDLE_KEY = "d";
        let HOST_FOLDER_REFRESH_INTERVAL_MS = 10000;

        let DEFAULT_AUTO_RUN = true;
        let DEFAULT_LOAD_DESKTOP = false;
        let DEFAULT_SOUND_ENABLED = true;
        let DEFAULT_DISABLE_HIDE_CURSOR = false;
        let DEFAULT_APP_DIRECTORY = "/home/username/.wine/dosdevices/c:/files";
        let DEFAULT_BPP = 32;
        let DEFAULT_FRAME_SKIP = "0";
        let DEFAULT_AUDIO_FREQ = 11025;
        let DEFAULT_ROOT_ZIP_FILE = "boxedwine.zip";
        // registry=[name,...] runs these commands, in order, before the program
        let REGISTRY_PRESETS = {
            gdi: 'reg add "HKCU\\Software\\Wine\\Direct3D" /v DirectDrawRenderer /t REG_SZ /d gdi /f',
            cdrom: 'reg add "HKLM\\Software\\Wine\\Drives" /v d: /d cdrom /f'
        };
        // wine-override=[name,...] adds these to WINEDLLOVERRIDES
        let WINE_OVERRIDE_PRESETS = {
            nomono: "mscoree,mshtml=" // skip the Wine Mono and Gecko installs
        };
        //params
        let Config = {};
        Config.locateRootBaseUrl = ""; // ie "assets/"
        Config.locateAppBaseUrl = "";
        Config.locateOverlayBaseUrl = "";
        Config.locateCDRomBaseUrl = "";
        Config.urlParams = "";
        Config.storageMode = STORAGE_INDEXED_DB;
        Config.rootStorageMode = "";
        Config.persist_d_drive = true;
        Config.showSaveJITCache = true;
        Config.runLinux = false;
        Config.WorkingDir = "";
        Config.PlayScript = "";
        Config.loadDesktop = false;
        Config.appSubfolder = "";
				
        var isRunning = false;
        var pointerLockEventsConfigured = false;

        (function() {
          function patchBufferData(contextPrototype) {
            if (!contextPrototype || contextPrototype.__boxedwineBufferDataPadding) {
              return;
            }
            contextPrototype.__boxedwineBufferDataPadding = true;
            function copyBytes(src, srcOffset, length) {
              if (!src) {
                return null;
              }
              var bytes = null;
              if (src instanceof ArrayBuffer) {
                bytes = new Uint8Array(src);
              } else if (ArrayBuffer.isView(src)) {
                bytes = new Uint8Array(src.buffer, src.byteOffset, src.byteLength);
              } else if (typeof src.length === "number") {
                bytes = Uint8Array.from(src);
              }
              if (!bytes) {
                return null;
              }
              var start = srcOffset || 0;
              var end = typeof length === "number" ? start + length : bytes.length;
              return new Uint8Array(bytes.slice(start, end));
            }
            var originalBufferData = contextPrototype.bufferData;
            contextPrototype.bufferData = function(target, srcOrSize, usage, srcOffset, length) {
              if (arguments.length === 5 && target === this.ARRAY_BUFFER && srcOrSize && typeof srcOffset === "number" && typeof length === "number") {
                var extraPadding = 256;
                var paddedLength = length + extraPadding;
                var padded = new Uint8Array(paddedLength);
                var sourceBytes = copyBytes(srcOrSize, srcOffset, length);
                if (sourceBytes) {
                  padded.set(sourceBytes);
                }
                return originalBufferData.call(this, target, padded, usage);
              }
              if (arguments.length === 3 && target === this.ARRAY_BUFFER && srcOrSize && typeof srcOrSize !== "number") {
                var sourceData = copyBytes(srcOrSize);
                if (sourceData) {
                  var viewPadding = 256;
                  var paddedView = new Uint8Array(sourceData.length + viewPadding);
                  paddedView.set(sourceData);
                  return originalBufferData.call(this, target, paddedView, usage);
                }
              }
              if (arguments.length === 3 && target === this.ARRAY_BUFFER && typeof srcOrSize === "number" && srcOrSize > 0) {
                var numericPadding = 256;
                return originalBufferData.call(this, target, srcOrSize + numericPadding, usage);
              }
              return originalBufferData.apply(this, arguments);
            };
          }
          patchBufferData(window.WebGLRenderingContext && window.WebGLRenderingContext.prototype);
          patchBufferData(window.WebGL2RenderingContext && window.WebGL2RenderingContext.prototype);
        })();

      	var statusElement = document.getElementById('status');
      	var progressElement = document.getElementById('progress');
      	var spinnerElement = document.getElementById('spinner');
        var dropzone = document.getElementById("dropzone");
        var hostFolderBusy = false;
        var awaitingStart = false;
        var hostFolderRefreshBusy = false;
        var hostFolderRefreshPendingReason = "";
        var hostFolderRefreshTimer = null;
        var hostFolderRefreshObserver = null;
        var hostFolderRefreshEventsConfigured = false;
        var hostFolderInvalidateWarningShown = false;

        function setConfiguration() {
            Config.appDirPrefix = DEFAULT_APP_DIRECTORY;
            Config.locateRootBaseUrl = getBaseUrl("rootBase");
            Config.locateAppBaseUrl = getBaseUrl("appBase");
            Config.locateOverlayBaseUrl = getBaseUrl("overlayBase");
            Config.storageMode = getStorageMode();
            Config.rootStorageMode = getRootStorageMode();
            Config.isAutoRunSet = getAutoRun();
            Config.loadDesktop = getLoadDesktop();
            Config.rootZipFile = getRootZipFile("root"); //MANUAL:"base.zip";
            Config.extraZipFiles = getZipFileList("overlay"); //MANUAL:"dlls.zip;fonts.zip";
            Config.appZipFile = getAppZipFile("app"); //MANUAL:"chomp.zip";
            Config.cdromISOFile = getCDRomISOFile("cdrom");
            Config.appPayload = getPayload("app-payload"); 
            Config.extraPayload = getPayload("overlay-payload"); 
            Config.Program = getExecutable(); //MANUAL:"CHOMP.EXE";
            Config.ProgramArgs = getProgramArgs();
            Config.registryCommands = getRegistryCommands();
            Config.dllOverrides = getPresets("wine-override", WINE_OVERRIDE_PRESETS).join(";");
            Config.WorkingDir = getWorkingDir();
            Config.PlayScript = getPlayScript();
            Config.runLinux = getRunLinux();
            Config.isSoundEnabled = getSound();
            Config.recordJITCache = getJitRecord();
            Config.wasmModuleBrokerEnabled = getWasmModuleBrokerEnabled();
            Config.safeMemory = getSafeMemory();
            Config.audioFreq = getAudioFreq();
            Config.disableHideCursor = getDisableHideCursor();
            Config.disableWasmJitForWrittenCode = getDisableWasmJitForWrittenCode();
            Config.bpp = getBitsPerPixel();
			Config.cpu = getCPU();
			Config.envProp = getEnvProp();
			Config.emEnvProps = getEmscriptenEnvProps();
			Config.frameSkip = getFrameSkip();
			Config.resolution = getResolution();
			Config.networking = getNetworkingConfig();
			Config.payloadZipFile = "app.zip";
			Config.hostFolderHandle = null;
			Config.hostFolderPersistedHandle = null;
			Config.hostFolderPersistedName = "";
			Config.hostFolderPermissionState = "unknown";
			Config.hostFolderEnabled = false;
			Config.hostFolderReadWrite = true;
			Config.hostFolderManifest = new Map();
			Config.hostFolderMounted = false;
            configureBrowserStorage();
        }
        function configureBrowserStorage() {
            // IDBFS names databases after their mount points. Keep each game's
            // Wine prefix and D: drive together, independent of build or root ZIP.
            // storageKey= overrides the per-game key so separate launches can share
            // one store; storageKey=default selects the desktop/upload storage.
            let storageKey = allowParameterOverride() ? decodeUrlValue(getParameter("storageKey")).trim() : "";
            let key = "";
            if (storageKey.length > 0) {
                key = storageKey.toLowerCase() === "default" ? "" : "key/" + encodeURIComponent(storageKey);
                console.log("setting storage key to: " + (key || "default"));
            } else if (Config.appZipFile.length > 0) {
                key = "app/" + encodeURIComponent(decodeUrlValue(Config.appZipFile));
            } else if (Config.extraZipFiles.length > 0) {
                key = "overlay/" + Config.extraZipFiles.map(function(name) {
                    return encodeURIComponent(decodeUrlValue(name));
                }).join("+");
            }
            // Desktop/upload launches without an archive retain their existing storage.
            ROOT = key ? "/root/" + key : "/root";
            Config.d_drive = key ? "/d_drive/" + key : "/d_drive";
        }
        function getStorageMode() {
            var storage = getParameter("storage").trim().toLowerCase();
            if (!allowParameterOverride() || storage === "") {
                if (allowParameterOverride() && getParameter("regressionBuild").trim() !== "") {
                    console.log("setting storage mode to: " + STORAGE_MEMORY + " for regression build");
                    return STORAGE_MEMORY;
                }
                return Config.storageMode;
            }
            if (storage === "memory" || storage === "mem") {
                console.log("setting storage mode to: " + STORAGE_MEMORY);
                return STORAGE_MEMORY;
            }
            if (storage === "indexed_db" || storage === "indexeddb" || storage === "idb") {
                console.log("setting storage mode to: " + STORAGE_INDEXED_DB);
                return STORAGE_INDEXED_DB;
            }
            if (storage === "hostfolder" || storage === "host_folder" || storage === "host-folder") {
                console.log("setting storage mode to: " + STORAGE_HOST_FOLDER);
                return STORAGE_HOST_FOLDER;
            }
            console.log("unknown storage mode: " + storage);
            return Config.storageMode;
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
        function getNetworkingConfig() {
            var mode = getParameter("network").trim().toLowerCase();
            if (!allowParameterOverride() || mode.length == 0 || mode == "false" || mode == "off") {
                console.log("networking disabled");
                return { enabled: false };
            }
            if (mode != "true" && mode != "websocket" && mode != "tcp") {
                console.log("networking mode not recognized: " + mode);
                return { enabled: false };
            }
            var gatewayUrl = decodeUrlValue(getParameter("networkGateway").trim());
            var debug = getParameter("networkDebug").trim().toLowerCase();
            if (gatewayUrl.length == 0) {
                var protocol = window.location.protocol === "https:" ? "wss:" : "ws:";
                var hostname = window.location.hostname || "127.0.0.1";
                gatewayUrl = protocol + "//" + hostname + ":8001/boxedwine-network";
            }
            console.log("networking enabled through gateway: " + gatewayUrl);
            return {
                enabled: true,
                mode: "websocket",
                gatewayUrl: gatewayUrl,
                debug: debug == "true" || debug == "1"
            };
        }
        function getEnvProp() {
            var property = getParameter("env").trim();
            if(allowParameterOverride()){
                if(property.length > 6) {
                	if( (property.startsWith("%22") && property.endsWith("%22") )
                		|| (property.startsWith('%27') && property.endsWith('%27'))){
                    	let kv = property.substring(3, property.length - 3).split(':');
                    	return kv[0].trim() + "=" + kv[1].trim();
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
            if (Config.storageMode === STORAGE_HOST_FOLDER) {
                auto = false;
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
        function getRootStorageMode() {
            var defaultRootStorageMode = Config.storageMode === STORAGE_MEMORY ? STORAGE_MEMORY : STORAGE_INDEXED_DB;
            var rootStorageMode = Config.rootStorageMode || defaultRootStorageMode;
            var rootStorageParam = getParameter("rootfs");
            if (allowParameterOverride() && rootStorageParam.length > 0) {
                rootStorageMode = rootStorageParam;
            }
            rootStorageMode = rootStorageMode.toLowerCase();
            if (rootStorageMode === "memory") {
                rootStorageMode = STORAGE_MEMORY;
            } else if (rootStorageMode === "indexeddb" || rootStorageMode === "indexed_db") {
                rootStorageMode = STORAGE_INDEXED_DB;
            } else {
                rootStorageMode = defaultRootStorageMode;
            }
            console.log("setting root fs storage mode to: " + rootStorageMode);
            return rootStorageMode;
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

        function getWasmModuleBrokerEnabled() {
            var value = getParameter("wasmModuleBroker");
            if (!allowParameterOverride()) {
                return true;
            }
            return value != "0";
        }

        function getSafeMemory() {
            return allowParameterOverride() && getParameter("safe-memory") === "true";
        }

        function getAudioFreq() {
            var audioFreq = getParameter("audioFreq");
            if (!allowParameterOverride()) {
                audioFreq = DEFAULT_AUDIO_FREQ;
            } else if (audioFreq == "22050") {
                audioFreq = 22050;
            } else if (audioFreq == "11025" || audioFreq == "") {
                audioFreq = DEFAULT_AUDIO_FREQ;
            } else {
                audioFreq = DEFAULT_AUDIO_FREQ;
            }
            console.log("setting audioFreq to: " + audioFreq);
            return audioFreq;
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

        function getDisableWasmJitForWrittenCode() {
            var disableWasmJitForWrittenCode = getParameter("disableWasmJitForWrittenCode");
            if (!allowParameterOverride()) {
                disableWasmJitForWrittenCode = false;
            } else if (disableWasmJitForWrittenCode == "true") {
                disableWasmJitForWrittenCode = true;
                console.log("setting disableWasmJitForWrittenCode to: " + disableWasmJitForWrittenCode);
            } else {
                disableWasmJitForWrittenCode = false;
            }
            return disableWasmJitForWrittenCode;
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
                prog = decodeUrlValue(prog);
                console.log("setting program to execute to: "+prog);
            }
            return prog;
        }
        function getProgramArgs() {
            var args = getParameter("args");
            if (!allowParameterOverride() || args === "") {
                return [];
            }
            args = decodeUrlValue(args);
            let result = splitCommandLine(args);
            if (result.length > 0) {
                console.log("setting program arguments to: " + result.join(" "));
            }
            return result;
        }
        // param=[name,...] -> the values of the named presets, in order
        function getPresets(param, presets) {
            var list = getParameter(param);
            if (!allowParameterOverride() || list === "") {
                return [];
            }
            list = decodeUrlValue(list).trim();
            if (list.startsWith("[") && list.endsWith("]")) {
                list = list.substring(1, list.length - 1);
            }
            let values = [];
            list.split(/[,;]/).forEach(function(item) {
                let name = item.trim().toLowerCase();
                if (name.length == 0) {
                    return;
                }
                if (!Object.prototype.hasOwnProperty.call(presets, name)) {
                    console.log("unknown " + param + " preset: " + name);
                    return;
                }
                console.log("setting " + param + " preset " + name + " to: " + presets[name]);
                values.push(presets[name]);
            });
            return values;
        }
        function getRegistryCommands() {
            return getPresets("registry", REGISTRY_PRESETS).map(function(command) {
                return splitCommandLine(command);
            });
        }
        function getWorkingDir() {
            var workingDir = getParameter("w");
            if (workingDir === "") {
                workingDir = getParameter("workingDir");
            }
            if (!allowParameterOverride() || workingDir === "") {
                return "";
            }
            workingDir = decodeUrlValue(workingDir);
            // c:/files -> /home/username/.wine/dosdevices/c:/files, as Wine resolves it for p
            let drive = workingDir.match(/^([a-z]):(?:[\/\\]|$)/i);
            if (drive) {
                workingDir = "/home/username/.wine/dosdevices/" + drive[1].toLowerCase() + ":/"
                    + workingDir.substring(drive[0].length).replace(/\\/g, "/");
            }
            console.log("setting working directory to: " + workingDir);
            return workingDir;
        }
        function getPlayScript() {
            var playScript = getParameter("play");
            if (!allowParameterOverride() || playScript === "") {
                return "";
            }
            playScript = decodeUrlValue(playScript);
            console.log("setting automation play script to: " + playScript);
            return playScript;
        }
        function getRunLinux() {
            var linux = getParameter("linux");
            if (!allowParameterOverride()) {
                linux = "false";
            }
            var result = linux == "true" || linux == "1";
            if (result) {
                console.log("running program directly as Linux executable");
            }
            return result;
        }
        function getCDRomISOFile(param) {
            var filename =  getParameter(param);
            if(!allowParameterOverride() || filename===""){
                filename = "";
                console.log("not setting " + param + " ISO file");
            }else{
                console.log("setting " + param + " ISO file to: "+filename);
            }
            return filename;
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
        function getBaseUrl(param) {

            var baseUrl = getParameter(param);
            if (!allowParameterOverride() || baseUrl === "") {
                return "";
            }
            baseUrl = decodeURIComponent(baseUrl);
            if (!baseUrl.endsWith("/")) {
                baseUrl += "/";
            }
            console.log("setting " + param + " to: " + baseUrl);
            return baseUrl;
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
        // WASM JIT persistent-cache key convention: 'v6-<eip>-<blockHash>'
        // where both values are 8-digit lowercase hex. Must stay in sync
        // with the inline key construction in the EM_JS bodies in
        // source/emulation/cpu/wasm/jitWasmCodeGen.cpp.
        // ---------------------------------------------------------------
        // v6 adds CPU::wasmJitHelperOp, shifting the offsets embedded in v5 modules.
        var BOXEDWINE_WASM_JIT_CACHE_VERSION = 'v6';
        function boxedwineWasmJitHex32(value) {
            return ('00000000' + ((value >>> 0).toString(16))).slice(-8);
        }
        function boxedwineWasmJitCacheKey(eip, blockHash) {
            return BOXEDWINE_WASM_JIT_CACHE_VERSION + '-' + boxedwineWasmJitHex32(eip) + '-' + boxedwineWasmJitHex32(blockHash);
        }
        function parseWasmJitCacheKey(filename) {
            var slash = Math.max(filename.lastIndexOf('/'), filename.lastIndexOf('\\'));
            var base = slash >= 0 ? filename.slice(slash + 1) : filename;
            var stem = base.toLowerCase().endsWith('.wasm') ? base.slice(0, -5) : base;
            var parts = stem.split('-');
            if (parts.length !== 3 || parts[0] !== BOXEDWINE_WASM_JIT_CACHE_VERSION ||
                    parts[1].length !== 8 || parts[2].length !== 8) return null;
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
        function buildCDRomFileSystems(callback) {
			if(Config.cdromISOFile.length > 0){
				loadFile(Config.locateCDRomBaseUrl, Config.cdromISOFile, (uint8Array) => {
					createFile("/", Config.cdromISOFile, uint8Array);
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
            console.log("Use Root FS storage mode: "+Config.rootStorageMode);
            FS.mkdirTree(ROOT);
            FS.mkdirTree(Config.d_drive);
            var mountedPersistentFileSystem = false;
            if (Config.rootStorageMode == STORAGE_INDEXED_DB) {
                FS.mount(IDBFS, {autoPersist: true}, ROOT);
                mountedPersistentFileSystem = true;
            }
            if (Config.storageMode == STORAGE_INDEXED_DB) {
                if (Config.persist_d_drive) {
                    FS.mount(IDBFS, {autoPersist: true}, Config.d_drive);
                    mountedPersistentFileSystem = true;
                }
            }
            if (mountedPersistentFileSystem) {
                FS.syncfs(true, function (err) {
                    if (err) {
                        console.log('unable to sync browser storage');
                    } else {
                        callback();
                    }
                });
            } else {
                callback();
            }
        }
        function buildBrowserFileSystem() {
            // The Save JIT Cache button is handled by updateSaveJitButton(),
            // called from initWasmJitCache once the persistence mode is known.
            spinnerElement.style.display = 'none';
            toggleConsole();
            if(Config.isAutoRunSet){
                start();
            }else{
                // Downloads are done, but the "setupBoxedWine" run dependency stays
                // pending until start(), so emscripten will not clear the status yet.
                awaitingStart = true;
                Module.setStatus('');
                var startBtn = document.getElementById('startbtn');
                startBtn.disabled = false;
                startBtn.textContent = getHostFolderStartButtonText();
                startBtn.style.display = "";
                // Get File(s) copies files off d:; files go in by drag and drop or a mounted host folder
                document.getElementById('downloadbtn').style.display = "";
                updateHostFolderControls();
            }
        }
        function closeGetFilesModal() { //called by boxedwine.html
		}
        // The dialogs open and close through the URL fragment (CSS :target), so
        // closing one leaves #close in the address bar; drop it once closed.
        function removeCloseFragment() {
            if (window.location.hash === "#close") {
                history.replaceState(null, "", window.location.pathname + window.location.search);
            }
        }
        window.addEventListener("hashchange", removeCloseFragment);
        async function start() { //called by boxedwine.html
        	if(isRunning){
                return;
            }
            if (hostFolderBusy) {
                return;
            }
            if (isHostFolderStorage() && !Config.hostFolderEnabled) {
                if (!(await mountHostFolder())) {
                    updateHostFolderControls();
                    return;
                }
            }
            startEmulator();
        }
        function startEmulator() {
            isRunning = true;

            document.getElementById('startbtn').style.display = 'none';
            if (!isPointerLockEnabled()) {
                Config.disableHideCursor = true;
            }

            var params = getEmulatorParams();
            if (!Module['arguments']) {
                Module['arguments'] = [];
            }
            Module['arguments'].length = 0;
            for(var i=0; i < params.length; i++) {
                Module['arguments'].push(params[i]);
            }

            document.getElementById('startbtn').textContent = "Running...";
            awaitingStart = false;
            Module["removeRunDependency"]("setupBoxedWine");
        }
        function isPointerLockEnabled() {
            var pointerLock = document.getElementById('pointerLock');
            return pointerLock && pointerLock.checked;
        }
        function requestCanvasPointerLock(canvas) {
            if (!canvas || !isPointerLockEnabled() || document.pointerLockElement === canvas) {
                return;
            }
            if (typeof canvas.requestPointerLock !== "function") {
                console.log("Pointer lock is not supported by this browser");
                return;
            }
            try {
                var result = canvas.requestPointerLock();
                if (result && typeof result.catch === "function") {
                    result.catch(function(error) {
                        console.log("Unable to lock pointer: " + error);
                    });
                }
            } catch (error) {
                console.log("Unable to lock pointer: " + error);
            }
        }
        function togglePointerLock() {
            var canvas = document.getElementById('canvas');
            if (isPointerLockEnabled()) {
                requestCanvasPointerLock(canvas);
            } else if (document.pointerLockElement === canvas && typeof document.exitPointerLock === "function") {
                document.exitPointerLock();
            }
        }
        function setupPointerLock(canvas) {
            var pointerLock = document.getElementById('pointerLock');
            if (!canvas || !pointerLock) {
                return;
            }
            pointerLock.addEventListener("change", togglePointerLock, false);
            canvas.addEventListener("click", function() {
                requestCanvasPointerLock(canvas);
            }, false);
            if (!pointerLockEventsConfigured) {
                pointerLockEventsConfigured = true;
                document.addEventListener("pointerlockchange", function() {
                    console.log(document.pointerLockElement === canvas ? "Pointer locked" : "Pointer unlocked");
                }, false);
                document.addEventListener("pointerlockerror", function() {
                    console.log("Unable to lock pointer");
                }, false);
            }
        }
        var initialSetup = function(){
            console.log("running initial setup");
            setConfiguration();
            Module["boxedwineNetworking"] = Config.networking;
            if (Config.emEnvProps.length > 0) {
            	Config.emEnvProps.forEach(function(prop){
            		ENV[prop.key] = prop.value;
            	});
            }

            // preRun executes after the WASM exports have been installed. Only
            // JIT builds export this persistence latch and can use the cache.
            if (typeof Module._wasm_jit_set_persistence_active === 'function') {
                // Create /tmp-jit-modules so the JIT cache import path is valid.
                try { FS.mkdir('/tmp-jit-modules'); } catch(e) {}

                // Preload the JIT module cache (clear stale IndexedDB blocks, then
                // fetch the per-app server zip) before the emulator starts. This
                // dependency is resolved inside initWasmJitCache's callback.
                Module["addRunDependency"]("wasm-jit-cache");
                initWasmJitCache(function() {
                    Module["removeRunDependency"]("wasm-jit-cache");
                });
            }

            Module["addRunDependency"]("setupBoxedWine");
            loadPersistedHostFolderHandle().then(function() {
                initBrowserFilesystem(() => {
                    spinnerElement.style.display = '';
                    spinnerElement.hidden = false;

                    buildCDRomFileSystems(() => {
                        buildExtraFileSystems(() => {
                            buildAppFileSystem(() => {
                                loadFile(Config.locateRootBaseUrl, Config.rootZipFile, (rootZipfileBytes) => {
                                    createFile("/", Config.rootZipFile, rootZipfileBytes);
                                    buildBrowserFileSystem();
                                });
                            });
                        });
                    });
                });
            });
        }
        // Copies a dropped file, or a folder and everything in it, onto d: keeping
        // its folders. Finishes only once every file is written or has failed.
        async function copyDroppedEntry(entry, copied) {
            try {
                if (entry.isDirectory) {
                    let reader = entry.createReader();
                    let children;
                    do { // a folder's entries come in batches, ending with an empty one
                        children = await new Promise((resolve, reject) => reader.readEntries(resolve, reject));
                        for (let child of children) {
                            await copyDroppedEntry(child, copied);
                        }
                    } while (children.length > 0);
                } else {
                    let file = await new Promise((resolve, reject) => entry.file(resolve, reject));
                    let path = Config.d_drive + entry.fullPath;
                    FS.mkdirTree(path.substring(0, path.lastIndexOf("/")));
                    FS.writeFile(path, new Uint8Array(await file.arrayBuffer()));
                    addDroppedProgram(entry.fullPath, copied);
                }
            } catch (e) {
                copied.failed++;
                console.log("Unable to copy " + entry.fullPath + " to d: " + e);
            }
        }
        function addDroppedProgram(fullPath, copied) {
            let uppercase = fullPath.toUpperCase();
            if (uppercase.endsWith(".EXE") || uppercase.endsWith(".BAT")) {
                copied.exeFiles.push(fullPath);
            }
        }
        // A zip dropped on its own is unzipped into a d: folder named after it, so
        // game.zip becomes d:/game. Zips inside dropped folders stay as files.
        async function expandDroppedZip(entry, copied) {
            let zipName = entry.fullPath.substring(entry.fullPath.lastIndexOf("/") + 1);
            let folderName = zipName.substring(0, zipName.length - 4);
            let items = [];
            let data = null;
            try {
                if (/^\.*$/.test(folderName)) {
                    throw new Error("no folder name");
                }
                let file = await new Promise((resolve, reject) => entry.file(resolve, reject));
                data = new Uint8Array(await file.arrayBuffer());
                items = getZipEntries(data).slice(1).map(function(zipEntry) {
                    let parts = zipEntry.filename.replace(/\\/g, "/").split("/").filter((part) => part != "" && part != ".");
                    return { zipEntry: zipEntry, parts: parts };
                }).filter((item) => item.parts[0] != "__MACOSX"); // macOS Finder metadata, never needed
                if (items.length == 0) {
                    throw new Error("not a zip file");
                }
            } catch (e) {
                copied.failed++;
                console.log("Unable to unzip " + entry.fullPath + " to d: " + e);
                return;
            }
            // zip -r game.zip game/ already holds a game folder: don't make d:/game/game
            let top = folderName.toLowerCase();
            if (items.every((item) => item.parts.length > 0 && item.parts[0].toLowerCase() == top
                    && (item.parts.length > 1 || item.zipEntry.directory))) {
                items.forEach((item) => item.parts.shift());
            }
            for (let item of items) {
                if (item.parts.length == 0) {
                    continue;
                }
                let fullPath = "/" + folderName + "/" + item.parts.join("/");
                try {
                    // zip paths are untrusted: never write outside the folder
                    if (item.parts.indexOf("..") >= 0) {
                        throw new Error("path leaves the folder");
                    }
                    let path = Config.d_drive + fullPath;
                    if (item.zipEntry.directory) {
                        FS.mkdirTree(path);
                    } else {
                        FS.mkdirTree(path.substring(0, path.lastIndexOf("/")));
                        FS.writeFile(path, await extractZipEntry(data, item.zipEntry));
                        addDroppedProgram(fullPath, copied);
                    }
                } catch (e) {
                    copied.failed++;
                    console.log("Unable to unzip " + item.zipEntry.filename + " from " + entry.fullPath + " to d: " + e);
                }
            }
        }
        function populateModalExe(exeFiles) {
            var root = document.getElementById('items');
            root.textContent = '';
            let listElement = document.createElement("ul");
            for(let i = 0; i < exeFiles.length; i++) {
                let fullPath = exeFiles[i];
                let element = document.createElement("li");
                element.addEventListener("click", function(event){execute(fullPath);}, false);
                element.textContent = fullPath; // dropped file names are text, never HTML
                listElement.appendChild(element);
            }
            root.appendChild(listElement);
        }
        function execute(filename) {
            document.getElementById('openModalExeClick').click();

            var file = filename.substring(filename.lastIndexOf("/")+1, filename.length);
            var path = filename.substring(0, filename.lastIndexOf("/"));

            Config.WorkingDir = "/home/username/.wine/dosdevices/d:/" + path.substring(1);
            Config.Program = file;

            startEmulator();
        }
        dropzone.addEventListener("dragover", function(event){
            event.preventDefault();
        }, false);
        dropzone.addEventListener("drop", async function(event){
            event.preventDefault();
            // the dropped items are only readable until the first await
            let entries = Array.from(event.dataTransfer.items, (item) => item.webkitGetAsEntry()).filter(Boolean);
            // with auto run off, list the dropped programs so one can be started
            let showExecute = !Config.isAutoRunSet && !isRunning;
            let message = document.getElementById('message');
            if (showExecute) {
                document.getElementById('modalLinkExe').click();
                message.textContent = "Uploading files...";
                populateModalExe([]);
            }
            let copied = { exeFiles: [], failed: 0 };
            for (let entry of entries) {
                if (!entry.isDirectory && /\.zip$/i.test(entry.fullPath)) {
                    await expandDroppedZip(entry, copied);
                } else {
                    await copyDroppedEntry(entry, copied);
                }
            }
            if (showExecute) {
                let problems = [];
                if (copied.failed > 0) {
                    problems.push(copied.failed + (copied.failed == 1 ? " file or folder" : " files or folders") + " could not be copied to d:");
                }
                if (copied.exeFiles.length == 0) {
                    problems.push("No executable files found");
                }
                message.textContent = problems.join(". ");
                populateModalExe(copied.exeFiles);
            }
        }, false);
        function isHostFolderAccessSupported() {
            return typeof window !== "undefined" && typeof window.showDirectoryPicker === "function";
        }
        function isHostFolderStorage() {
            return Config.storageMode === STORAGE_HOST_FOLDER;
        }
        function getHostFolderPermissionMode() {
            return Config.hostFolderReadWrite ? "readwrite" : "read";
        }
        function getHostFolderStartButtonText() {
            if (!isHostFolderStorage()) {
                return "Start";
            }
            if (Config.hostFolderEnabled) {
                return "Start";
            }
            return Config.hostFolderPersistedHandle ? "Reconnect d:" : "Mount d:";
        }
        function updateHostFolderControls() {
            var startBtn = document.getElementById('startbtn');
            if (startBtn && !isRunning) {
                startBtn.textContent = getHostFolderStartButtonText();
                startBtn.disabled = hostFolderBusy;
            }
        }
        function logHostFolderMessage(message, isError) {
            if (!message) {
                return;
            }
            if (isError) {
                console.error(message);
            } else {
                console.log(message);
            }
            var output = document.getElementById('output');
            if (output) {
                output.value += message + "\n";
                output.scrollTop = output.scrollHeight;
            }
        }
        function setHostFolderStatus(message, isError) {
            if (message) {
                logHostFolderMessage(message, isError);
            }
        }
        function setHostFolderBusy(busy, message) {
            hostFolderBusy = busy;
            if (message) {
                setHostFolderStatus(message, false);
            }
            updateHostFolderControls();
        }
        function summarizeHostFolderRefresh(result) {
            if (!result) {
                return 0;
            }
            return (result.created || 0) + (result.updated || 0) + (result.removed || 0);
        }
        function getHostFolderRefreshMount() {
            let mount = getHostFolderMount();
            if (!mount || !mount.type || typeof mount.type.refreshFromHost !== "function") {
                return null;
            }
            return mount;
        }
        function syncHostFolderBoxedwineDirectories(mount, result) {
            if (typeof Module === "undefined" || typeof Module.ccall !== "function") {
                return false;
            }
            // Boxedwine only needs the directories whose entries changed, as native
            // paths relative to the mount ("." for the mount itself).
            let dirs = new Set();
            result.changedPaths.forEach(function(path) {
                let relative = path.substring(mount.mountpoint.length).replace(/^\/+/, "");
                let slash = relative.lastIndexOf("/");
                dirs.add(slash > 0 ? relative.substring(0, slash) : ".");
            });
            try {
                Module.ccall("boxedwineHostFolderSyncDirectories", null, ["string", "string"],
                    ["/mnt/drive_d", Array.from(dirs).sort().join("\n")]);
                return true;
            } catch (e) {
                if (!hostFolderInvalidateWarningShown) {
                    hostFolderInvalidateWarningShown = true;
                    logHostFolderMessage("Unable to notify Boxedwine about host folder refresh: " + e, true);
                }
                return false;
            }
        }
        function refreshHostFolderFromHost(reason) {
            if (!Config.hostFolderMounted) {
                return Promise.resolve(false);
            }
            let mount = getHostFolderRefreshMount();
            if (!mount) {
                return Promise.resolve(false);
            }
            if (hostFolderRefreshBusy) {
                hostFolderRefreshPendingReason = reason || "pending";
                return Promise.resolve(false);
            }
            hostFolderRefreshBusy = true;
            return new Promise(function(resolve) {
                mount.type.refreshFromHost(mount, function(err, result) {
                    hostFolderRefreshBusy = false;
                    if (err) {
                        logHostFolderMessage("Host folder refresh failed: " + err, true);
                        resolve(false);
                    } else {
                        let changeCount = summarizeHostFolderRefresh(result);
                        if (changeCount > 0) {
                            syncHostFolderBoxedwineDirectories(mount, result);
                            logHostFolderMessage("Refreshed D: folder from host (" + changeCount + " change" + (changeCount === 1 ? "" : "s") + ")", false);
                        }
                        resolve(changeCount > 0);
                    }
                    if (hostFolderRefreshPendingReason) {
                        let pendingReason = hostFolderRefreshPendingReason;
                        hostFolderRefreshPendingReason = "";
                        setTimeout(function() {
                            refreshHostFolderFromHost(pendingReason);
                        }, 0);
                    }
                });
            });
        }
        function scheduleHostFolderRefresh(reason) {
            if (!Config.hostFolderMounted) {
                return;
            }
            setTimeout(function() {
                refreshHostFolderFromHost(reason);
            }, 0);
        }
        async function startHostFolderObserver() {
            if (typeof FileSystemObserver !== "function" || !Config.hostFolderHandle) {
                return;
            }
            try {
                hostFolderRefreshObserver = new FileSystemObserver(function(records) {
                    for (let i = 0; i < records.length; i++) {
                        if (records[i].type === "errored") {
                            logHostFolderMessage("Host folder observer stopped; falling back to periodic refresh", true);
                            if (hostFolderRefreshObserver) {
                                hostFolderRefreshObserver.disconnect();
                            }
                            hostFolderRefreshObserver = null;
                            return;
                        }
                    }
                    scheduleHostFolderRefresh("observer");
                });
                await hostFolderRefreshObserver.observe(Config.hostFolderHandle, {recursive: true});
                logHostFolderMessage("Watching D: folder for host changes", false);
            } catch (e) {
                hostFolderRefreshObserver = null;
                logHostFolderMessage("Host folder change observer is unavailable; using periodic refresh", false);
            }
        }
        function startHostFolderRefresh() {
            if (!Config.hostFolderMounted || hostFolderRefreshTimer) {
                return;
            }
            if (!hostFolderRefreshEventsConfigured) {
                hostFolderRefreshEventsConfigured = true;
                window.addEventListener("focus", function() {
                    scheduleHostFolderRefresh("focus");
                }, false);
                document.addEventListener("visibilitychange", function() {
                    if (document.visibilityState === "visible") {
                        scheduleHostFolderRefresh("visible");
                    }
                }, false);
            }
            hostFolderRefreshTimer = setInterval(function() {
                refreshHostFolderFromHost("poll");
            }, HOST_FOLDER_REFRESH_INTERVAL_MS);
            startHostFolderObserver();
        }
        function stopHostFolderRefresh() {
            if (hostFolderRefreshTimer) {
                clearInterval(hostFolderRefreshTimer);
                hostFolderRefreshTimer = null;
            }
            if (hostFolderRefreshObserver) {
                hostFolderRefreshObserver.disconnect();
                hostFolderRefreshObserver = null;
            }
            hostFolderRefreshBusy = false;
            hostFolderRefreshPendingReason = "";
        }
        function isHostFolderHandlePersistenceSupported() {
            return typeof indexedDB !== "undefined";
        }
        function idbRequestAsPromise(request) {
            return new Promise(function(resolve, reject) {
                request.onsuccess = function() {
                    resolve(request.result);
                };
                request.onerror = function() {
                    reject(request.error);
                };
            });
        }
        function idbTransactionAsPromise(transaction) {
            return new Promise(function(resolve, reject) {
                transaction.oncomplete = function() {
                    resolve();
                };
                transaction.onerror = function() {
                    reject(transaction.error);
                };
                transaction.onabort = function() {
                    reject(transaction.error || new Error("IndexedDB transaction aborted"));
                };
            });
        }
        function openHostFolderHandleDb() {
            return new Promise(function(resolve, reject) {
                if (!isHostFolderHandlePersistenceSupported()) {
                    reject(new Error("IndexedDB is not available"));
                    return;
                }
                var request = indexedDB.open(HOST_FOLDER_HANDLE_DB, HOST_FOLDER_HANDLE_DB_VERSION);
                request.onupgradeneeded = function() {
                    var db = request.result;
                    if (!db.objectStoreNames.contains(HOST_FOLDER_HANDLE_STORE)) {
                        db.createObjectStore(HOST_FOLDER_HANDLE_STORE);
                    }
                };
                request.onsuccess = function() {
                    resolve(request.result);
                };
                request.onerror = function() {
                    reject(request.error);
                };
                request.onblocked = function() {
                    reject(new Error("Host folder handle database is blocked"));
                };
            });
        }
        async function readPersistedHostFolderRecord() {
            var db = await openHostFolderHandleDb();
            try {
                var transaction = db.transaction(HOST_FOLDER_HANDLE_STORE, "readonly");
                var store = transaction.objectStore(HOST_FOLDER_HANDLE_STORE);
                return await idbRequestAsPromise(store.get(HOST_FOLDER_HANDLE_KEY));
            } finally {
                db.close();
            }
        }
        async function writePersistedHostFolderHandle(handle) {
            if (!handle) {
                return;
            }
            var db = await openHostFolderHandleDb();
            try {
                var transaction = db.transaction(HOST_FOLDER_HANDLE_STORE, "readwrite");
                var store = transaction.objectStore(HOST_FOLDER_HANDLE_STORE);
                store.put({
                    handle: handle,
                    name: handle.name || "Host folder",
                    storedAt: Date.now()
                }, HOST_FOLDER_HANDLE_KEY);
                await idbTransactionAsPromise(transaction);
            } finally {
                db.close();
            }
        }
        async function deletePersistedHostFolderHandle() {
            var db = await openHostFolderHandleDb();
            try {
                var transaction = db.transaction(HOST_FOLDER_HANDLE_STORE, "readwrite");
                var store = transaction.objectStore(HOST_FOLDER_HANDLE_STORE);
                store.delete(HOST_FOLDER_HANDLE_KEY);
                await idbTransactionAsPromise(transaction);
            } finally {
                db.close();
            }
        }
        async function forgetPersistedHostFolderHandle(removeStoredHandle) {
            Config.hostFolderPersistedHandle = null;
            Config.hostFolderPersistedName = "";
            Config.hostFolderPermissionState = "unknown";
            updateHostFolderControls();
            if (!removeStoredHandle || !isHostFolderHandlePersistenceSupported()) {
                return;
            }
            try {
                await deletePersistedHostFolderHandle();
            } catch (e) {
                logHostFolderMessage("Unable to forget remembered D: folder: " + e, true);
            }
        }
        async function loadPersistedHostFolderHandle() {
            if (!isHostFolderStorage() || !isHostFolderAccessSupported()) {
                return;
            }
            if (!isHostFolderHandlePersistenceSupported()) {
                logHostFolderMessage("Remembered D: folder is not available because IndexedDB is disabled", false);
                return;
            }
            try {
                var record = await readPersistedHostFolderRecord();
                var handle = record && record.handle ? record.handle : record;
                if (!handle) {
                    return;
                }
                if (handle.kind && handle.kind !== "directory") {
                    logHostFolderMessage("Remembered D: folder is no longer a directory handle", true);
                    await forgetPersistedHostFolderHandle(true);
                    return;
                }
                var permissionState = await queryHostFolderPermission(handle, getHostFolderPermissionMode());
                Config.hostFolderPersistedHandle = handle;
                Config.hostFolderPersistedName = record && record.name ? record.name : (handle.name || "Host folder");
                Config.hostFolderPermissionState = permissionState;
                if (Config.hostFolderPermissionState === "granted") {
                    logHostFolderMessage("Remembered D: folder is ready to reconnect: " + Config.hostFolderPersistedName, false);
                } else if (Config.hostFolderPermissionState === "prompt") {
                    logHostFolderMessage("Remembered D: folder needs permission; click Reconnect d:", false);
                } else {
                    logHostFolderMessage("Remembered D: folder permission is denied; choose a folder again", true);
                    await forgetPersistedHostFolderHandle(true);
                }
            } catch (e) {
                await forgetPersistedHostFolderHandle(false);
                logHostFolderMessage("Unable to load remembered D: folder: " + e, true);
            }
        }
        async function rememberHostFolderHandle(handle) {
            Config.hostFolderPersistedHandle = handle;
            Config.hostFolderPersistedName = handle && handle.name ? handle.name : "Host folder";
            Config.hostFolderPermissionState = "granted";
            updateHostFolderControls();
            if (!isHostFolderHandlePersistenceSupported()) {
                logHostFolderMessage("D: folder mounted, but IndexedDB is disabled so it cannot be remembered", false);
                return;
            }
            try {
                await writePersistedHostFolderHandle(handle);
                logHostFolderMessage("Remembered D: folder for future sessions", false);
            } catch (e) {
                logHostFolderMessage("D: folder mounted, but could not be remembered: " + e, true);
            }
        }
        async function queryHostFolderPermission(handle, mode) {
            if (!handle || typeof handle.queryPermission !== "function") {
                return "granted";
            }
            return await handle.queryPermission({mode: mode});
        }
        async function ensureHostFolderPermission(handle, mode) {
            if (!handle || typeof handle.queryPermission !== "function") {
                return true;
            }
            let state = await queryHostFolderPermission(handle, mode);
            Config.hostFolderPermissionState = state;
            if (state === "granted") {
                return true;
            }
            if (typeof handle.requestPermission !== "function") {
                return false;
            }
            state = await handle.requestPermission({mode: mode});
            Config.hostFolderPermissionState = state;
            return state === "granted";
        }
        async function getHostFolderHandleForMount(mode) {
            if (Config.hostFolderPersistedHandle) {
                setHostFolderStatus("Reconnecting remembered D: folder...", false);
                if (await ensureHostFolderPermission(Config.hostFolderPersistedHandle, mode)) {
                    return Config.hostFolderPersistedHandle;
                }
                setHostFolderStatus("Remembered D: folder permission was not granted; click Mount d: to choose a folder", true);
                await forgetPersistedHostFolderHandle(true);
                return null;
            }
            return await window.showDirectoryPicker({
                id: "boxedwine-host-drive-d",
                mode: mode
            });
        }
        function getHostFolderMount() {
            if (!Config.hostFolderMounted) {
                return null;
            }
            let lookup = FS.lookupPath(Config.d_drive, {follow_mount: false});
            return lookup && lookup.node ? lookup.node.mounted : null;
        }
        function syncHostFolderMount(populate) {
            return new Promise(function(resolve, reject) {
                let mount = getHostFolderMount();
                if (!mount || !mount.type || typeof mount.type.syncfs !== "function") {
                    resolve();
                    return;
                }
                mount.type.syncfs(mount, populate, function(err) {
                    if (err) {
                        reject(err);
                    } else {
                        resolve();
                    }
                });
            });
        }
        async function unmountHostFolder() {
            if (!Config.hostFolderMounted) {
                return;
            }
            await syncHostFolderMount(false);
            FS.unmount(Config.d_drive);
            Config.hostFolderMounted = false;
            Config.hostFolderEnabled = false;
            Config.hostFolderHandle = null;
            stopHostFolderRefresh();
        }
        async function mountHostFolder() {
            if (isRunning || hostFolderBusy) {
                return false;
            }
            if (!isHostFolderStorage()) {
                setHostFolderStatus("Host folder mounting is only available when storage=hostfolder", true);
                return false;
            }
            if (!isHostFolderAccessSupported()) {
                setHostFolderStatus("Host folder mounting is not supported by this browser", true);
                return false;
            }
            if (typeof installFSFS !== "function") {
                setHostFolderStatus("Host folder mounting support was not loaded", true);
                return false;
            }
            try {
                let permissionMode = getHostFolderPermissionMode();
                let handle = await getHostFolderHandleForMount(permissionMode);
                if (!handle) {
                    return false;
                }
                setHostFolderBusy(true, "Mounting host folder...");
                if (!(await ensureHostFolderPermission(handle, permissionMode))) {
                    setHostFolderStatus("Host folder permission was not granted", true);
                    return false;
                }
                await unmountHostFolder();
                let fsfs = installFSFS();
                Config.hostFolderHandle = handle;
                FS.mount(fsfs, {
                    dirHandle: handle,
                    autoPersist: true,
                    onPersistError: function(error) {
                        logHostFolderMessage("Host folder auto-sync failed: " + error, true);
                    },
                    onPersistComplete: function() {
                        logHostFolderMessage("Synced D: folder", false);
                    },
                    onConflict: function(source, conflicts) {
                        if (conflicts.length > 0) {
                            logHostFolderMessage("Host folder " + source + " skipped " + conflicts.length + " conflicting path" + (conflicts.length === 1 ? "" : "s"), true);
                        }
                    }
                }, Config.d_drive);
                Config.hostFolderMounted = true;
                await syncHostFolderMount(true);
                Config.hostFolderEnabled = true;
                await rememberHostFolderHandle(handle);
                startHostFolderRefresh();
                setHostFolderStatus("Mounted D: folder (auto-sync enabled)", false);
                return true;
            } catch (e) {
                if (Config.hostFolderMounted && !Config.hostFolderEnabled) {
                    // The initial populate failed: drop the partial mount without
                    // syncing it back, so a retry does not hit the same error.
                    try { FS.unmount(Config.d_drive); } catch (unmountError) {}
                    Config.hostFolderMounted = false;
                    Config.hostFolderHandle = null;
                    stopHostFolderRefresh();
                }
                if (e && (e.name === "NotFoundError" || e.name === "NotAllowedError")) {
                    // The remembered folder was moved, deleted or revoked; forget it
                    // so the next click offers the folder picker again.
                    await forgetPersistedHostFolderHandle(true);
                }
                if (e && e.name === "AbortError") {
                    setHostFolderStatus("Host folder selection canceled", false);
                } else {
                    setHostFolderStatus("Unable to mount host folder: " + e, true);
                }
                return false;
            } finally {
                setHostFolderBusy(false);
            }
        }
        function getEmulatorParams() {
            let params = ["-root", ROOT];
            // overlays go first: the first zip holding a path wins, so an
            // overlay's files replace the same files in the root zip
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
            params.push("-zip");
            params.push(Config.rootZipFile);
            if(Config.cdromISOFile.length > 0){
				params.push("-cdrom");
				params.push(Config.cdromISOFile);
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
            if (Config.frameSkip != "0") {
            	params.push("-skipFrameFPS");
            	params.push(Config.frameSkip);
			}            
            
            if(!Config.isSoundEnabled){
                params.push("-nosound");
            }
            if(Config.audioFreq != DEFAULT_AUDIO_FREQ){
                params.push("-audioFreq");
                params.push("" + Config.audioFreq);
            }
            if (Config.disableHideCursor) {
                params.push("-disableHideCursor");
            }
            if (Config.disableWasmJitForWrittenCode) {
                params.push("-disableWasmJitForWrittenCode");
            }
            if (Config.safeMemory) {
                params.push("-safe-memory");
            }
            if(Config.bpp != DEFAULT_BPP){
                params.push("-bpp");
                params.push("" + Config.bpp);
            }
            if(Config.cpu.length > 0){
                params.push("-" + Config.cpu);
            }
            let dllOverrides = Config.dllOverrides;
            if(Config.envProp.length > 0){
                params.push("-env");
                if (dllOverrides.length > 0 && Config.envProp.startsWith("WINEDLLOVERRIDES=")) {
                    // keep a single WINEDLLOVERRIDES, the presets win for the same dll
                    params.push(Config.envProp + ";" + dllOverrides);
                    dllOverrides = "";
                } else {
                    params.push(Config.envProp);
                }
            }
            if (dllOverrides.length > 0) {
                params.push("-env");
                params.push("WINEDLLOVERRIDES=" + dllOverrides);
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
            if (Config.PlayScript.length > 0) {
                params.push("-play");
                params.push(Config.PlayScript);
            }
            if (Config.runLinux && Config.Program.length > 0 && !Config.loadDesktop) {
                params.push(Config.Program);
                for (let i = 0; i < Config.ProgramArgs.length; i++) {
                    params.push(Config.ProgramArgs[i]);
                }
            } else {
                params.push("/bin/wine");
                if (Config.registryCommands.length > 0) {
                    // cmd /c reg add ... && reg add ... && <program>
                    params.push("cmd");
                    params.push("/c");
                    for (let i = 0; i < Config.registryCommands.length; i++) {
                        params.push(...Config.registryCommands[i]);
                        params.push("&&");
                    }
                }
                if(Config.Program.length > 0 && !Config.loadDesktop){
                    if (Config.Program.endsWith('.bat')) {
                        params.push("cmd");
                        params.push("/c");
                    }
                    params.push(Config.Program);
                    for (let i = 0; i < Config.ProgramArgs.length; i++) {
                        params.push(Config.ProgramArgs[i]);
                    }
                }else{
	                params.push("explorer");
                    params.push("/desktop=shell");
                }
            }
            console.log("Emulator params:" + params);
            return params;
        }
      var Module = {
        preRun: [initialSetup],
        thisProgram: "./boxedwine", // argv[0]; emscripten's default is ./this.program
        arguments: [],
        postRun: [],
        boxedwineNetworking: { enabled: false },
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
            if (typeof Module._wasm_jit_mt_set_module_broker_enabled === 'function') {
                Module._wasm_jit_mt_set_module_broker_enabled(Config.wasmModuleBrokerEnabled ? 1 : 0);
            }
            if (Module.wasmJitPersistenceWanted &&
                    typeof Module._wasm_jit_set_persistence_active === 'function') {
                Module._wasm_jit_set_persistence_active();
            }
            // Record sessions also collect the fetchNext transition profile
            // for the Save-JIT-Cache export; replay sessions never export,
            // so they skip the recording cost.
            if (Config.recordJITCache === true &&
                    typeof Module._wasm_jit_set_record_active === 'function') {
                Module._wasm_jit_set_record_active();
            }
            if (typeof Module._wasm_jit_mt_register !== 'function') return;
            // A fully piped zip can have zero flat blocks, so the group
            // feeding below must not sit behind a flat-cache early return.
            if (!Module.wasmJitCache || Module.wasmJitCache.size === 0) {
                console.log('[WASM JIT MT] no flat blocks to preload');
            } else {
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
            }
            // Piped (grouped) modules: feed unpatched bytes + manifest rows
            // into the shared C++ registry (workers cannot reach this Module).
            if (Module.wasmJitMtPendingGroups && Module.wasmJitMtPendingGroups.length &&
                    typeof Module._wasm_jit_mt_register_group === 'function') {
                var writeCStr = function(s) {
                    var ptr = Module._malloc(s.length + 1);
                    if (!ptr) return 0;
                    for (var i = 0; i < s.length; i++) {
                        Module.HEAPU8[ptr + i] = s.charCodeAt(i) & 0xff; // export names are ASCII keys
                    }
                    Module.HEAPU8[ptr + s.length] = 0;
                    return ptr;
                };
                var groupCount = 0, entryCount = 0, patchCount = 0;
                Module.wasmJitMtPendingGroups.forEach(function(group) {
                    var ptr = Module._malloc(group.bytes.length);
                    if (!ptr) return;
                    Module.HEAPU8.set(group.bytes, ptr);
                    var groupIdx = Module._wasm_jit_mt_register_group(ptr, group.bytes.length);
                    Module._free(ptr);
                    if (groupIdx < 0) return;
                    groupCount++;
                    group.entries.forEach(function(entry) {
                        var cacheKey = parseWasmJitCacheKey(String(entry.key));
                        if (!cacheKey) return;
                        var namePtr = writeCStr(String(entry.exportName || entry.key));
                        if (!namePtr) return;
                        Module._wasm_jit_mt_register_group_entry(groupIdx,
                            cacheKey.eip, cacheKey.blockHash, namePtr, entry.relocCount >>> 0);
                        Module._free(namePtr);
                        entryCount++;
                    });
                    group.patches.forEach(function(patch) {
                        var targetKey = parseWasmJitCacheKey(String(patch.targetKey));
                        if (!targetKey) return;
                        Module._wasm_jit_mt_register_group_patch(groupIdx,
                            patch.offset >>> 0, targetKey.eip, targetKey.blockHash);
                        patchCount++;
                    });
                });
                console.log('[WASM JIT MT] registered ' + groupCount + ' piped groups (' +
                    entryCount + ' entries, ' + patchCount + ' direct-call patches)');
                Module.wasmJitMtPendingGroups = null;
            }
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
            var element = document.getElementById('output');
            if (element) {
              element.value += text + "\n";
              element.scrollTop = element.scrollHeight;
            }
          }
        },
        boxedwinePresentFrame: (function() {
          var canvas = null;
          var bitmapContext = null;
          var fallbackContext = null;
          var cropCanvas = null;
          var cropContext = null;
          function finishPresent(presentState) {
            if (presentState && typeof Atomics !== "undefined") {
              Atomics.store(presentState, 0, 0);
            }
          }
          return function(bitmap, presentState, sx, sy, sw, sh) {
            if (!bitmap) {
              finishPresent(presentState);
              return;
            }
            sx = Math.max(0, sx | 0);
            sy = Math.max(0, sy | 0);
            sw = Math.max(1, Math.min(bitmap.width - sx, sw | 0 || bitmap.width));
            sh = Math.max(1, Math.min(bitmap.height - sy, sh | 0 || bitmap.height));
            var cropped = sx !== 0 || sy !== 0 || sw !== bitmap.width || sh !== bitmap.height;
            if (!canvas) {
              canvas = document.getElementById('boxedwine-webgl-canvas-0');
            }
            if (!canvas) {
              if (typeof bitmap.close === "function") {
                bitmap.close();
              }
              finishPresent(presentState);
              return;
            }
            if (canvas.width !== sw) {
              canvas.width = sw;
            }
            if (canvas.height !== sh) {
              canvas.height = sh;
            }
            var canvasFrame = canvas.parentElement;
            if (canvasFrame) {
              canvasFrame.style.setProperty("--boxedwine-canvas-width", canvas.width || 800);
              canvasFrame.style.setProperty("--boxedwine-canvas-height", canvas.height || 600);
            }
            if (!bitmapContext) {
              bitmapContext = canvas.getContext("bitmaprenderer");
            }
            if (bitmapContext && typeof bitmapContext.transferFromImageBitmap === "function") {
              if (cropped) {
                if (!cropCanvas || cropCanvas.width !== sw || cropCanvas.height !== sh) {
                  cropCanvas = typeof OffscreenCanvas !== "undefined" ? new OffscreenCanvas(sw, sh) : document.createElement("canvas");
                  cropCanvas.width = sw;
                  cropCanvas.height = sh;
                  cropContext = null;
                }
                if (!cropContext) {
                  cropContext = cropCanvas.getContext("2d");
                }
                if (cropContext && typeof cropCanvas.transferToImageBitmap === "function") {
                  cropContext.clearRect(0, 0, sw, sh);
                  cropContext.drawImage(bitmap, sx, sy, sw, sh, 0, 0, sw, sh);
                  if (typeof bitmap.close === "function") {
                    bitmap.close();
                  }
                  bitmapContext.transferFromImageBitmap(cropCanvas.transferToImageBitmap());
                  finishPresent(presentState);
                  return;
                }
              }
              bitmapContext.transferFromImageBitmap(bitmap);
              finishPresent(presentState);
              return;
            }
            if (!fallbackContext) {
              fallbackContext = canvas.getContext("2d");
            }
            if (fallbackContext) {
              fallbackContext.drawImage(bitmap, sx, sy, sw, sh, 0, 0, sw, sh);
            }
            if (typeof bitmap.close === "function") {
              bitmap.close();
            }
            finishPresent(presentState);
          };
        })(),
        boxedwinePresentSoftwareFrame: (function() {
          var canvas = null;
          var context = null;
          function finishPresent(presentState) {
            if (presentState && typeof Atomics !== "undefined") {
              Atomics.store(presentState, 0, 0);
            }
          }
          return function(width, height, buffer, presentState) {
            if (!canvas) {
              canvas = document.getElementById('boxedwine-webgl-canvas-0');
            }
            if (!canvas || !buffer) {
              finishPresent(presentState);
              return;
            }
            if (canvas.width !== width) {
              canvas.width = width;
            }
            if (canvas.height !== height) {
              canvas.height = height;
            }
            var canvasFrame = canvas.parentElement;
            if (canvasFrame) {
              canvasFrame.style.setProperty("--boxedwine-canvas-width", width || 800);
              canvasFrame.style.setProperty("--boxedwine-canvas-height", height || 600);
            }
            if (!context) {
              context = canvas.getContext("2d");
            }
            if (context) {
              context.putImageData(new ImageData(new Uint8ClampedArray(buffer), width, height), 0, 0);
            }
            finishPresent(presentState);
          };
        })(),
        canvas: (function() {
          var canvas = document.getElementById('canvas');
          var canvasFrame = canvas ? canvas.parentElement : null;
          var updateCanvasFrameSize = function() {
            if (!canvasFrame || !canvas) {
              return;
            }
            var width = Number(canvas.width) || 800;
            var height = Number(canvas.height) || 600;
            canvasFrame.style.setProperty("--boxedwine-canvas-width", width);
            canvasFrame.style.setProperty("--boxedwine-canvas-height", height);
          };

          // As a default initial behavior, pop up an alert when webgl context is lost. To make your
          // application robust, you may want to override this behavior before shipping!
          // See http://www.khronos.org/registry/webgl/specs/latest/1.0/#5.15.2
          canvas.addEventListener("webglcontextlost", function(e) { alert('WebGL context lost. You will need to reload the page.'); e.preventDefault(); }, false);
          var originalGetContext = canvas.getContext.bind(canvas);
          canvas.getContext = function(type, attributes) {
            if (type === "webgl" || type === "webgl2" || type === "experimental-webgl") {
              attributes = Object.assign({}, attributes || {}, { preserveDrawingBuffer: true });
            }
            return originalGetContext(type, attributes);
          };
          setupCanvasFullscreen(canvas);
          setupPointerLock(canvas);
          canvas.width  = 800;
          canvas.height = 600;
          updateCanvasFrameSize();
          if (typeof MutationObserver !== "undefined") {
            new MutationObserver(updateCanvasFrameSize).observe(canvas, {
              attributes: true,
              attributeFilter: ["width", "height"]
            });
          }
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
          // Keep the status clear while the page waits for the user to press start.
          Module.setStatus(left && !awaitingStart ? 'Preparing... (' + (this.totalDependencies-left) + '/' + this.totalDependencies + ')' : '');
        }
      };
      Module.setStatus('Downloading...');
      // Suppress browser alert() dialogs so runtime failures do not block the
      // page and the developer console remains available for diagnostics.
      window.alert = function(msg) { console.error('[alert suppressed]', msg); };
      window.onerror = function(message, source, lineno, colno, error) {
        var details = message || 'unknown error';
        if (source) details += ' at ' + source + ':' + lineno + ':' + colno;
        if (error && error.stack) details += '\n' + error.stack;
        Module.printErr(details);
        Module.setStatus('Exception thrown, see JavaScript console');
        spinnerElement.style.display = 'none';
        Module.setStatus = function(text) {
          if (text) Module.printErr('[post-exception status] ' + text);
        };
      };
      window.onunhandledrejection = function(event) {
        var reason = event && event.reason;
        Module.printErr('Unhandled promise rejection: ' + (reason && reason.stack ? reason.stack : reason));
      };
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
// Fullscreen must include SDL's input/GL canvas and the sibling presentation
// canvas used by GDI and pthread GL. A fullscreen canvas hides its siblings,
// even when their CSS z-index would normally place them above it.
function setupCanvasFullscreen(canvas) {
    var target = document.getElementById('dropzone');
    if (!target || !target.contains(canvas)) return;
    ['requestFullscreen', 'webkitRequestFullscreen', 'webkitRequestFullScreen'].forEach(function(name) {
        if (typeof canvas[name] === 'function' && typeof target[name] === 'function') {
            canvas[name] = function(options) { return target[name](options); };
        }
    });
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
                     Module.wasmJitGroupEntryMap.size > 0 ||
                     (Module.wasmJitMtPendingGroups && Module.wasmJitMtPendingGroups.length > 0);
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
//   v6-*.wasm                            flat block modules -> wasmJitCache
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
        // Grouped reloc parity: merged group modules carry intra-group
        // direct-call sites whose second argument (the *target* block's
        // reloc slot array) is an i32.const placeholder. The loader
        // allocates a zeroed array per entry and patches each site with the
        // target's array address before the group compiles; the runtime
        // copies values in on each cache lookup, every table call passes
        // the block's own array as the second parameter, and free_block
        // zeroes arrays on eviction.
        var earlyGroupedManifest = manifestResults.find(function(m) { return m && m.format === 'boxedwine-wasm-jit-grouped-cache'; }) || null;
        if (earlyGroupedManifest &&
                earlyGroupedManifest.cacheVersion !== BOXEDWINE_WASM_JIT_CACHE_VERSION) {
            console.warn('[WASM JIT] grouped cache version ' +
                (earlyGroupedManifest.cacheVersion || 'unknown') + ' does not match ' +
                BOXEDWINE_WASM_JIT_CACHE_VERSION + '; ignoring grouped modules');
            earlyGroupedManifest = null;
        }
        // Grouped zips are build-shape-specific: MT-piped zips carry mt:true
        // (their direct-call tails have no ST slice-budget check), ST zips
        // don't. Loading the wrong shape would either bake the ST budget
        // address into MT workers or strip ST scheduling accounting.
        if (earlyGroupedManifest && !!earlyGroupedManifest.mt !== !!isMT) {
            console.warn('[WASM JIT] grouped cache zip was piped for ' +
                (earlyGroupedManifest.mt ? 'multi-threaded' : 'single-threaded') +
                ' builds but this build is ' + (isMT ? 'multi-threaded' : 'single-threaded') +
                '; ignoring grouped modules (re-run the pipeline against this build\'s recording)');
            earlyGroupedManifest = null;
        }
        var relocWorkByPath = new Map();
        if (earlyGroupedManifest && earlyGroupedManifest.groups && !isMT) {
            earlyGroupedManifest.groups.forEach(function(group) {
                var entries = (group.entries || []).filter(function(entry) {
                    return entry && (entry.relocCount >>> 0) > 0;
                });
                var patches = group.directCallPatches || [];
                if (!entries.length && !patches.length) return;
                relocWorkByPath.set(group.path, { entries: entries, patches: patches });
            });
        }
        if (!Module.wasmJitGroupRelocArrays) Module.wasmJitGroupRelocArrays = new Map();
        // Groups with reloc work are staged UNPATCHED: arrays and patched-in
        // addresses are per guest process (DecodedOp pointers are only valid
        // in one address space), so the runtime lookup path lazily allocates
        // a process's arrays, patches a private copy and compiles it on that
        // process's first touch of the group. Groups without reloc work are
        // process-agnostic and precompile shared, as before.
        if (!Module.wasmJitGroupUnpatched) Module.wasmJitGroupUnpatched = new Map();
        var relocPending = 0;
        var groupResults = isMT ? [] : await Promise.all(groupModules.map(async function(e) {
            try {
                var data = e.method === 8 ? await inflateRaw(e.data) : e.data;
                var relocWork = relocWorkByPath.get(e.path);
                if (relocWork) {
                    Module.wasmJitGroupUnpatched.set(e.path, {
                        bytes: new Uint8Array(data),
                        entries: relocWork.entries,
                        patches: relocWork.patches
                    });
                    relocPending++;
                    return null; // compiled per process by the lookup path
                }
                return { path: e.path, module: await WebAssembly.compile(data) };
            } catch(ex) {
                console.warn('[WASM JIT] grouped module precompile failed for path=' + e.path + ':', ex);
                return null;
            }
        }));
        // MT-piped groups: stage unpatched bytes + manifest rows for
        // onRuntimeInitialized to feed into the shared C++ group registry
        // (per-process patching, per-worker compilation, slot claiming all
        // happen on the C++/worker side).
        if (isMT && earlyGroupedManifest && groupModules.length > 0) {
            var mtRowsByPath = new Map();
            earlyGroupedManifest.groups.forEach(function(group) {
                mtRowsByPath.set(group.path, {
                    entries: group.entries || [],
                    patches: group.directCallPatches || []
                });
            });
            Module.wasmJitMtPendingGroups = [];
            await Promise.all(groupModules.map(async function(e) {
                try {
                    var data = e.method === 8 ? await inflateRaw(e.data) : e.data;
                    var rows = mtRowsByPath.get(e.path);
                    if (!rows) return;
                    Module.wasmJitMtPendingGroups.push({
                        bytes: new Uint8Array(data),
                        entries: rows.entries,
                        patches: rows.patches
                    });
                } catch(ex) {
                    console.warn('[WASM JIT] MT group stage failed for path=' + e.path + ':', ex);
                }
            }));
            console.log('[WASM JIT MT] staged ' + Module.wasmJitMtPendingGroups.length +
                ' piped groups for C++ registration');
        }

        var groupedManifest = earlyGroupedManifest;
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
                        ' relocGroups=' + relocPending +
                        ' functionImports=' + (imports.functionImportsBefore || 0) + '->' + (imports.functionImportsAfterGroupedEstimate || 0));
        } else {
            console.log('[WASM JIT] imported ' + valid.length +
                        ' blocks from server cache precompiled=' + Module.wasmJitCompiledCache.size);
        }
    } catch(e) {
        console.warn('[WASM JIT] server cache import failed:', e);
    }
}

// Export all cached JIT modules as a ZIP and trigger a browser download.
// createZip deflates the entries in parallel (typically halving file size versus
// STORED) and builds the ZIP entirely in JavaScript; minizip is not involved.
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
        var items = [];
        var exportedCacheKeys = new Set();
        Module.wasmJitCache.forEach(function(data, key) {
            key = String(key);
            if (!parseWasmJitCacheKey(key)) return;
            exportedCacheKeys.add(key);
            items.push({ name: key + '.wasm', data: data });
        });

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
                cacheVersion: BOXEDWINE_WASM_JIT_CACHE_VERSION,
                generatedAt: new Date().toISOString(),
                entryCount: manifestEntries.length,
                runtime: Module.wasmJitRuntimeConstants || null,
                entries: manifestEntries
            };
            items.push({
                name: 'boxedwine-jit-manifest.json',
                data: new TextEncoder().encode(JSON.stringify(manifest, null, 2))
            });
        }

        // Interior-transition profile sidecar; the pipeline parses it (or a
        // pasted console log) into profile-guided split hints.
        if (Module.wasmJitInteriorProfileLines && Module.wasmJitInteriorProfileLines.length > 0) {
            items.push({
                name: 'boxedwine-jit-profile.txt',
                data: new TextEncoder().encode(Module.wasmJitInteriorProfileLines.join('\n') + '\n')
            });
        }

        var zipFilename = getJitCacheZipFilename();
        downloadBlob(await createZip(items), zipFilename);
        console.log('[WASM JIT] exported ' + items.length + ' blocks to ' + zipFilename);
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
            var separator = param.indexOf("=");
            var key = separator >= 0 ? param.substring(0, separator) : param;
            if(key === inputKey){
                retVal = separator >= 0 ? param.substring(separator + 1) : "";
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
function decodeUrlValue(value) {
    try {
        return decodeURIComponent(value.replace(/\+/g, " "));
    } catch (e) {
        return value.split("%20").join(" ");
    }
}
function splitCommandLine(value) {
    let result = [];
    let current = "";
    let quote = "";
    for (let i = 0; i < value.length; i++) {
        let c = value[i];
        if (quote.length > 0) {
            if (c === quote) {
                quote = "";
            } else {
                current += c;
            }
        } else if (c === '"' || c === "'") {
            quote = c;
        } else if (/\s/.test(c)) {
            if (current.length > 0) {
                result.push(current);
                current = "";
            }
        } else {
            current += c;
        }
    }
    if (current.length > 0) {
        result.push(current);
    }
    return result;
}
var index = 0;
var files = [];
var selectedItem;
var selectedFilename;
var selectedPath = "";
var drives = []; // [{drive: "c:", dir}] for the tops of the Get File(s) tree
function driveOf(path) {
	return drives.find(function(drive) { return path.startsWith(drive.dir); });
}
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
	selectedPath = fullpath;
	// show the path as Wine sees it
	let drive = driveOf(fullpath);
	document.getElementById('selectedItem').value = drive.drive + "/" + fullpath.substring(drive.dir.length);
	selectedFilename = filename
}
function endsWith(str, suffix) {
    return str.indexOf(suffix, str.length - suffix.length) !== -1;
}
var zipping = false;
// Downloads the selected file, or the selected folder as a zip of everything in it.
async function extract() {
	let file = selectedPath;
	if (file == null || file.length == 0 || files.length <= 1 || zipping) {
		return;
	}
	if (!endsWith(file, "/")) {
		downloadBlob(new Blob([FS.readFile(file, { encoding: 'binary' })], {type: "octet/stream"}), selectedFilename);
		return;
	}
	// the top of a drive downloads as c.zip or d.zip; its folder name is internal
	let drive = driveOf(file);
	await downloadZip(file, files, file === drive.dir ? drive.drive.charAt(0) : file.substring(0, file.length - 1).split("/").pop());
}
// Zips everything Wine and programs changed (ROOT) as overlay.zip. Loaded with
// ?overlay=overlay.zip on a fresh browser, Wine starts without setting itself up again.
async function saveOverlay() {
	let entries = [];
	readFiles(ROOT + "/", entries);
	// leave out the emulator's deleted-file lists and its transient or virtual folders
	entries = entries.filter(function(entry) {
		let path = (entry.dir + entry.filename).substring(ROOT.length + 1);
		return !/^(tmp|dev|proc|mnt)\//.test(path) && !/^[^\/]*\.deleted$/.test(path);
	});
	await downloadZip(ROOT + "/", entries, "overlay");
}
async function downloadZip(folder, entries, name) {
	if (zipping) {
		return;
	}
	let status = document.getElementById('loadStatus');
	status.textContent = "Zipping " + name + "...";
	status.style.display = "";
	zipping = true;
	try {
		downloadBlob(await zipFolder(folder, entries), name + ".zip");
		status.style.display = "none";
	} catch (e) {
		status.textContent = "Unable to zip " + name + ": " + e.message;
	} finally {
		zipping = false;
	}
}
function downloadBlob(blob, filename) {
	let url = window.URL.createObjectURL(blob);
	let link = document.createElement('a');
	link.setAttribute('download', filename);
	link.setAttribute('href', url);
	document.body.appendChild(link);
	link.click();
	document.body.removeChild(link);
	window.URL.revokeObjectURL(url);
}
// Zips the entries (from readFiles) under folder, a path ending in "/", with
// paths relative to the folder.
async function zipFolder(folder, entries) {
	let items = [];
	entries.forEach(function(entry) {
		let path = entry.dir + entry.filename;
		if (path.length > folder.length && path.startsWith(folder)) {
			let item = { name: path.substring(folder.length), date: new Date(FS.stat(path).mtime) };
			if (entry.filename.length > 0) {
				item.data = FS.readFile(path);
			}
			items.push(item);
		}
	});
	return createZip(items);
}
// The tree is built with DOM nodes, never HTML strings: file names come from
// the emulated file system, so they must only ever be set as text.
function treeRow(content) {
	let row = document.createElement("tr");
	let cell = document.createElement("td");
	content.forEach(function(child) {
		cell.appendChild(child);
	});
	row.appendChild(cell);
	return row;
}
function treeItem(id, text, onClick) {
	let span = document.createElement("span");
	span.id = id + "-data";
	span.textContent = text;
	span.addEventListener("click", onClick, false);
	return span;
}
function treeToggle(id, suffix, label, display) {
	let span = document.createElement("span");
	span.id = id + suffix;
	span.style.display = display;
	let link = document.createElement("a");
	link.addEventListener("click", function() { toggleDirectory(id); }, false);
	let strong = document.createElement("strong");
	strong.textContent = label;
	link.appendChild(strong);
	span.appendChild(link);
	return span;
}
function leaf(entry) {
    index++;
	let id = String(index);
	return treeRow([treeItem(id, entry.filename, function() { select(id, entry.dir, entry.filename); })]);
}
function branch(entries) {
	var item = entries[index];
    index++;
	var dir = item.dir;
    var dirName = dir.substring(0, dir.length - 1);
    dirName = dirName.substring(dirName.lastIndexOf("/")+1,dirName.length);
	let id = String(index);
	let children = document.createElement("div");
	children.id = id;
	children.style.display = "none";
	let table = document.createElement("table");
    while(index < entries.length){
    	var nextItem = entries[index];
    	if(nextItem.dir === item.dir){
    		table.appendChild(leaf(nextItem));
    	}else if(parentDir(nextItem.dir) === item.dir){
    		table.appendChild(branch(entries));
    	}else{
    		break;
    	}
    }
	children.appendChild(table);
	return treeRow([
		treeToggle(id, "-expand", "+", ""),
		treeToggle(id, "-contract", "-", "none"),
		treeItem(id, "[" + (dir === driveOf(dir).dir ? driveOf(dir).drive : dirName) + "]", function() { select(id, dir, null); }),
		children
	]);
}
function parentDir(childDir) {
    if(endsWith(childDir,"/")){
        childDir = childDir.substring(0,childDir.length - 1);
    }
    return childDir.substring(0, childDir.lastIndexOf('/') + 1);
}
function buildGetFilesModal() {
	// c: only holds what programs created or changed: the rest of c: is still in the zips
	buildGetFilesModalForDrives({ "c:": ROOT + "/home/username/.wine/drive_c", "d:": Config.d_drive });
}
function buildGetFilesModalForDrives(folders) {
    document.getElementById('modalLink').click();
    let root = document.getElementById('tree');
    //reset
    document.getElementById('selectedItem').value = "";
    selectedPath = "";
    selectedFilename = null;
	files = [];
	drives = [];
    root.innerHTML = "";
    index = 0;

	let table = document.createElement("table");
	let tops = [];
	Object.keys(folders).forEach(function(drive) {
		let dir = folders[drive] + "/";
		if (!FS.analyzePath(dir).exists) {
			return; // c: has no files of its own until a program writes one
		}
		drives.push({ drive: drive, dir: dir });
		readFiles(dir, files);
		tops.push(String(index + 1));
		table.appendChild(branch(files));
	});
	document.getElementById('loadStatus').style.display="none";
	root.appendChild(table);
	tops.forEach(toggleDirectory);
}
function readFiles(currentDir, files) {
    console.log("adding directory: " + currentDir);
    files.push({dir : currentDir, filename : ""});
    var entries = FS.readdir(currentDir).filter(function(param) {
        return param !== "." && param !== "..";
    });
    entries.forEach(function(entry) {
        var node = FS.lookupPath(currentDir + entry, { follow: false }).node;
        if (FS.isLink(node.mode)) {
            return; // a link back up the tree would recurse forever
        }
        if (FS.isDir(node.mode)) {
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
    var compressionMethod = view.getUint16(offset + 10, true);
    var compressedSize = view.getUint32(offset + 20, true);
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
      // used by extractZipEntry
      encrypted: (bitFlag & 0x1) != 0,
      compressionMethod: compressionMethod,
      compressedSize: compressedSize,
      localHeaderOffset: view.getUint32(offset + 42, true),
    });
    offset += 46 + fileNameLength + extraFieldLength + fileCommentLength;
  }
  return entries;
};
// Returns the uncompressed bytes of a getZipEntries() entry. Stored and
// deflated entries are supported, using the browser's own inflate.
async function extractZipEntry(data, entry) {
  if (entry.encrypted || (entry.compressionMethod != 0 && entry.compressionMethod != 8)) {
    throw new Error("unsupported zip entry (encrypted or compression method " + entry.compressionMethod + ")");
  }
  var view = new DataView(data.buffer, data.byteOffset, data.length);
  var offset = entry.localHeaderOffset;
  if (offset + 30 > data.length || view.getUint32(offset) != 0x504b0304) {
    throw new Error("bad zip local header");
  }
  // the local header's name and extra field lengths can differ from the central directory's
  var start = offset + 30 + view.getUint16(offset + 26, true) + view.getUint16(offset + 28, true);
  var compressed = data.subarray(start, start + entry.compressedSize);
  return entry.compressionMethod == 0 ? compressed : inflateRaw(compressed);
}
var crc32Table = null;
function crc32(data) {
  if (!crc32Table) {
    crc32Table = new Uint32Array(256);
    for (var n = 0; n < 256; n++) {
      var c = n;
      for (var k = 0; k < 8; k++) {
        c = (c & 1) ? (0xEDB88320 ^ (c >>> 1)) : (c >>> 1);
      }
      crc32Table[n] = c >>> 0;
    }
  }
  var crc = 0xFFFFFFFF;
  for (var i = 0; i < data.length; i++) {
    crc = crc32Table[(crc ^ data[i]) & 0xFF] ^ (crc >>> 8);
  }
  return (crc ^ 0xFFFFFFFF) >>> 0;
}
// zip stores local time as MS-DOS date (high 16 bits) and time (low 16 bits)
function dosDateTime(date) {
  if (!(date.getFullYear() >= 1980)) {
    date = new Date(1980, 0, 1); // the earliest MS-DOS date, also used for invalid dates
  }
  return ((date.getFullYear() - 1980) << 25 | (date.getMonth() + 1) << 21 | date.getDate() << 16
    | date.getHours() << 11 | date.getMinutes() << 5 | date.getSeconds() >> 1) >>> 0;
}
// Builds a zip Blob from [{name, data, date}] for folder downloads and the JIT
// cache export. Folder names end in "/" and have no data; no date means now.
// Entries are deflated in parallel, each kept only if that makes it smaller.
async function createZip(items) {
  var bodies = await Promise.all(items.map(async function(item) {
    var data = item.data || new Uint8Array(0);
    var deflated = data.length > 0 ? await deflateRaw(data) : data;
    return deflated.length < data.length ? { method: 8, data: deflated } : { method: 0, data: data };
  }));
  var encoder = new TextEncoder();
  var parts = [];
  var directory = [];
  var offset = 0;
  items.forEach(function(item, i) {
    var name = encoder.encode(item.name);
    var data = item.data || new Uint8Array(0);
    var body = bodies[i].data;
    var method = bodies[i].method;
    if (offset + 30 + name.length + body.length > 0xFFFFFFFF) {
      throw new Error("too big for a zip (over 4 GB)");
    }
    var crc = crc32(data);
    var time = dosDateTime(item.date || new Date());
    var local = new DataView(new ArrayBuffer(30));
    local.setUint32(0, 0x04034b50, true);
    local.setUint16(4, 20, true);       // version needed: 2.0 for deflate and folders
    local.setUint16(6, 0x800, true);    // names are UTF-8
    local.setUint16(8, method, true);
    local.setUint32(10, time, true);
    local.setUint32(14, crc, true);
    local.setUint32(18, body.length, true);
    local.setUint32(22, data.length, true);
    local.setUint16(26, name.length, true);
    var entry = new DataView(new ArrayBuffer(46));
    entry.setUint32(0, 0x02014b50, true);
    entry.setUint16(4, 20, true);       // made by MS-DOS, so attributes below are DOS ones
    entry.setUint16(6, 20, true);
    entry.setUint16(8, 0x800, true);
    entry.setUint16(10, method, true);
    entry.setUint32(12, time, true);
    entry.setUint32(16, crc, true);
    entry.setUint32(20, body.length, true);
    entry.setUint32(24, data.length, true);
    entry.setUint16(28, name.length, true);
    entry.setUint32(38, item.name.endsWith("/") ? 0x10 : 0, true); // DOS directory attribute
    entry.setUint32(42, offset, true);
    parts.push(new Uint8Array(local.buffer), name, body);
    directory.push(new Uint8Array(entry.buffer), name);
    offset += 30 + name.length + body.length;
  });
  var directorySize = directory.reduce(function(size, part) { return size + part.length; }, 0);
  if (offset + directorySize > 0xFFFFFFFF) {
    throw new Error("too big for a zip (over 4 GB)");
  }
  if (items.length >= 0xFFFF) {
    // the end record's 16-bit counts can't hold this many entries, so add Zip64
    // end records; sizes and offsets still fit in 32 bits, so the entries need none
    var zip64 = new DataView(new ArrayBuffer(56 + 20));
    zip64.setUint32(0, 0x06064b50, true);              // Zip64 end of central directory record
    zip64.setUint32(4, 44, true);                      // size of the rest of the record
    zip64.setUint16(12, 45, true);                     // version made by: 4.5 for Zip64
    zip64.setUint16(14, 45, true);                     // version needed
    zip64.setUint32(24, items.length, true);           // entries on this disk
    zip64.setUint32(32, items.length, true);           // total entries
    zip64.setUint32(40, directorySize, true);
    zip64.setUint32(48, offset, true);                 // central directory offset
    zip64.setUint32(56, 0x07064b50, true);             // Zip64 end of central directory locator
    zip64.setUint32(64, offset + directorySize, true); // where the Zip64 record starts
    zip64.setUint32(72, 1, true);                      // total disks
    directory.push(new Uint8Array(zip64.buffer));
  }
  var end = new DataView(new ArrayBuffer(22));
  end.setUint32(0, 0x06054b50, true);
  end.setUint16(8, Math.min(items.length, 0xFFFF), true);
  end.setUint16(10, Math.min(items.length, 0xFFFF), true);
  end.setUint32(12, directorySize, true);
  end.setUint32(16, offset, true);
  return new Blob(parts.concat(directory, [new Uint8Array(end.buffer)]), { type: "application/zip" });
}
