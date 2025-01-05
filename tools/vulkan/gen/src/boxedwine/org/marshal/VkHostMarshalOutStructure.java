package boxedwine.org.marshal;

import boxedwine.org.data.VkData;
import boxedwine.org.data.VkFunction;
import boxedwine.org.data.VkParam;

/**
 * Created by James on 8/22/2021.
 */
public class VkHostMarshalOutStructure extends VkHostMarshal {
    public void before(VkData data, VkFunction fn, StringBuilder out, VkParam param) throws Exception {
        out.append("    Marshal");
        out.append(param.paramType.name);
        out.append(" ");
        out.append(param.name);
        // even if param.paramType.returnedonly, we need to marshal it in just in case pNext has a value
        out.append("(pBoxedInfo, cpu->memory, ");
        out.append(param.paramArg);
        out.append(");\n");
        param.nameInFunction = "&"+param.name+".s";
        param.paramType.needMarshalOut = true;
        param.paramType.setNeedMarshalIn(data, true);
    }

    public void after(VkData data, VkFunction fn, StringBuilder out, VkParam param) throws Exception {
        out.append("    Marshal");
        out.append(param.paramType.name);
        out.append("::write(pBoxedInfo, cpu->memory, ");
        out.append(param.paramArg);
        out.append(", &");
        out.append(param.name);
        out.append(".s);\n");
    }
}
