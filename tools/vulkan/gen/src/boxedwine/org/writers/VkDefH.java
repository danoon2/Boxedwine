package boxedwine.org.writers;

import boxedwine.org.data.VkData;
import boxedwine.org.data.VkFunction;

import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStreamReader;
import java.nio.charset.Charset;
import java.util.Hashtable;

// the vulkan library in the emulated file system will call into Boxedwine so that the vulkan call
// can be passed to the host computer.  This is done by using the emulated instruction "int 9a".
// The emulated vulkan library and Boxedwine need to agree on the numerical value that represents
// each function, for example vkCreateInstance is function 1, vkDestroyInstance is function 2, etc
//
// This Java class will write out the vk_def file that maps each vulkan function to a number.  It
// will read in the previous definition in order to maintain backwards compatibility by maintaining
// the same numerical value for each existing function and only give new numbers to new functions.
public class VkDefH {
    static public void write(VkData data, String[] dirs) throws Exception {
        Hashtable<String, String> defs = new Hashtable<>();
        FileInputStream fis = new FileInputStream(dirs[0]+"vkdef.h");
        readDefs(fis, defs);
        fis.close();
        applyDefs(data, defs);
        for (String dir : dirs) {
            writeDefs(data, dir, defs);
        }
    }

    static private void readDefs(FileInputStream fis, Hashtable<String, String> defs) throws Exception {
        InputStreamReader isr = new InputStreamReader(fis, Charset.forName("UTF-8"));
        BufferedReader br = new BufferedReader(isr);
        String line;

        while ((line = br.readLine()) != null) {
            String[] parts = line.split(" ");
            defs.put(parts[1], parts[2]);
        }
    }

    static private void applyDefs(VkData data, Hashtable<String, String> defs) {
        int lastValue = 0;

        String s = defs.get("VK_LAST_VALUE");
        if (s != null) {
            lastValue = Integer.parseInt(s);
        }
        for (VkFunction fn : data.functions ) {
            String key = fn.name.substring(2);
            String def = defs.get(key);
            if (def != null) {
                fn.def = def;
            } else {
                lastValue++;
                def = String.valueOf(lastValue);
                fn.def = def;
                defs.put(key, def);
            }
        }
        defs.put("LAST_VALUE", String.valueOf(lastValue));
    }

    static public void writeDefs(VkData data, String dir, Hashtable<String, String> defs) throws Exception {
        FileOutputStream fosDefs = new FileOutputStream(dir+"vkdef.h");
        StringBuilder out = new StringBuilder();
        for (VkFunction fn : data.functions ) {
            out.append("#define ");
            out.append(fn.name.substring(2));
            out.append(" ");
            if (fn.def == null) {
                fn.def = defs.get(fn.name.substring(2));
            }
            if (fn.def == null) {
                fn.def = String.valueOf(defs.size());
                defs.put(fn.name, fn.def);
            }
            out.append(fn.def);
            out.append("\n");
        }
        out.append("#define VK_LAST_VALUE ");
        out.append(data.functions.size());
        out.append("\n");
        fosDefs.write(out.toString().getBytes());
        fosDefs.close();
    }
}
