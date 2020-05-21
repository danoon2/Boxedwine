package org.boxedwine;

import org.apache.commons.io.FilenameUtils;

import java.io.File;
import java.net.URL;

public class FileCache {
    static File getFile(URL url) {
        String fileName = FilenameUtils.getName(url.getPath());
        File result = new File(Settings.fileCachePath.getAbsolutePath() + File.separator + fileName);
        if (result.exists()) {
            return result;
        }
        if (NetworkUtils.download(url, result)) {
            return result;
        }
        return null;
    }
}
