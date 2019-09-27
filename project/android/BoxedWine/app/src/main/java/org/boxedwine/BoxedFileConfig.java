package org.boxedwine;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

public class BoxedFileConfig {
    private HashMap<String, String> values;
    private String path;

    BoxedFileConfig(String path) {
        values = new HashMap<>();
        this.path = path;
        try {
            BufferedReader bfr = new BufferedReader(new FileReader(new File(path)));
            String line;
            while ((line = bfr.readLine()) != null) {
                if (!line.startsWith("#") && !line.isEmpty()) {
                    String[] pair = line.trim().split("=");
                    if (pair.length==2) {
                        values.put(pair[0].trim(), pair[1].trim());
                    }
                }
            }
            bfr.close();
        } catch (IOException e) {
        }
    }

    String readString(String key, String defaultValue) {
        String result = values.get(key);
        if (result==null) {
            return defaultValue;
        }
        return result;
    }

    int readInt(String key, int defaultValue) {
        String result = values.get(key);
        if (result==null) {
            return defaultValue;
        }
        return Integer.parseInt(result);
    }

    boolean readBool(String key, boolean defaultValue) {
        String result = values.get(key);
        if (result==null) {
            return defaultValue;
        }
        return Boolean.parseBoolean(result);
    }

    void writeString(String key, String value) {
        if (value==null) {
            value = "";
        }
        values.put(key, value);
    }

    void writeInt(String key, int value) {
        values.put(key, String.valueOf(value));
    }

    void writeBool(String key, boolean value) {
        values.put(key, String.valueOf(value));
    }

    void save() {
        try {
            BufferedWriter bfw = new BufferedWriter(new FileWriter(new File(path)));
            for (Map.Entry mapElement : values.entrySet()) {
                String line = mapElement.getKey() + "=" + mapElement.getValue();
                bfw.write(line);
                bfw.newLine();
            }
            bfw.close();
        } catch (IOException e) {
        }
    }
}
