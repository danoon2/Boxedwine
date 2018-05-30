package reduce;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Set;

public class Reduce {

    private static final String LOG_START_TOKEN = "running initial setup";
    private static final String READ_FILE = "read file:";
    private static final String  FILENAME_END_MARKER = "printErr @ ";

    private int deleteFileCount = 0;
    private int deleteDirectoryCount = 0;
    
    Set<String> filesKept = new HashSet<>();

    private Reduce(ArrayList<String> logContents, String rootDir,Set<String> keepRules)
    {
        if(confirm()){
            Set<String> usedFiles = getFilesTouched(logContents);
            File directoryFile = new File(rootDir);
            deleteFiles(directoryFile,usedFiles,keepRules);
            
            System.out.println("Number of directories deleted:" + deleteDirectoryCount);
            System.out.println("Number of files deleted:" + deleteFileCount);
            System.out.println("The following list contains files expecting to keep but couldn't find: (lock files, cache etc.)");

            for(String file : usedFiles){
                if(!filesKept.contains(file)){
                    System.out.println(file);
                }
            }
            System.out.println("done");
        }else{
            System.out.println("aborted");
        }
    }
    private boolean confirm()
    {
        boolean confirmed = false;
        System.out.println("continue? (y,n)");
        try{
            int val = System.in.read();
            //121 y or 89 Y
            if(val == 121 || val == 89){
                confirmed= true;
            }
        }catch(IOException ioe){}
        return confirmed;
    }
    private boolean alwaysRemove(String filename)
    {
        if(filename.endsWith(".log")){//necessary to remove log files as root is mounted as readonly
            return true;
        }
        if(filename.endsWith(".DS_Store")){//screw you Apple.
            return true;
        }

        return false;
    }
    private boolean alwaysKeep(String filename)
    {
        if(filename.endsWith("init.sh")){
            return true;
        }
        if(filename.contains("fontconfig")){//X.org font cache
            return true;
        }
        if(filename.endsWith(".X0-lock")){//X.org lock file
            return true;
        }
        if(filename.endsWith("lastlog")){//X.org log file
            return true;
        }

        return false;
    }
    private void deleteFile(File directoryFile){
    	deleteFileCount++;
    	directoryFile.delete();
    }
    private void deleteDirectory(File directoryFile){
    	deleteDirectoryCount++;
        directoryFile.delete();
    }
    private boolean inKeepList(File directoryFile, Set<String> keepItems)
    {
        String filename = directoryFile.getName();
        if(keepItems.contains(filename)){
            return true;
        }
        //does the file belong to a directory that has been marked as 'to be kept'
        File parent = directoryFile.getParentFile();
        while(parent != null){
            String name = parent.getName();
            if(keepItems.contains(name)){
                return true;
            }
            parent = parent.getParentFile();
            String path = parent.getPath();
            if(path.indexOf("/root") == -1){
                break;
            }
        }
        //now the other way, see if any regexp match the file
        String fullFilename = directoryFile.getAbsolutePath();
        for(String regexp : keepItems){
            try{
                if(fullFilename.matches(regexp)){
                    return true;
                }
            }catch(Exception e){}//may not be a valid regexp
        }
        return false;
    }
    private void deleteFiles(File directoryFile, Set<String> filesToKeep, Set<String> keepRules){
        try{
            String filename = directoryFile.getPath();
            int index = filename.indexOf("/root");
            filename = filename.substring(index);
            if(directoryFile.isFile()){
                if(alwaysRemove(filename)){
                    deleteFile(directoryFile);
                }else{
                    if(alwaysKeep(filename)){
                        filesKept.add(filename);
                    }else{
                        if(filesToKeep.contains(filename)){
                            filesKept.add(filename);
                        }else if(!inKeepList(directoryFile, keepRules)){
                            deleteFile(directoryFile);
                        }
                    }
                }
            }else{
                filesKept.add(filename);
                File[] files = directoryFile.listFiles();
                for(File file : files){
                    deleteFiles(file, filesToKeep, keepRules); //recurse
                }
                File[] afterFiles = directoryFile.listFiles();        
                if(afterFiles == null || afterFiles.length == 0){
                	deleteDirectory(directoryFile);
                }
            }
        }catch(Exception e){
            e.printStackTrace();
        }
    }


    public Set<String> getFilesTouched(ArrayList<String> contents){
        HashSet<String> files = new HashSet<>();
        boolean validFile = false;
        for(String line : contents){
            if(!validFile && line.endsWith(LOG_START_TOKEN)){
                validFile = true;
            }else{
                int index = line.indexOf(READ_FILE);
                if(index > -1){
                    String file = line.substring(index + READ_FILE.length() + 1);
                    int markerIndex = file.indexOf(FILENAME_END_MARKER);
                    if(markerIndex > 0){
                        file = file.substring(0, markerIndex).trim();
                    }else{
                        file = file.substring(0).trim();                    	
                    }
                    //System.out.println(file);
                    files.add(file);
                }
            }
        }
        if(!validFile){
            throw new Error("Log file is not valid. Truncated perhaps?");
        }
        return files;
    }
    private static Set<String> extractList(String[] extractList){
        Set<String> list = new HashSet<>();
        if(extractList.length>2){
            for(int i=2;i<extractList.length;i++){
                list.add(extractList[i]);
                System.out.println("keeping:"+extractList[i]);

            }
        }
        return list;
    }
    public static void main(String[] args) {
        if(args.length < 2){
            System.out.println("Reduce is used to prune a file system so it only contains files that are needed.");
            System.out.println("Usage: Reduce usagelogfile.txt c:\temp\root bin gamedir tmp");
            System.out.println("Where: 	usagelogfile.txt is the chrome console log (use save as right mouse context menu option)");
            System.out.println("c:\temp\root is the location of the root file system to be pruned");
            System.out.println("bin gamedir is the list of files, dirs, regexp(s) to use to indicate additional files to keep");
            System.exit(-1);
        }
        new Reduce(readLog(args[0]), rootDir(args[1]), extractList(args));
		/*
		String logfile = "log.txt";
		String root = "root";
		Set<String> keepFiles = new HashSet<>();
		//keepFiles.add("bin");
		//keepFiles.add("notepad.exe");
		new Reduce(readLog(logfile), rootDir(root), keepFiles);
		*/
    }
    private static ArrayList<String> readLog(String filename){
        if(filename == null){
            throw new Error("Please specify the filename for the usage log file");
        }
        if(!(filename.startsWith("/") || filename.startsWith("\\"))){
            filename = System.getProperty("user.dir") + "/" + filename;
        }
        File file = checkFileExists(filename);
        System.out.println("Log file:"+filename);
        return readFile(file);
    }
    private static File checkFileExists(String filename){
        File file = new File(filename);
        if(!file.exists()){
            throw new Error("Can't find file:"+filename);
        }
        return file;
    }
    private static File checkDirectoryExists(String filename){
        File file = new File(filename);
        if(!file.exists() || !file.isDirectory()){
            throw new Error("Can't find directory:"+filename);
        }
        return file;
    }
    private static String rootDir(String dir)
    {
        if(dir==null){
            throw new Error("Please specify a root directory for the filesystem.");
        }
        if(!(dir.startsWith("/") || dir.startsWith("\\"))){
            dir = System.getProperty("user.dir") + "/" + dir;
        }
        if(!dir.endsWith("/root")){
            throw new Error("Root directory must end in /root");
        }
        checkDirectoryExists(dir);
        System.out.println("Root directory:"+dir);
        return dir;
    }
    private static ArrayList<String> readFile(File file)
    {
        ArrayList<String> contents = new ArrayList<String>();
        BufferedReader br =  null;
        try{
            br = new BufferedReader(new FileReader(file));
            boolean finished=false;
            while(!finished){
                String line = br.readLine();
                if(line==null){
                    finished=true;
                }else{
                    contents.add(line);
                }
            }
        }catch(Exception e){
            e.printStackTrace();
        }finally{
            if(br!=null){
                try {
                    br.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
        return contents;
    }
}
