package boxedwine.org.data;

import boxedwine.org.marshal.VkHostMarshal;

public class VkParam {
    public VkType paramType;
    public String name;

    // xml attributes
    public String altlen;
    public String len;
    public String optional;
    public boolean noautovalidity;
    public String featurelink;
    public String limittype;
    public String selection;
    public String externsync;
    public String selector;
    public String objecttype;
    public String api;
    public String deprecated;
    public String stride;
    public String validstructs;

    public String full;
    public int offset; // offset in structure containing this member

    public String secondArrayLen; // for double arrays, this is the length of the inner arrays
    public int arrayLen;
    public boolean isPointer;
    public boolean isDoublePointer;
    public int sizeof;
    public boolean isConst;

    // used by VkHost
    public String paramArg;
    public VkParam countParam;
    public boolean countInStructure;
    public String nameInFunction;
    public String countString;

    public boolean postProcessed; // used to prevent recursion

    public String getEmulatedType() throws Exception {
        if (isPointer || isDoublePointer) {
            return "U32";
        }
        return paramType.getEmulatedType();
    }

    public int getSize() {
        if (isPointer) {
            return 4;
        }
        if (arrayLen != 0) {
            return arrayLen * paramType.sizeof;
        }
        if (paramType.name.equals("void") && sizeof != 0) {
            return sizeof;
        }
        return paramType.sizeof;
    }

    public int getAlignment() {
        if (isPointer) {
            return 4;
        }
        if (paramType.name.equals("void") && sizeof != 0) {
            return sizeof;
        }
        return paramType.getAlignment();
    }

    public boolean isArray() {
        return (len != null && len.length() > 0) || arrayLen != 0;
    }
}
