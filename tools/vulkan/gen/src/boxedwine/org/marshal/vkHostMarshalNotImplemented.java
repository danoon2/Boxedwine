package boxedwine.org.marshal;

import boxedwine.org.VkFunction;
import boxedwine.org.VkParam;

public class vkHostMarshalNotImplemented extends VkHostMarshal {
    public void before(VkFunction fn, StringBuilder out, VkParam param) throws Exception {
        out.append("    ");
        out.append(param.paramType.name);
        if (param.isPointer) {
            out.append("*");
        }
        out.append(" ");
        out.append(param.name);
        out.append(";\n");

        out.append("    kpanic(\"vkUpdateDescriptorSetWithTemplate not implemented\");\n");
    }

    public void after(VkFunction fn, StringBuilder out, VkParam param) throws Exception {

    }
}
