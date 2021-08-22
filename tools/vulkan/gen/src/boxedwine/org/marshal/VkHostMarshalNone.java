package boxedwine.org.marshal;

import boxedwine.org.VkFunction;
import boxedwine.org.VkParam;

/**
 * Created by James on 8/22/2021.
 */
public class VkHostMarshalNone extends VkHostMarshal {
    public void before(VkFunction fn, StringBuilder out, VkParam param) {
        out.append("    ");
        out.append(param.full);
        out.append(" = ");
        if (param.paramType != null) {
            out.append("("+param.paramType.name+")");
        }
        out.append(param.paramArg);
        out.append(";\n");
    }

    public void after(VkFunction fn, StringBuilder out, VkParam param) {

    }
}
