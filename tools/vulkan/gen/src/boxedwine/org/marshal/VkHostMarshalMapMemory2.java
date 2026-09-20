package boxedwine.org.marshal;

import boxedwine.org.data.VkData;
import boxedwine.org.data.VkFunction;
import boxedwine.org.data.VkParam;

public class VkHostMarshalMapMemory2 extends VkHostMarshal {
    public void before(VkData data, VkFunction fn, StringBuilder out, VkParam param) throws Exception {
        out.append("    ");
        out.append(param.paramType.name);
        out.append(" ");
        if (param.isPointer) {
            out.append("*");
        }
        if (param.isDoublePointer) {
            param.name = "pData";
            param.nameInFunction = "&pData";
        }
        out.append(param.name);
        out.append(" = NULL;\n");
    }

    public void after(VkData data, VkFunction fn, StringBuilder out, VkParam param) throws Exception {
        out.append("    if (EAX == VK_SUCCESS) {\n        U32 address = mapVkMemory(pBoxedInfo, pMemoryMapInfo->memory, pData, pMemoryMapInfo->offset, pMemoryMapInfo->size);\n");
        out.append("        if (address) cpu->memory->writed(" + param.paramArg + ", address);\n");
        out.append("        else { pBoxedInfo->pvkUnmapMemory(device, pMemoryMapInfo->memory); EAX = VK_ERROR_MEMORY_MAP_FAILED; }\n    }\n");
    }
}
