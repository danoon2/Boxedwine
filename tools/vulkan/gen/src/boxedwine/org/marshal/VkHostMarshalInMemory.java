package boxedwine.org.marshal;

import boxedwine.org.VkFunction;
import boxedwine.org.VkParam;

/**
 * Created by James on 8/22/2021.
 */
public class VkHostMarshalInMemory extends VkHostMarshal {
    // uint32_t* pPhysicalDeviceCount = (uint32_t*)getPhysicalAddress(ARG2, 4);
    public void before(VkFunction fn, StringBuilder out, VkParam param) {
        out.append("    ");
        out.append(param.paramType.name);
        out.append("*");
        if (param.isDoublePointer) {
            out.append("*");
        }
        out.append(" ");
        out.append(param.name);
        out.append(" = (");
        out.append(param.paramType.name);
        out.append("*");
        if (param.isDoublePointer) {
            out.append("*");
        }
        out.append(")getPhysicalAddress(");
        out.append(param.paramArg);
        out.append(", ");
        if (param.arrayLen == 0 && param.countParam != null) {
            out.append("(U32)");
            if (param.countParam.isPointer) {
                out.append("(");
                out.append(param.countParam.name + " * " + param.countParam.sizeof);
                out.append(" ? *");
                out.append(param.countParam.name);
                out.append(" : 0)");
            } else {
                out.append(param.countParam.name + " * " + param.countParam.sizeof);
            }

        } else if (param.arrayLen != 0) {
            out.append(param.arrayLen);
        } else {
            out.append(param.sizeof);
        }
        out.append(");\n");
    }

    public void after(VkFunction fn, StringBuilder out, VkParam param) {

    }
}