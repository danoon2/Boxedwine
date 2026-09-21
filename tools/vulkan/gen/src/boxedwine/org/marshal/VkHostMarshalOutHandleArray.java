package boxedwine.org.marshal;

import boxedwine.org.data.VkData;
import boxedwine.org.data.VkFunction;
import boxedwine.org.data.VkParam;

/**
 * Created by James on 8/22/2021.
 */
public class VkHostMarshalOutHandleArray extends VkHostMarshal {
    public void before(VkData data, VkFunction fn, StringBuilder out, VkParam param) throws Exception {
        out.append("    ");
        out.append(param.paramType.name);
        out.append("* ");
        out.append(param.name);
        out.append(" = NULL;\n");
        param.countString = "";
        if (!param.countInStructure && param.countParam.isPointer) {
            param.countString += "*";
        }
        param.countString += param.len;
        out.append("    const U32 " + param.name + "Capacity = " + param.countString + ";\n");
        out.append("    if (" + param.paramArg + ") {\n        " + param.name + " = new "
                + param.paramType.name + "[" + param.name + "Capacity]();\n    }\n");
    }

    public void after(VkData data, VkFunction fn, StringBuilder out, VkParam param) throws Exception {
        out.append("    if (");
        out.append(param.paramArg);
        out.append(") {\n");
        if (fn.returnType.name.equals("VkResult")) out.append("        if ((VkResult)EAX >= VK_SUCCESS) {\n");
        out.append("        for (U32 i=0;i<std::min((U32)(" + param.countString + "), " + param.name + "Capacity);i++) {\n");
        out.append("            U32 wrapper = createVulkanPtr(cpu->memory, " + param.name + "[i], pBoxedInfo);\n");
        if (fn.name.equals("vkAllocateCommandBuffers"))
            out.append("            registerVulkanCommandBuffer(pBoxedInfo, pAllocateInfo->commandPool, wrapper);\n");
        out.append("            cpu->memory->writed(");
        out.append(param.paramArg);
        out.append(" + i*4, wrapper);\n        }\n");
        if (fn.name.equals("vkAllocateCommandBuffers")) {
            // Unlike ordinary query outputs, allocation failure must clear every slot.
            out.append("        } else {\n            for (U32 i=0;i<" + param.name + "Capacity;++i) cpu->memory->writed(" + param.paramArg + " + i*4, 0);\n");
        }
        if (fn.returnType.name.equals("VkResult")) out.append("        }\n");
        out.append("        delete[] ");
        out.append(param.name);
        out.append(";\n    }\n");
    }
}
