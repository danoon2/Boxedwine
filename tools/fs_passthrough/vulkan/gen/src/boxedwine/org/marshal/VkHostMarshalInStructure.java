package boxedwine.org.marshal;

import boxedwine.org.data.VkCopyData;
import boxedwine.org.data.VkData;
import boxedwine.org.data.VkFunction;
import boxedwine.org.data.VkParam;

/**
 * Created by James on 8/22/2021.
 */
public class VkHostMarshalInStructure extends VkHostMarshal {
    public void before(VkData data, VkFunction fn, StringBuilder out, VkParam param) throws Exception {
        if (param.paramType.name.equals("VkAllocationCallbacks")) {
            out.append("    static bool shown; if (!shown && " + param.paramArg + ") { klog(\""+fn.name+":VkAllocationCallbacks not implemented\"); shown = true;}\n");
            out.append("    ");
            out.append(param.paramType.name);
            out.append("* ");
            out.append(param.name);
            out.append(" = NULL;\n");
        } else {
            VkCopyData copyData = data.copyData.get(fn.name);
            if (copyData != null && copyData.createFunction.equals(fn.name) && copyData.createParamName.equals(param.name)) {
                out.append("    std::shared_ptr<Marshal");
                out.append(param.paramType.name);
                out.append("> local_");
                out.append(param.name);
                out.append(" = std::make_shared<Marshal");
                out.append(param.paramType.name);
                out.append(">(pBoxedInfo, cpu->memory, ");
                out.append(param.paramArg);
                out.append(");\n");
                out.append("    ");
                out.append(param.paramType.name);
                out.append("* ");
                out.append(param.name);
                out.append(" = &local_");
                out.append(param.name);
                out.append("->s;\n");
            } else {
                out.append("    Marshal");
                out.append(param.paramType.name);
                out.append(" local_");
                out.append(param.name);
                out.append("(pBoxedInfo, cpu->memory, ");
                out.append(param.paramArg);
                out.append(");\n");
                out.append("    ");
                out.append(param.paramType.name);
                out.append("* ");
                out.append(param.name);
                out.append(" = &local_");
                out.append(param.name);
                out.append(".s;\n");
            }
            param.paramType.setNeedMarshalIn(data, true);
        }
    }

    public void after(VkData data, VkFunction fn, StringBuilder out, VkParam param) throws Exception {
        VkCopyData copyData = data.copyData.get(fn.name);
        if (copyData != null && copyData.createFunction.equals(fn.name) && copyData.createParamName.equals(param.name)) {
            if (copyData.conditionToStore != null) {
                out.append("    if (");
                out.append(copyData.conditionToStore);
                out.append(") {\n    ");
            }
            String cacheFunction;
            if (copyData.marshalType.equals("MarshalVkDescriptorUpdateTemplateCreateInfo")) cacheFunction = "cacheDescriptorTemplate";
            else if (copyData.marshalType.equals("MarshalVkImageCreateInfo")) cacheFunction = "cacheImageInfo";
            else throw new Exception("Unowned Vulkan cache: " + copyData.marshalType);
            out.append("    " + cacheFunction + "(pBoxedInfo, (U64)" + copyData.createKey + ", local_pCreateInfo->s);\n");
            if (copyData.conditionToStore != null) {
                out.append("    }\n");
            }
        }
    }
}
