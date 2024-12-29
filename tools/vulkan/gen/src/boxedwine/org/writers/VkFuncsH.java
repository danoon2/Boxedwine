package boxedwine.org.writers;

import boxedwine.org.data.VkData;
import boxedwine.org.data.VkFunction;

import java.io.FileOutputStream;

public class VkFuncsH {
    public static void write(VkData data, String dir) throws Exception {
        FileOutputStream fosFuncs = new FileOutputStream(dir+"vkfuncs.h");
        StringBuilder out = new StringBuilder();
        for (VkFunction fn : data.functions ) {
            if (data.manuallyHandledFunctions.contains(fn.name)) {
                continue;
            }
            if (fn.params.size() == 0 || fn.params.elementAt(0).paramType == null || fn.params.elementAt(0).paramType.getType() == null) {
                int ii=0;
            }
            if (fn.params.elementAt(0).paramType.getType().equals("VK_DEFINE_HANDLE")) {
                out.append("VKFUNC_INSTANCE(");
            } else {
                out.append("VKFUNC(");
            }
            out.append(fn.name.substring(2));
            out.append(")\n");
        }
        fosFuncs.write(out.toString().getBytes());
        fosFuncs.close();
    }
}
