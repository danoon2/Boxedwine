package org.boxedwine;

import android.content.res.AssetManager;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Vector;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;
import java.util.zip.ZipInputStream;

public class BoxedUtils {
    static void deleteRecursive(File fileOrDirectory) {
        if (fileOrDirectory.isDirectory()) {
            for (File child : fileOrDirectory.listFiles()) {
                deleteRecursive(child);
            }
        }
        fileOrDirectory.delete();
    }

    // ext should be lower case
    static void getAllFileRecursivlyThatHaveExt(File dir, String ext, Vector<File> results) {
        if (dir.isDirectory()) {
            for (File child : dir.listFiles()) {
                if (child.isDirectory()) {
                    getAllFileRecursivlyThatHaveExt(child, ext, results);
                } else if (child.getName().toLowerCase().endsWith(ext)) {
                    results.addElement(child);
                }
            }
        }
    }

    static void extraAsset(AssetManager assetManager, File directory, String fileName) {
        InputStream is;
        FileOutputStream os;

        try {
            is = assetManager.open(fileName);
            if (is!=null) {

                if (!directory.exists()) {
                    directory.mkdirs();
                }
                File file = new File(directory + File.separator + fileName);
                os = new FileOutputStream(file, true);

                int count;
                byte[] data = new byte[1024*32];
                while ((count = is.read(data)) != -1) {
                    os.write(data, 0, count);
                }
                os.close();
                is.close();
                if (!file.exists()) {
                    int ii=0;
                }
            }
        } catch (IOException e) {
        }
    }
    static void extractFileFromZip(File zipFile, String zipFilePath, File filePathName) {
        InputStream is = null;
        ZipFile zip;
        try  {
            zip = new ZipFile(zipFile);

            ZipEntry ze = zip.getEntry(zipFilePath);
            if (ze!=null) {
                is = zip.getInputStream(ze);
                File parentDir = filePathName.getParentFile();
                parentDir.mkdirs();
                FileOutputStream os = new FileOutputStream(filePathName);
                byte[] data = new byte[16*1024];
                int count;
                while ((count=is.read(data))!=-1) {
                    os.write(data, 0, count);
                }
                os.close();
                is.close();
            }
            zip.close();
        }  catch(IOException e) {
            e.printStackTrace();
        }
    }
    static byte[] readFileFromZip(File zipFile, String fileName) {
        InputStream is = null;
        ZipFile zip;
        try  {
            zip = new ZipFile(zipFile);

            ZipEntry ze = zip.getEntry(fileName);
            if (ze!=null) {
                is = zip.getInputStream(ze);
                byte[] data = new byte[(int) ze.getSize()];
                is.read(data);
                is.close();
                zip.close();
                return data;
            }
            is.close();
            zip.close();
        }  catch(IOException e) {
            e.printStackTrace();
        }
        return null;
    }
}
