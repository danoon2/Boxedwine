package boxedwine.org.marshal;

import boxedwine.org.data.VkData;
import boxedwine.org.data.VkFunction;
import boxedwine.org.data.VkParam;

public class vkHostMarshalNotImplemented extends VkHostMarshal {
    public void before(VkData data, VkFunction fn, StringBuilder out, VkParam param) throws Exception {
        out.append("    ");
        out.append(param.paramType.name);
        if (param.isPointer) {
            out.append("*");
        }
        if (param.isDoublePointer) {
            out.append("*");
        }
        out.append(" ");
        out.append(param.name);
        out.append(";\n");

        out.append("    kpanic(\""+fn.name+" not implemented\");\n");
        System.out.println(fn.name + " incomplete");
        if (fn.name.equals("vkMapMemory2KHR")) {
            int ii=0;
        }
    }

    public void after(VkData data, VkFunction fn, StringBuilder out, VkParam param) throws Exception {

    }
}
