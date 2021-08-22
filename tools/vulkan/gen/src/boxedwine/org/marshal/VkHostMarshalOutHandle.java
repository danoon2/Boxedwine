package boxedwine.org.marshal;

import boxedwine.org.VkFunction;
import boxedwine.org.VkParam;

public class VkHostMarshalOutHandle extends VkHostMarshal {
    // VkInstance pInstance;
    public void before(VkFunction fn, StringBuilder out, VkParam param) {
        out.append("    ");
        out.append(param.paramType.name);
        out.append(" ");
        out.append(param.name);
        out.append(";\n");
        param.nameInFunction = "&"+param.name;
    }

    public void after(VkFunction fn, StringBuilder out, VkParam param) {

    }
}
