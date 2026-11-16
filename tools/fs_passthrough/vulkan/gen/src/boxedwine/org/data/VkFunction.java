package boxedwine.org.data;

import java.util.Vector;

public class VkFunction {
    public VkType returnType;
    public String name;
    public Vector<VkParam> params = new Vector<VkParam>();
    public String def; // define value in vk_def.h
    public String extension;

    // used to make a copy for alias functions
    public VkFunction clone() {
        VkFunction fn = new VkFunction();
        fn.returnType = returnType;
        fn.name = name;
        fn.def = def;
        fn.extension = extension;
        fn.params = params; // ok to not deep clone this
        return fn;
    }
}
