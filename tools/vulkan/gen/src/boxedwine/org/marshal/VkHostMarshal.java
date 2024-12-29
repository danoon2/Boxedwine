package boxedwine.org.marshal;

import boxedwine.org.data.VkFunction;
import boxedwine.org.data.VkParam;

public abstract class VkHostMarshal {
    public abstract void before(VkFunction fn, StringBuilder out, VkParam param) throws Exception;
    public abstract void after(VkFunction fn, StringBuilder out, VkParam param) throws Exception;
}
