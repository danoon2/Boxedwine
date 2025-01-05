package boxedwine.org.marshal;

import boxedwine.org.data.VkData;
import boxedwine.org.data.VkFunction;
import boxedwine.org.data.VkParam;

/**
 * Created by James on 8/22/2021.
 */
public class VkHostMarshalInEnumArray extends VkHostMarshalInMemory {
    public void before(VkData data, VkFunction fn, StringBuilder out, VkParam param) throws Exception {
        out.append("    static_assert (sizeof(" + param.paramType.name+ ") == 4, \"unhandled enum size\");\n");
        super.before(data, fn, out, param);
    }

    public void after(VkData data, VkFunction fn, StringBuilder out, VkParam param) throws Exception {

    }
}