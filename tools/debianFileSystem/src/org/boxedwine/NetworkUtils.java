package org.boxedwine;

import org.apache.commons.io.FileUtils;

import java.io.File;
import java.net.URL;

public class NetworkUtils {
    static boolean download(URL url, File file) {
        try {
            FileUtils.copyURLToFile(url,  file);
            return true;
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }
    }
}
