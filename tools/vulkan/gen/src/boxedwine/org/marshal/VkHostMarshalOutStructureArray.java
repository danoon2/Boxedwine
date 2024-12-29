package boxedwine.org.marshal;

import boxedwine.org.data.VkFunction;
import boxedwine.org.data.VkParam;

/**
 * Created by James on 8/22/2021.
 */
public class VkHostMarshalOutStructureArray extends VkHostMarshal {
    public void before(VkFunction fn, StringBuilder out, VkParam param) throws Exception {
        out.append("    ");
        out.append(param.paramType.name);
        out.append("* ");
        out.append(param.name);
        out.append(" = NULL;\n");
        out.append("    if (");
        out.append(param.paramArg);
        out.append(") {\n        ");
        out.append(param.name);
        out.append(" = new ");
        out.append(param.paramType.name);
        out.append("[");
        param.countString = "";
        if (param.countParam != null) {
            if (!param.countInStructure && param.countParam.isPointer) {
                param.countString += "*";
            }
            param.countString += param.countParam.name;
        } else if (param.len != null) {
            param.countString += param.len;
        } else {
            throw new Exception();
        }
        out.append(param.countString);
        out.append("];\n    }\n");
    }

    public void after(VkFunction fn, StringBuilder out, VkParam param) throws Exception {
        out.append("    if (");
        out.append(param.paramArg);
        out.append(") {\n");
        out.append("        for (U32 i=0;i<");
        out.append(param.countString);
        out.append(";i++) {\n            Marshal");
        out.append(param.paramType.name);
        if (!fn.params.elementAt(0).paramType.getType().equals("VK_DEFINE_HANDLE")) {
            out.append("::write(nullptr, cpu->memory, ");
        } else {
            out.append("::write(pBoxedInfo, cpu->memory, ");
        }
        out.append(param.paramArg);
        out.append(" + i * ");
        out.append(param.paramType.sizeof);
        out.append(", &");
        out.append(param.name);
        out.append("[i]);\n        }\n");
        out.append("        delete[] ");
        out.append(param.name);
        out.append(";\n    }\n");
        param.paramType.needMarshalOut = true;
    }
}