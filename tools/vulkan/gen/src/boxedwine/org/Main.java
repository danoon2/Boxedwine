package boxedwine.org;

import boxedwine.org.data.VkData;
import boxedwine.org.data.VkFunction;
import boxedwine.org.data.VkParam;
import boxedwine.org.data.VkType;
import boxedwine.org.marshal.*;
import boxedwine.org.parser.VkParser;
import boxedwine.org.writers.VkC;
import boxedwine.org.writers.VkDefH;
import boxedwine.org.writers.VkFuncsH;
import boxedwine.org.writers.VkHost;
import org.w3c.dom.*;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import java.io.*;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Vector;

public class Main {
    static String hostSource = "source/vulkan/";
    static String fsSource = "tools/vulkan/";

    public static void main(String[] args) {
        try {
            VkData data = new VkData();
            VkParser.parse(data);

            VkDefH.write(data, new String[] {fsSource, hostSource});

            VkFuncsH.write(data, hostSource);
            VkC.write(data, fsSource);

            VkHost.write(data, hostSource);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
