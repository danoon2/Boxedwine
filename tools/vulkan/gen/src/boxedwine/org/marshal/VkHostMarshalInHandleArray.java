package boxedwine.org.marshal;

import boxedwine.org.data.VkData;
import boxedwine.org.data.VkFunction;
import boxedwine.org.data.VkParam;

/**
 * Created by James on 8/22/2021.
 */
public class VkHostMarshalInHandleArray extends VkHostMarshal {
    public void before(VkData data, VkFunction fn, StringBuilder out, VkParam param) throws Exception {
        if (param.stride != null) {
            throw new Exception();
        }
        out.append("    ");
        out.append(param.paramType.name);
        out.append("* ");
        out.append(param.name);
        out.append(" = new ");
        out.append(param.paramType.name);
        out.append("[");
        param.countString = "";
        if (!param.countInStructure && param.countParam.isPointer) {
            param.countString += "*";
        }
        param.countString += param.countParam.name;
        out.append(param.countString);
        out.append("];\n");
        out.append("    for (U32 i=0;i<");
        out.append(param.countString);
        out.append(";i++) {\n        ");
        out.append(param.name);
        out.append("[i] = (");
        out.append(param.paramType.name);
        out.append(")getVulkanPtr(cpu->memory, cpu->memory->readd(");
        out.append(param.paramArg);
        out.append(" + i*4));\n    }\n");
    }

    public void after(VkData data, VkFunction fn, StringBuilder out, VkParam param) throws Exception {
        out.append("    delete[] ");
        out.append(param.name);
        out.append(";\n");
    }
}
