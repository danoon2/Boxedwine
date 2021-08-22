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
        if (sizeof == 8) {
            return "U64";
        } else if (sizeof == 0) {
            return "void";
        } else if (sizeof <= 4) {
            return "U32";
        }
        throw new Exception("Unhandled param size: "+sizeof);
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
            for (VkParam param : members) {
                result += param.getSize();
            }
        }
        return result;
    }

    public String name;
    public String type;
    public String category;
    public boolean returnedonly;
    public VkType parent;
    public Vector<VkParam> members;

    private int sizeof;
}
