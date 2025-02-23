These instructions will allow you to build your own version of Wine that can be used by Boxedwine.  I doubt very many people will every need to do this, but if you need a specific patch included in a particular version of Wine to run a particular game/app, then this is a good place to start.

1. Install Debian 11 (32-bit). This can be in a Virtual Machine, like VMWare or on a real machine.
> * Just to build faster, make sure you give it a couple of cpu in the VM
2. Make sure your user has sudo access. On Debian 11 you can do that by:
> * Hit Alt-Ctrl-F1 and login as root
> * Where username is the name of the user you created when installing Debian 10: "usermod -aG sudo username"
> * Hit Alt-F7 to go back to the graphical environment if you want. You will need to log out and back into your Window environment for the group change to take effect.
3. Install dependencies: at the command prompt, type:
> * sudo apt-get install build-essential
> * sudo apt-get install git
> * sudo apt-get build-dep wine
4. Install OSS header so that wineoss.drv.so will build
> * Download OSS source
> * Unzip the file, for example, "tar -xvf oss-v4.2-build2019-src-gpl.tar.bz2"
> * Make directory: "sudo mkdir -p /usr/lib/oss/include/sys"
> * copy include/soundcard.h to /usr/lib/oss/include/sys/soundcard.h: "sudo cp include/soundcard.h /usr/lib/oss/include/sys/"
5. In your home directory, checkout Boxedwine with this command "git clone https://github.com/danoon2/Boxedwine.git"
6. Change directory to "cd ~/Boxedwine/tools/buildWine"
7. Run script with Bash "bash buildAll.sh"
8. The script will ask for sudo access to install Wine into /opt. It will then zip it up. This is why its best to this in a VM instead on a real machine.
9. When it is all done, you should have some zipped up wine files in the ~/Boxedwine/tools/buildWine directory.

### Toubleshooting
If you get any build errors, like __acrt_iob_func undefined, try deleting the wine-git directory and build again.