These instructions will allow you to build your own version of Wine that can be used by Boxedwine.  I doubt very many people will every need to do this, but if you need a specific patch included in a particular version of Wine to run a particular game/app, then this is a good place to start.

This will build different versions of Wine, including the custom winex11.drv.so replacement that Boxedwine needs.

Freetype 2.8.1 broke the Wine build. So for Wine versions 2.18 or later, Freetype 2.8.1 or later must be used. This is why I use Debian 10 for Wine 3.0 and later and Debian 9 for Wine 2.0 or earlier.

1. Install Debian 9 or 10 (32-bit). This can be in a Virtual Machine, like VMWare or on a real machine.
> * VMWare Player: It will build Wine faster if you give it 4 CPUs.
> * For Wine 3.0 or higher Debian 10 , for example: debian-10.4.0-i386-netinst.iso
> * For Wine 1.6 to 2.0 Debian 9 , for example: debian-9.12.0-i386-netinst.iso
2. Make sure your user has sudo access. On Debian 10 you can do that by:
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
10. These zips only contain Wine, not the full file system. But the Wine zip files do contain a depends.txt file that tells Boxedwine to also load debian10.zip. If you would prefer to only have 1 file system zip file, the Wine zip and Debian 10 zip files can be merged together.
11. To prevent Wine creating the .wine directory when launching Boxedwine, I will run something like Winecfg with the new file system, then copy the premade /home/username/.wine folder in the Wine file system.

### Toubleshooting
If you get any build errors, like __acrt_iob_func undefined, try deleting the wine-git directory and build again.