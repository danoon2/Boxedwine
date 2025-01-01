package boxedwine.org.data;

import java.util.Vector;

public class VkType {
    public VkType() {
    }

    public VkType(String name, String type, String category, int size) {
        this.name = name;
        this.setType(type);
        this.setCategory(category);
        this.sizeof = size;
    }

    public String getEmulatedType() throws Exception {
        if (name.equals("void")) {
            return "void";
        }
        if (sizeof == 8) {
            return "U64";
        }
        return "U32";
    }
    public int getEmulatedReadWriteWidth() throws Exception {
        if (getType().equals("VK_DEFINE_NON_DISPATCHABLE_HANDLE")) {
            return 8;
        }
        if (getType().equals("VK_DEFINE_HANDLE")) {
            return 4;
        }
        if (sizeof == 8) {
            return 8;
        } else if (sizeof == 4 || getCategory().equals("enum")) {
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

    public int getAlignment() {
        if (sizeof > 0) {
            return Math.min(sizeof, 4);
        }
        if (members != null && members.size() > 0) {
            int maxAlignment = 0;

            for (VkParam param : members) {
                int alignment = param.getAlignment();
                if (alignment > maxAlignment) {
                    maxAlignment = alignment;
                }
            }
            return maxAlignment;
        }
        if (getType().equals("VkFlags") || getType().equals("uint32_t") || getType().equals("VK_DEFINE_HANDLE")) {
            sizeof = 4;
            return sizeof;
        }
        if (getType().equals("uint64_t") || getType().equals("VK_DEFINE_NON_DISPATCHABLE_HANDLE") || getType().equals("VkFlags64")) {
            return 4; // win32 the alignment is 8, but 32-bit linux it is 4
        }
        System.out.println("VkParam.getAlignment failed");
        System.exit(1);
        return 0;
    }

    // xml data
    public String name;
    private String type = "";
    private String category = "";
    public String objtypeenum;
    public String requires;
    public VkType parent;
    public boolean deprecated;
    public boolean returnedonly;
    public String api;
    public String comment;
    public String bitvalues;
    public String[] structextends;
    public boolean allowduplicate;

    public Vector<VkParam> members = new Vector<>();
    private boolean needMarshalIn;
    public boolean needMarshalOut;
    public boolean needDestructor;
    public String structType;

    public int sizeof;
    public int alignment;

    public String getCategory() {
        return category;
    }

    public void setCategory(String category) {
        if (category == null) {
            this.category = "";
        } else {
            this.category = category;
        }
    }

    public String getType() {
        return type;
    }

    public void setType(String type) {
        if (type == null) {
            this.type = "";
        } else {
            this.type = type;
        }
    }

    public boolean isNeedMarshalIn() {
        return needMarshalIn;
    }

    public void setNeedMarshalIn(VkData data, boolean needMarshalIn) {
        if (!data.types.containsKey(this.name)) {
            data.types.put(this.name, this);
            data.orderedTypes.add(this);
        }
        this.needMarshalIn = needMarshalIn;
    }
}
