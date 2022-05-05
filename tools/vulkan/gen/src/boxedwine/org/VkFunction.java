package boxedwine.org;

import java.util.Vector;

public class VkFunction {
    public VkType returnType;
    public String name;
    public Vector<VkParam> params = new Vector<VkParam>();
    public String def;
    public String extension;

    public VkFunction clone() {
        VkFunction fn = new VkFunction();
        fn.returnType = returnType;
        fn.name = name;
        fn.def = def;
        fn.extension = extension;
        fn.params = params; // ok to not deep clone this
        return fn;
    }

    public void getCountParam(VkParam param) {
        if (param.len != null) {
            for (VkParam p : params) {
                if (p.name.equals(param.len)) {
                    param.countParam = p;
                    return;
                }
                if (param.len.startsWith(p.name + "->")) {
                    param.countParam = p;
                    param.countInStructure = true;
                    return;
                }
            }
        }
    }
}
