package boxedwine.org.marshal;

import boxedwine.org.data.VkData;
import boxedwine.org.data.VkFunction;
import boxedwine.org.data.VkParam;

public class VkHostMarshalOutHandle extends VkHostMarshal {
    // VkInstance pInstance;
    public void before(VkData data, VkFunction fn, StringBuilder out, VkParam param) throws Exception {
        out.append("    ");
        out.append(param.paramType.name);
        out.append(" ");
        out.append(param.name);
        out.append(" = VK_NULL_HANDLE;\n");
        param.nameInFunction = (param.isPointer?"&":"")+param.name;
    }

    public void after(VkData data, VkFunction fn, StringBuilder out, VkParam param) throws Exception {
        if (fn.returnType.name.equals("VkResult")) out.append("    if (EAX == VK_SUCCESS) {\n");
        out.append("    cpu->memory->writed(");
        out.append(param.paramArg);
        out.append(", createVulkanPtr(cpu->memory, ");
        out.append(param.name);
        out.append(", ");
        if (fn.name.equals("vkCreateInstance")) {
            out.append("NULL");
        } else {
            out.append("pBoxedInfo");
        }
        out.append("));\n");
        if (fn.returnType.name.equals("VkResult")) out.append("    }\n");
    }
}
