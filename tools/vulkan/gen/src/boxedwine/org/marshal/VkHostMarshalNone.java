package boxedwine.org.marshal;

import boxedwine.org.VkFunction;
import boxedwine.org.VkParam;

/**
 * Created by James on 8/22/2021.
 */
public class VkHostMarshalNone extends VkHostMarshal {
    public void before(VkFunction fn, StringBuilder out, VkParam param) throws Exception {
        out.append("    ");
        out.append(param.full);
        out.append(" = ");
        if (param.getSize() <= 4) {
            if (param.paramType != null) {
                out.append("(" + param.paramType.name + ")");
            }
            out.append(param.paramArg);
        } else {
            out.append("(" + param.paramType.name + ")ARG64(");
            out.append(param.paramArg);
            out.append(", ");
            out.append(param.paramArg2);
            out.append(")");
        }
        out.append(";\n");
    }

    public void after(VkFunction fn, StringBuilder out, VkParam param) throws Exception {
        if (fn.name.equals("vkFreeMemory") && param.name.equals("memory")) {
            out.append("    unregisterVkMemoryAllocation(memory);\n");
        } else if (fn.name.equals("vkUnmapMemory") && param.name.equals("memory")) {
            out.append("    unmapVkMemory(memory);\n");
        }
    }
}
