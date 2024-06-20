package boxedwine.org;

import java.util.Vector;

public class VkType {
    VkType() {
    }
    VkType(String name, String type, String category, int sizeof) {
        this.name = name;
        this.type = type;
        this.category = category;
        this.sizeof = sizeof;
    }
    public String getEmulatedType() throws Exception {
        if (type.equals("VK_DEFINE_NON_DISPATCHABLE_HANDLE")) {
            return "U64";
        }
        if (type.equals("VK_DEFINE_HANDLE")) {
            return "U32";
        }
        if (parent != null) {
            return parent.getEmulatedType();
        }
        if (sizeof == 8) {
            return "U64";
        } else if (type.equals("void")) {
            return "void";
        } else if (sizeof == 4 || category.equals("enum") || sizeof == 2) {
            return "U32";
        } else {
            throw new Exception("Unknow size");
        }
    }
    public int getEmulatedReadWriteWidth() throws Exception {
        if (type.equals("VK_DEFINE_NON_DISPATCHABLE_HANDLE")) {
            return 8;
        }
        if (type.equals("VK_DEFINE_HANDLE")) {
            return 4;
        }
        if (sizeof == 8) {
            return 8;
        } else if (sizeof == 4 || category.equals("enum")) {
            return 4;
        } else if (sizeof == 2) {
            return 2;
        } else {

            if (parent != null) {
                return parent.getEmulatedReadWriteWidth();
            }
            throw new Exception("Unknow size");
        }
    }
    public boolean hasParent(String parentName) {
        if (name.equals(parentName)) {
            return true;
        }
        if (parent != null) {
            return parent.hasParent(parentName);
        }
        return false;
    }
    public boolean needsMarshaling() {
        if (category.equals("enum")) {
            // size if not guaranteed in C99, could be char, could be int
            return true;
        }
        if (type.equals("size_t")) {
            return true;
        }
        if (type.equals("VK_DEFINE_HANDLE")) {
            return true;
        }
        if (type.equals("VK_DEFINE_NON_DISPATCHABLE_HANDLE")) {
            return false;
        }
        if (sizeof > 0) { // native type
            return false;
        }
        if (members != null) {
            for (VkParam member : members) {
                if (member.needsMarshaling()) {
                    return true;
                }
            }
            return false;
        }
        if (parent != null) {
            return parent.needsMarshaling();
        }
        return true;
    }
    public int getSize() {
        if (category.equals("enum")) {
            return 4;
        }
        int result = sizeof;
        if (members != null) {
            if (type.equals("union")) {
                int max = 0;
                for (VkParam param : members) {
                    max = Math.max(max, param.getSize());
                }
                result += max;
            } else {
                for (VkParam param : members) {
                    result += param.getSize();
                }
            }
        }
        if (type.equals("VK_DEFINE_NON_DISPATCHABLE_HANDLE")) {
            return 8;
        }
        if (type.equals("VK_DEFINE_HANDLE")) {
            return 4;
        }
        if (result == 0 && parent != null) {
            return parent.getSize();
        }
        return result;
    }

    public void copy(VkType from) {
        name = from.name;
        type = from.type;
        category = from.category;
        returnedonly = from.returnedonly;
        parent = from.parent;
        members = from.members;
        needMarshalIn = from.needMarshalIn;
        needMarshalOut = from.needMarshalOut;
        sizeof = from.sizeof;
    }
    public String name;
    public String type;
    public String category;
    public boolean returnedonly;
    public VkType parent;
    public Vector<VkParam> members = new Vector<>();
    public boolean needMarshalIn;
    public boolean needMarshalOut;
    public String values;

    private int sizeof;
}
