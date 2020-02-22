The goal is to run one script and create a file system that Boxedwine
can use.  Currently this is not how it works.  The Debian based
file system is created by hand.

buildAll.sh currently builds different versions of Wine that are ready
to be merged with the base file system.  After that things like creating
the initial .wine directory need to be done as well as making sure ldconfig
has run while /opt/wine/lib/libwine.so is installed.
