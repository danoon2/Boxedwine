package common;

import java.io.File;
import java.nio.file.Files;
import java.util.ArrayList;
import java.util.List;
/*
Small utility to build a filesystem consisting of files that are commonly used
To be used in conjunction with &ondemand parameter

Input:
1. The output from the browser console when running boxedwine emscripten port with the Module parameter logReadFiles : true (see boxedwine-shell.js)
2. Directory path to a *COPY* of the filesystem used to generate the above

Output:
The above referenced boxedwine filesystem will be stripped of all files not referenced in the logfile

The output can then be zipped up and referenced using &overlay=common.zip
Note: 
1. the overlay parameter expects that the zip file structure does not begin with /root in the case of a root overlay.
a) cd root
b) zip -r common.zip .
 */
public class Main {

    private static final String FILE_TOKEN = " read file: ";
    public Main() throws Exception {

        String logFile = "/Users/blah/Desktop/base/wine-1.7-basic_and_daytona.txt";//notepad_and_daytona.txt";
        String baseDirectory = "/Users/blah/Desktop/base"; //do not include /root at the end.
        List<String> commonFiles = extractCommon(logFile);
        makeCommonFS(commonFiles, baseDirectory);
        //List<String> remainingFiles = verify(directory);
        //diff(commonFiles, remainingFiles);
        System.out.println("DONE");
    }
    private void diff(List<String> left, List<String> right) {
        int count = 0;
        for(String item : left) {
            if(!right.contains(item)) {
                System.out.println("not found:" + item);
                count++;
            }
        }
        System.out.println("total:" + count);
    }
    private List<String> verify(String directory) {
        File outputDir = new File(directory+ "/root");
        if (!outputDir.exists()) {
            throw new Error("directory not found:" + directory);
        }
        return recurseSubdirectory(directory, outputDir);
    }

    private List<String> recurseSubdirectory(String directory, File outputDir) {
        List<String> filesAdPaths = new ArrayList<>();
        File[] subDirs = outputDir.listFiles();
        if(subDirs != null && subDirs.length > 0) {
            for (File file : subDirs) {
                if (file.isFile()) {
                    String path = file.getAbsolutePath().substring(directory.length());
                    //System.out.println("file:" + path);
                    filesAdPaths.add(path);
                }else{
                    filesAdPaths.addAll(recurseSubdirectory(directory, file));
                }
            }
        }
        return filesAdPaths;
    }

    private List<String> extractCommon(String logFile) throws Exception {
        List<String> filesAndPaths = new ArrayList<>();
        File file = new File(logFile);
        List<String> lines = Files.readAllLines(file.toPath());
        for (String line : lines) {
            if (line.contains(FILE_TOKEN)) {
                String fileAndPath = extractFile(line);
                //System.out.println("fileAndPath=" + fileAndPath);
                if (fileAndPath.startsWith("/root")
                        && !fileAndPath.startsWith("/root/tmp/")){
                       // && !fileAndPath.contains("notepad")) {
                    filesAndPaths.add(fileAndPath);
                }
            }
        }
        return filesAndPaths;
    }
    private void makeCommonFS(List<String> filesAndPaths, String directory) {
        File outputDir = new File(directory+ "/root");
        if (!outputDir.exists()) {
            throw new Error("directory not found:" + directory);
        }
        recurseSubdirectory(directory, filesAndPaths, outputDir);
    }

    private void recurseSubdirectory(String directory, List<String> filesAndPaths, File outputDir) {
        File[] subDirs = outputDir.listFiles();
        if(subDirs != null && subDirs.length > 0) {
            for (File file : subDirs) {
                if (file.isFile()) {
                    recurseDelete(directory, filesAndPaths, file);
                }else{
                    recurseSubdirectory(directory, filesAndPaths, file);
                }
            }
            subDirs = outputDir.listFiles();
            if(subDirs == null || subDirs.length == 0) {
                if(outputDir.isDirectory()) {
                    outputDir.delete();
                }
            }
        } else {
            if(outputDir.isDirectory()) {
                outputDir.delete();
            }
        }
    }
    private boolean recurseDelete(String directory, List<String> filesAndPaths, File file) {
        String path = file.getAbsolutePath().substring(directory.length());
        boolean found = false;
        for(String fileAndPath : filesAndPaths) {
            if(fileAndPath.startsWith(path)){
                found = true;
                break;
            }
        }
        if(!found) {
            file.delete();
            //System.out.println("deleting:" + path);
            return true;
        }
        return false;
    }

    private String extractFile(String line) {
        String entry = line.substring(line.indexOf(FILE_TOKEN) + FILE_TOKEN.length()).trim();
        String removeBase = entry.replace("/root/base","/root");
        return removeBase;
    }
    public static void main(String[] args) throws Exception {
        new Main();
    }
}
