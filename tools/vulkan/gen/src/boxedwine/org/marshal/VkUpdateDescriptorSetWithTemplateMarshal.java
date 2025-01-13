package boxedwine.org.marshal;

import boxedwine.org.data.VkData;
import boxedwine.org.data.VkFunction;
import boxedwine.org.data.VkParam;

public class VkUpdateDescriptorSetWithTemplateMarshal extends VkHostMarshal {
    public void before(VkData data, VkFunction fn, StringBuilder out, VkParam param) throws Exception {
        out.append("    U32 dataSize = calculateUpdateDescriptorSetWithTemplateDataSize(pBoxedInfo, descriptorUpdateTemplate);\n");
        out.append("    const void* pData = cpu->memory->lockReadOnlyMemory(ARG4, dataSize);\n");
    }

    public void after(VkData data, VkFunction fn, StringBuilder out, VkParam param) throws Exception {
        out.append("    cpu->memory->unlockMemory((U8*)pData);\n");
    }
}
