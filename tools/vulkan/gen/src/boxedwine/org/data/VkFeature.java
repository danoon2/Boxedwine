package boxedwine.org.data;

import java.util.Vector;

public class VkFeature {
    public String name;
    public String api;
    public String number;
    public String depends;
    public String comment;

    public Vector<String> functions = new Vector<>();
    public Vector<String> extendsStructures = new Vector<>();
    public Vector<String> types = new Vector<>();

}
