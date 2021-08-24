package boxedwine.org.marshal;

import boxedwine.org.VkFunction;
import boxedwine.org.VkParam;

/**
 * Created by James on 8/22/2021.
 */
public class VkHostMarshalInMemory extends VkHostMarshal {
    // uint32_t* pPhysicalDeviceCount = (uint32_t*)getPhysicalAddress(ARG2, 4);
    public void before(VkFunction fn, StringBuilder out, VkParam param) throws Exception {
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
            if (param.countParam.isPointer && !param.countParam.isConst) {
                out.append("(");
                out.append(param.countParam.name);
                out.append(" ? ");
                if (!param.countInStructure) {
                    out.append("*");
                }
                out.append(param.len);
                out.append(" : 0)");
            } else {
                out.append(param.len + " * " + param.countParam.getSize());
            }

        } else if (param.arrayLen != 0) {
            out.append(param.arrayLen * param.paramType.getSize());
        } else {
            out.append(param.getSize());
        }
        out.append(");\n");
    }

    public void after(VkFunction fn, StringBuilder out, VkParam param) throws Exception {

    }
}