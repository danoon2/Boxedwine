package org.boxedwine;

import java.io.File;

public class Settings {
    static public File fileCachePath;
    static public File outputDir;
    static public File extraFiles;
    static public File finishedZip; // cannot be in ouputDir
    static public String boxedwinePath;
    static public String winePath; // wine zip file system so that ld will see libwine
}
