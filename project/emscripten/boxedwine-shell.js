        let ALLOW_PARAM_OVERRIDE_FROM_URL = true;
        let ROOT = "/root";
        let STORAGE_INDEXED_DB = "INDEXED_DB";
        let STORAGE_MEMORY = "MEMORY";

        let DEFAULT_AUTO_RUN = true;
        let DEFAULT_SOUND_ENABLED = true;
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
        Config.WorkingDir = "";
				
        var isRunning = false;
        var ExeFileTimer = null;

      	var statusElement = document.getElementById('status');
      	var progressElement = document.getElementById('progress');
      	var spinnerElement = document.getElementById('spinner');
        var dropzone = document.getElementById("dropzone");

        function setConfiguration() {
            Config.appDirPrefix = DEFAULT_APP_DIRECTORY;
            Config.isAutoRunSet = getAutoRun();
            Config.rootZipFile = getRootZipFile("root"); //MANUAL:"base.zip";
            Config.extraZipFiles = getZipFileList("overlay"); //MANUAL:"dlls.zip;fonts.zip";
            Config.appZipFile = getAppZipFile("app"); //MANUAL:"chomp.zip";
            Config.appPayload = getPayload("app-payload"); 
            Config.extraPayload = getPayload("overlay-payload"); 
            Config.Program = getExecutable(); //MANUAL:"CHOMP.EXE";
            Config.isSoundEnabled = getSound();
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

            if(Config.WorkingDir.length > 0){
                params.push("-w");
                params.push(Config.WorkingDir);
            }else if(Config.appPayload.length > 0 && Config.Program.length > 0 && Config.Program.substring(0 ,1) != "/"){
                params.push("-w");
                params.push(Config.appDirPrefix);
            }else if(Config.appZipFile.length > 0 && Config.Program.length > 0 && Config.Program.substring(0 ,1) != "/"){
                params.push("-w");
                params.push(Config.appDirPrefix);
            }
        	params.push("/bin/wine");
            if(Config.Program.length > 0){
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