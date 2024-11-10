package boxedwine.org.marshal;

import boxedwine.org.VkFunction;
import boxedwine.org.VkParam;

/**
 * Created by James on 8/22/2021.
 */
public class VkHostMarshalInHandle extends VkHostMarshal {
    public void before(VkFunction fn, StringBuilder out, VkParam param) throws Exception {
        out.append("    ");
        out.append(param.full);
        out.append(" = ");
        out.append("("+param.paramType.name+")");
        out.append("getVulkanPtr(cpu->memory, ");
        out.append(param.paramArg);
        out.append(")");
        out.append(";\n");
    }

    public void after(VkFunction fn, StringBuilder out, VkParam param) throws Exception {
        if (fn.name.startsWith("vkDestroy") && fn.name.substring(9).toLowerCase().equals(param.name.toLowerCase())) {
            out.append("    freeVulkanPtr(cpu->memory, ");
            out.append(param.paramArg);
            out.append(");\n");
        }
    }
}