package boxedwine.org;

import boxedwine.org.marshal.VkHostMarshal;

import java.util.Vector;

public class VkParam {
    public VkType paramType;
    public String name;
    public String len; // for pointer types, this is how many items are in it
    public int arrayLen;
    public String full;
    public boolean isPointer;
    public boolean isDoublePointer;
    public int sizeof;
    public boolean isConst;
    public VkHostMarshal marshal;

    // used by VkHost
    public String paramArg;
    public VkParam countParam;
    public boolean countInStructure;
    public String nameInFunction;
    public String countString;

    public String getEmulatedType() throws Exception {
        if (isPointer || isDoublePointer) {
            return "U32";
        }
        return paramType.getEmulatedType();
    }

    public boolean needsMarshaling() {
        if (isPointer) {
            return true;
        }
        return paramType.needsMarshaling();
    }

    public int getSize() {
        if (isPointer) {
            return 4;
        }
        if (arrayLen != 0) {
            return arrayLen * paramType.getSize();
        }
        return paramType.getSize();
    }

    public boolean isArray() {
        return (len != null && len.length() > 0) || arrayLen != 0;
    }
}
