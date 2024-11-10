package boxedwine.org.marshal;

import boxedwine.org.VkFunction;
import boxedwine.org.VkParam;

/**
 * Created by James on 8/22/2021.
 */
public class VkHostMarshalOutStructure extends VkHostMarshal {
    public void before(VkFunction fn, StringBuilder out, VkParam param) throws Exception {
        out.append("    Marshal");
        out.append(param.paramType.name);
        out.append(" ");
        out.append(param.name);
        // even if param.paramType.returnedonly, we need to marshal it in just in case pNext has a value
        out.append("(cpu->memory, ");
        out.append(param.paramArg);
        out.append(");\n");
        param.nameInFunction = "&"+param.name+".s";
        param.paramType.needMarshalOut = true;
        param.paramType.needMarshalIn = true;
    }

    public void after(VkFunction fn, StringBuilder out, VkParam param) throws Exception {
        out.append("    Marshal");
        out.append(param.paramType.name);
        out.append("::write(cpu->memory, ");
        out.append(param.paramArg);
        out.append(", &");
        out.append(param.name);
        out.append(".s);\n");
    }
}
