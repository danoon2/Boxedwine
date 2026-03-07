/*
 *  Copyright (C) 2012-2025  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
package boxedwine.org;

import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Vector;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

import static java.nio.file.StandardCopyOption.REPLACE_EXISTING;

public class Main {
    static String wineZip;
    static String scriptDir;
    static String boxedWineExe = "boxedwine";
    static Vector<String> extraCommands = new Vector<>();
    static boolean verbose = false;
    static String perfName = "Performance";
    static boolean atleastOneFailed = false;

    static class Results {
        int exitCode;
        boolean scriptFinished;
        String commandLine;
        int timeToComplete;
        Vector<String> output;
    }
    static public void copyFolder(Path src, Path dest) throws IOException {
        Files.walk(src).forEach(source -> copy(source, dest.resolve(src.relativize(source))));
    }

    static private void copy(Path source, Path dest) {
        try {
            Files.copy(source, dest, REPLACE_EXISTING);
        } catch (Exception e) {
            throw new RuntimeException(e.getMessage(), e);
        }
    }

    public static void runBoxedWine(String name, String directory, String scriptDir, String[] parts, Results results) throws IOException {
        ProcessBuilder builder = new ProcessBuilder();
        List<String> commands = new ArrayList<>();
        commands.add(boxedWineExe);
        commands.add("-automation");
        commands.add(directory+File.separator+scriptDir);
        commands.add("-root");
        commands.add(directory+File.separator+"root");
        commands.add("-zip");
        commands.add(wineZip);
        commands.add("-w");
        commands.add(parts[0]);
        int lastIndex = -1;
        for (int i=2;i<parts.length;i++) {
            if (parts[i].equals("ARGS")) {
                lastIndex = i + 1;
                break;
            }
            commands.add(parts[i]);
        }
        for (int i=0;i<extraCommands.size();i++) {
            commands.add(extraCommands.elementAt(i));
        }
        commands.add("/bin/wine");
        commands.add(parts[1]);
        if (lastIndex > 1) {
            for (int i = lastIndex; i < parts.length; i++) {
                commands.add(parts[i]);
            }
        }
        builder.command(commands);
        results.commandLine = "boxedwine -automation "+directory+File.separator+scriptDir+" -root "+directory+File.separator+"root -zip \""+wineZip+"\" -w \""+parts[0]+"\" /bin/wine \""+parts[1]+"\"";
        if (lastIndex > 1) {
            for (int i = lastIndex; i < parts.length; i++) {
                results.commandLine += " " + parts[i];
            }
        }
        for (int i=0;i<extraCommands.size();i++) {
            results.commandLine+=" "+extraCommands.elementAt(i);
        }
        if (verbose) {
            System.out.println(results.commandLine);
        }
        builder.directory(new File(directory));
        Process process = builder.start();
        StreamGobbler streamGobbler = new StreamGobbler(process.getInputStream(), name);
        streamGobbler.run();
        try {
            if (!process.waitFor(30, TimeUnit.MINUTES)) {
                System.out.println("Killing "+parts[1]);
                process.destroyForcibly();
                results.exitCode = 1; // 111 is the valid return code
            } else {
                results.exitCode = process.exitValue();
            }
            results.scriptFinished = streamGobbler.scriptFinished;
            results.output = streamGobbler.lines;
        } catch (InterruptedException e) {
            throw new IOException("Failed to run "+parts[1]);
        }
    }

    public static void deleteDir(File file) throws IOException {
        if (file.isDirectory()) {
            File[] entries = file.listFiles();
            if (entries != null) {
                for (File entry : entries) {
                    deleteDir(entry);
                }
            }
        }
        if (!file.delete()) {
            file.deleteOnExit();
        }
    }

    public static String[] getLines(String path) {
        try {
            List lines = Files.readAllLines(Paths.get(path));
            String[] result = new String[lines.size()];
            for (int i=0;i<lines.size();i++) {
                result[i] = (String)lines.get(i);
            }
            return result;
        } catch (IOException e) {

        }
        return new String[0];
    }

    public static void runTest(String name, String filesPath, String path, Results results) throws IOException {
        File rootPath = new File(path+File.separator+"root");
        if (rootPath.exists()) {
            deleteDir(rootPath);
        }
        rootPath.mkdir();
        File copiedFilesPath = new File(path+File.separator+"root"+File.separator+"files");
        copyFolder(new File(filesPath).toPath(), copiedFilesPath.toPath());

        String[] installLines = getLines(path + File.separator+"Install.txt");
        String[] installLines2 = getLines(path + File.separator+"Install2.txt");
        String[] playLines = getLines(path + File.separator+"Play.txt");

        long startTime = System.currentTimeMillis();
        if (installLines.length>0) {
            if (installLines.length<2) {
                throw new IOException(path+File.separator+"Install.txt needs to have 2 lines");
            }
            runBoxedWine(name, path, "install", installLines, results);
            if (results.exitCode!=111) {
                return;
            }
        }
        if (installLines2.length>0) {
            if (installLines2.length<2) {
                throw new IOException(path+File.separator+"Install2.txt needs to have 2 lines");
            }
            runBoxedWine(name, path, "install2", installLines2, results);
            if (results.exitCode!=111) {
                return;
            }
        }
        String parseFile = null;
        String parseKey = null;

        if (playLines.length>0) {
            if (playLines.length<2) {
                throw new IOException(path+File.separator+"Play.txt needs to have 2 lines");
            }
            if (playLines[0].startsWith("PARSE_FILE=")) {
                parseFile = playLines[0].substring(11);
                String[] tmp = new String[playLines.length - 1];
                for (int i=0;i<playLines.length - 1;i++) {
                    tmp[i] = playLines[1+i];
                }
                playLines = tmp;
            }
            if (playLines[0].startsWith("PARSE_KEY=")) {
                parseKey = playLines[0].substring(10);
                String[] tmp = new String[playLines.length - 1];
                for (int i=0;i<playLines.length - 1;i++) {
                    tmp[i] = playLines[1+i];
                }
                playLines = tmp;
            }
            runBoxedWine(name, path, "play", playLines, results);
        }
        results.timeToComplete = (int)((System.currentTimeMillis()-startTime)/1000);
        if (results.exitCode==111) {
            if (parseFile != null && parseKey != null) {
                try {
                    File file = new File(path+File.separator+"root"+parseFile);
                    System.out.println("Looking in " + file.getPath());

                    String[] parseLines = null;

                    if (file.isFile() && file.exists()) {
                        System.out.println("Using " + file.getPath());
                        parseLines = getLines(file.getAbsolutePath());
                    }
                    if (file.isDirectory() && file.exists()) {
                        System.out.println("Searching directory");
                        for (File sub : file.listFiles()) {
                            if (sub.isFile()) {
                                System.out.println("Using " + sub.getPath());
                                parseLines = getLines(sub.getAbsolutePath());
                                break;
                            }
                        }
                    }

                    if (parseFile == null) {
                        System.out.println("Did not find any lines");
                        results.exitCode = 1;
                    } else {
                        boolean foundKey = false;
                        for (String line : parseLines) {
                            if (line.startsWith(parseKey)) {
                                System.out.println("Found key: "+line);
                                foundKey = true;
                                String value = line.substring(parseKey.length());
                                String originalValue = value;
                                if (value.contains(".")) {
                                     double d = Double.parseDouble(value) * 10000;
                                     int i=(int)d;
                                     value = String.valueOf(i);
                                }
                                String header = "platform,value";
                                String platform = "Platform";
                                String testName = perfName;

                                if (testName.contains("-")) {
                                    platform = testName.substring(testName.indexOf('-')+1);
                                    testName = testName.substring(0, testName.indexOf('-'));
                                }
                                String json = "{\n" +
                                        "    \"name\": \""+perfName+"\",\n" +
                                        "    \"value\": "+originalValue+"\n}";
                                Files.write(Paths.get(path + File.separator + "perf-" + perfName + ".json"), json.getBytes());
                                String csv = perfName + "\n" + originalValue;
                                Path csvPath = Paths.get(path + File.separator + "perf-" + perfName + ".csv");
                                Files.write(csvPath, csv.getBytes());
                                System.out.println("Wrote " + csvPath.toString());
                            }
                        }
                        if (!foundKey) {
                            System.out.println("Did not find key");
                            results.exitCode = 1;
                        }
                    }
                } catch (Exception e) {
                    results.exitCode = 1;
                    System.out.println(e.getMessage());
                }
            }
            deleteDir(rootPath);
        }
    }

    public static void runTest(String name) {
        try {
            Results results = null;
            for (int i=0;i<3;i++) {
                results = new Results();
                runTest(name, scriptDir + File.separator + name + File.separator + "files", scriptDir + name, results);
                if (results.exitCode == 111) {
                    System.out.println("OK     " + name + " completed in " + results.timeToComplete + " seconds");
                    return;
                }
                System.out.println("RETRY  " + name);
            }
            System.out.println("FAILED " + name + ".  Error Code " + results.exitCode);
            atleastOneFailed = true;
            if (results.scriptFinished) {
                System.out.println("    Script succeeded but Boxedwine did not exit cleanly");
            }
            System.out.println(results.commandLine);
            for (String line : results.output) {
                System.out.println("    " + line);
            }
        } catch (IOException e) {
            e.printStackTrace();
            System.exit(4);
        }
    }

    public static void main(String[] args) {
        for (int i=0;i<args.length;i++) {
            System.out.println(args[i]);
        }
        if (args.length<2) {
            System.out.print("You must pass in <zip dir> and <script dir>");
            System.exit(3);
            return;
        }
        int index=0;
        for (;index<args.length;index++) {
            if (args[index].equals("-v")) {
                verbose = true;
                continue;
            } else if (args[index].equals("-name")) {
                perfName = args[index + 1];
                index++;
                continue;
            } else {
                break;
            }
        }
        wineZip = args[index++];
        scriptDir = args[index++];
        if (!scriptDir.endsWith("\\") && !scriptDir.endsWith("/")) {
            scriptDir+=File.separator;
        }
        if (args.length>1) {
            boxedWineExe = args[index++];
        }
        if (verbose) {
            System.out.println("zipDir = "+wineZip);
            System.out.println("scriptDir = "+scriptDir);
            System.out.println("boxedWineExe = "+boxedWineExe);
        }
        if (args.length>2) {
            for (;index<args.length;) {
                extraCommands.addElement(args[index++]);
            }
        }
        File dir = new File(scriptDir);
        File[] scripts = dir.listFiles();
        if (scripts==null) {
            System.out.println("Did not find any script folder in "+scriptDir);
            System.exit(2);
            return;
        }
        Arrays.sort(scripts);
        long startTime = System.currentTimeMillis();
        int cores = Runtime.getRuntime().availableProcessors();
        if (cores > 4) {
            cores = cores / 2 + 1;
        } else if (cores == 4) {
            cores = 2;
        }
        System.out.println("Starting automation: using " + cores + " cores.");
        ExecutorService exec = Executors.newFixedThreadPool(cores);
        for (File f : scripts) {
            if (f.isDirectory()) {
                exec.submit(() -> {
                    runTest(f.getName());
                });
            }
        }

        exec.shutdown();
        try {
            if (!exec.awaitTermination(60, TimeUnit.MINUTES)) {
                exec.shutdownNow();
            }
        } catch (InterruptedException ex) {
            exec.shutdownNow();
        }

        int elapsedTime = (int)((System.currentTimeMillis()-startTime)/1000);
        int minutes = elapsedTime / 60;
        int seconds = elapsedTime - minutes*60;
        System.out.println("Total Time: "+minutes+"m "+seconds+"s");
        if (atleastOneFailed) {
            System.out.println("1 or more scripts failed:  Exiting with code 1");
            System.exit(1);
        }
    }
}
