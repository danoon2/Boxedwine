package boxedwine.org;

public class VkType {
    VkType() {
    }
    VkType(String name, String type, String category, int sizeof) {
        this.name = name;
        this.type = type;
        this.category = category;
        this.sizeof = sizeof;
    }
    String getEmulatedType() throws Exception {
        if (sizeof == 8) {
            return "U64";
        } else if (sizeof == 0) {
            return "void";
        } else if (sizeof <= 4) {
            return "U32";
        }
        throw new Exception("Unhandled param size: "+sizeof);
    }
    String name;
    String type;
    String category;
    int sizeof;
    boolean returnedonly;
}
