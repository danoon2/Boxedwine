package boxedwine.org.marshal;

import boxedwine.org.data.VkData;
import boxedwine.org.data.VkFunction;
import boxedwine.org.data.VkParam;

public class VkUpdateDescriptorSetWithTemplateMarshal extends VkHostMarshal {
    public void before(VkData data, VkFunction fn, StringBuilder out, VkParam param) throws Exception {
        out.append("    std::vector<U8> templateData;\n");
        out.append("    const void* pData = marshalDescriptorTemplateData(pBoxedInfo, cpu->memory, descriptorUpdateTemplate, "
                + param.paramArg + ", templateData);\n");
    }

    public void after(VkData data, VkFunction fn, StringBuilder out, VkParam param) throws Exception {
    }
}
