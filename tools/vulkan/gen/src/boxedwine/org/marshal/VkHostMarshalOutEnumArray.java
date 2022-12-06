package boxedwine.org.marshal;

import boxedwine.org.VkFunction;
import boxedwine.org.VkParam;

/**
 * Created by James on 8/22/2021.
 */
public class VkHostMarshalOutEnumArray extends VkHostMarshalInMemory {
    public void before(VkFunction fn, StringBuilder out, VkParam param) throws Exception {
        out.append("    static_assert (sizeof(" + param.paramType.name+ ") == 4, \"unhandled enum size\");\n");
        super.before(fn, out, param);
    }

    public void after(VkFunction fn, StringBuilder out, VkParam param) throws Exception {

    }
}
