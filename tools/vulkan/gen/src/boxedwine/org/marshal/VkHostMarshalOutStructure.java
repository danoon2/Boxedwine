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
        out.append(";\n");
        param.nameInFunction = "&"+param.name+".s";
        param.paramType.needMarshalOut = true;
    }

    public void after(VkFunction fn, StringBuilder out, VkParam param) throws Exception {
        out.append("    Marshal");
        out.append(param.paramType.name);
        out.append("::write(");
        out.append(param.paramArg);
        out.append(", &");
        out.append(param.name);
        out.append(".s);\n");
    }
}
