package boxedwine.org.marshal;

import boxedwine.org.data.VkData;
import boxedwine.org.data.VkFunction;
import boxedwine.org.data.VkParam;

/**
 * Created by James on 8/22/2021.
 */
public class VkHostMarshalOutStructureArray extends VkHostMarshal {
    public void before(VkData data, VkFunction fn, StringBuilder out, VkParam param) throws Exception {
        out.append("    ");
        out.append(param.paramType.name);
        out.append("* ");
        out.append(param.name);
        out.append(" = NULL;\n");
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
        out.append("    const U32 " + param.name + "Capacity = " + param.countString + ";\n");
        out.append("    if (" + param.paramArg + ") {\n        " + param.name + " = new "
                + param.paramType.name + "[" + param.name + "Capacity]();\n");

        // vkGetPhysicalDeviceFragmentShadingRatesKHR requires that pFragmentShadingRates has sType set correctly
        out.append("        U32 address = ");
        out.append(param.paramArg);
        out.append(";\n");
        out.append("        for (U32 i=0;i<");
        out.append(param.name + "Capacity");
        out.append(";i++) {\n");
        out.append("            Marshal");
        out.append(param.paramType.name);
        out.append("::read(");
        if (fn.name.equals("vkEnumerateInstanceLayerProperties")) {
            out.append("nullptr");
        } else {
            out.append("pBoxedInfo");
        }
        out.append(", cpu->memory, address + i*");
        out.append(param.paramType.sizeof);
        out.append(", &");
        out.append(param.name);
        out.append("[i]);\n        }\n    }\n");
        param.paramType.setNeedMarshalIn(data, true);
    }

    public void after(VkData data, VkFunction fn, StringBuilder out, VkParam param) throws Exception {
        out.append("    if (");
        out.append(param.paramArg);
        out.append(") {\n");
        if (fn.returnType.name.equals("VkResult")) out.append("        if ((VkResult)EAX >= VK_SUCCESS) {\n");
        out.append("        for (U32 i=0;i<std::min((U32)(");
        out.append(param.countString + "), " + param.name + "Capacity)");
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
        if (fn.returnType.name.equals("VkResult")) out.append("        }\n");
        out.append("        for (U32 i=0;i<" + param.name + "Capacity;++i) {\n");
        out.append("            Marshal" + param.paramType.name + " owned; owned.s = " + param.name + "[i];\n        }\n");
        out.append("        delete[] ");
        out.append(param.name);
        out.append(";\n    }\n");
        param.paramType.needMarshalOut = true;
    }
}
