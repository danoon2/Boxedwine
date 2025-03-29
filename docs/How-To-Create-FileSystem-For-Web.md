To be able to run your application on the web using Boxedwine compiled with Emscripten to WASM, you need to put your files in a zip for Boxedwine to use.  There are a couple of ways to do this.  I will start with the easiest first.

Currently the Web/Emscripten building does not support any kind of 3D acceleration, but DirectDraw should work.

Also realize performance will not be great.  This build does not have a dynamic recompiler.

### The Easy way

This way assumes your application does not need to be installed and is pretty simple.

1. Download the sample build.

	You can get the lastest cutting edge compiled code from http://208.113.165.28:8080/job/Boxedwine/job/master/ Just download the artifact like build-115.zip or whatever version it is up to.

	This will contain all of the builds I test with on a regular basis including the Emscripten build which is in the Web directory in that download.

2. Try to run this build on your server with the demo game just to make sure everything works by browsing to test.html

3. Assuming step 2 went well, you can place your application in a zip file, see ski32.zip in the download as an example.

4. Modify test.html or create your own

	For the ski32.zip example, test.html will call

	boxedwine.html?app=ski32&p=ski32.exe

	So you can replace ski32 with the name of your zip file, without the zip extension and replace sk32.exe with the name of your application including the extension

### Slightly Harder Way

If your application needs to be installed and sets registry entries or installs files into the Windows directory then this way might work for you.

1.  I'd recommend at least following the above steps to verify that ski32.exe works for you.

2.  From the artifact you downloaded in the above steps from http://208.113.165.28:8080/job/Boxedwine/job/master/ You can run Boxedwine on your platform without any arguments.
	
	This will ask you if you want to download a file system if this is the first time you are running it.  I'm going to assume you say yes to this in the next steps.
	
3.  Decide which file system you want to use.  
	a. You can use boxedwine.zip that is used in the ski32 demo
		
	b. Or you can use a full file system of your choosing.

	The pros of using boxedwine.zip is that you will have a small file system.  The cons are that this small file system had some files removed in order to make it that small, so not all applications will work with it.  But a lot of applications and games should work with it just fine.  All the Demos in the Boxedwine UI work with it.
	
    I'd recommend trying boxedwine.zip unless you know it won't work for you.

4.  If you want to use a full file system continue with the following steps, but substitute boxedwine.zip with your full file system of choice or rename that file system to boxedwine.zip.
	
5.  You need to copy boxedwine.zip to the FileSystems2 folder in the Boxedwine data folder, you can find the location of the data folder in the Boxedwine UI under the Options tab.  On Windows this is "%APPDATA%\Boxedwine"
	
6.  Restart Boxedwine so that it will detect the boxedwine.zip file system.
	
7.  Go to the Install tab to install your application.  You can drag your installer onto the Boxedwine UI and it will default the install options for you.  Make sure you select "Wine 6.0 Minimal" as the file system, or if you are using a full file system then the name of that.  If you don't see the file system name, you will need to try steps 5 and 6 again.
	
8.  After your application is installed, you should try to run it.  If it doesn't run correctly and you are using the small boxedwine.zip, you should try a full file system.  Go to the Options tab of the Boxedwine UI and select "File Systems" on the left, then install "Wine 6.0".  Install your application using "Wine 6.0" as the file system.  If your application runs correctly with this, then you can't use small boxedwine.zip and you will need to use "Wine 6.0" as your file system, go back to step 4 and use the downloaded full filesystem.  If your application still doesn't work, you can look at the [Troubleshooting document](Troubleshooting-Games-Apps.md)
	
9.  Now go to the Container that has your installed application.  You can find this folder in the Boxedwine UI by going to the Containers tab and selecting your app on the left.  On the right hand side you will see "Storage Location", if you hit the "Open" Boxedwine should take your computer to that folder.  In that folder open "root".  There should be a subfolder called "home" there.  Using whatever zip tools you prefer, like 7zip, copy the home folder to boxedwine.zip, this will merge the new home folder contents with the existing home folder in boxedwine.zip.
	
10.  You now have a boxedwine.zip file system that contains your installed application and you should be able to run this on the web.

When it comes to running the application you just installed, probably the easiest way will be to create a bat file in home/username directory of boxedwine.zip.  For example, if you followed the above steps with the Caesar 3 demo, your games is installed in home\username\.wine\drive_c\SIERRA\CAESAR3DEMO inside boxedwine.zip.  A suitable bat file to run this will look like

	c:
	cd SIERRA\CAESAR3DEMO
	c3.exe

If you called your bat file "run.bat" then your web parameters would look

	boxedwine.html?p=run.bat

Notice there is no "app" parameter like in the ski32 demo because the application is not in a separate zip, it is all contained in boxedwine.zip.  We also don't need to worry about the path, because the default working directory is /home/username which is where you placed run.bat

### How to reduce the size of the full file system

1.  Install the application using the file system you want to shrink.
	
2.  In the Boxedwine UI check the "Cache Opened Files" option in the "Options" tab, or use the command line option "-cacheReads"
	
3.  Run your application.
	
4.  Now zip up the contents of your root folder.  In the Boxedwine UI, you can find this in the Container tab and selecting your application on the left.  On the right hand side you will see "Storage Location", if you hit the "Open" Boxedwine should take your computer to that folder.  
	
5.  This new zip will contain everything your application used from the file system so it is a complete mini file system tailored to your application.  Because of this you should probably exercise your application pretty well just to make sure it accesses all the files you might need from Wine.