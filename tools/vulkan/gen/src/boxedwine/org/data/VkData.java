package boxedwine.org.data;

import java.util.Hashtable;
import java.util.Vector;

public class VkData {
    public Vector<VkFunction> functions = new Vector<>();

    // for lookups
    public Hashtable<String, VkType> types = new Hashtable<>();
    // maintain order to prevent compiler errors of output
    public Vector<VkType> orderedTypes = new Vector<>();

    public Hashtable<String, String> constants = new Hashtable<>();

    public Vector<VkExtension> extensions = new Vector<>();
    public Vector<VkFeature> features = new Vector<>();

    public Vector<String> manuallyHandledFunctions = new Vector<>();

    public Vector<String> stubMarshalWrite = new Vector<>();
    public Vector<String> stubMarshalRead = new Vector<>();

    public Vector<String> ignoreStructTypes = new Vector<>();

    public VkFunction getFunctionByName(String name) {
        for (VkFunction fn : functions) {
            if (fn.name.equals(name)) {
                return fn;
            }
        }
        return null;
    }
}
