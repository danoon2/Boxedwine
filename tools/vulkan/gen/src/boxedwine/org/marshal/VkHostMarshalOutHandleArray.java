package boxedwine.org.marshal;

import boxedwine.org.data.VkFunction;
import boxedwine.org.data.VkParam;

/**
 * Created by James on 8/22/2021.
 */
public class VkHostMarshalOutHandleArray extends VkHostMarshal {
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
        if (!param.countInStructure && param.countParam.isPointer) {
            param.countString += "*";
        }
        param.countString += param.len;
        out.append(param.countString);
        out.append("];\n    }\n");
    }

    public void after(VkFunction fn, StringBuilder out, VkParam param) throws Exception {
        out.append("    if (");
        out.append(param.paramArg);
        out.append(") {\n");
        out.append("        for (U32 i=0;i<");
        out.append(param.countString);
        out.append(";i++) {\n            cpu->memory->writed(");
        out.append(param.paramArg);
        out.append(" + i*4, createVulkanPtr(cpu->memory, (U64)");
        out.append(param.name);
        out.append("[i], pBoxedInfo));\n        }\n");
        out.append("        delete[] ");
        out.append(param.name);
        out.append(";\n    }\n");
    }
}