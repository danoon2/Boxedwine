package boxedwine.org.data;

import java.util.Vector;

public class VkExtension {
    static public class VkExtensionRequire {
        public Vector<VkType> types = new Vector<>();
        public Vector<VkFunction> functions = new Vector<>();
        public String depends;
        public String api;
        public String comment;
        public Vector<String> extendsStructures = new Vector<>();
    }
    public Vector<VkExtensionRequire> require = new Vector<>();

    public String name;
    public int number;
    public String type;
    public String author;
    public String depends;
    public String platform;
    public String contact;
    public String supported;
    public String ratified;
    public boolean nofeatures;
    public String promotedto;
    public String comment;
    public String deprecatedby;
    public String specialuse;
    public String obsoletedby;
    public boolean provisional;
    public String sortorder;
}
