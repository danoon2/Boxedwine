package boxedwine.org.marshal;

import boxedwine.org.VkFunction;
import boxedwine.org.VkParam;

/**
 * Created by James on 8/22/2021.
 */
public class VkHostMarshalMapMemory extends VkHostMarshal {
    public void before(VkFunction fn, StringBuilder out, VkParam param) throws Exception {
        out.append("    ");
        out.append(param.paramType.name);
        out.append(" ");
        if (param.isPointer) {
            out.append("*");
        }
        if (param.isDoublePointer) {
            out.append("*");
        }
        out.append(param.name);
        out.append(" = NULL;\n");
        out.append("    kpanic(\"");
        out.append(fn.name);
        out.append(" not implemented\");\n");
    }

    public void after(VkFunction fn, StringBuilder out, VkParam param) throws Exception {

    }
}
