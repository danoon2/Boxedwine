package boxedwine.org;

public class VkParam {
    VkType paramType;
    String name;
    String len; // for pointer types, this is how many items are in it
    String full;
    boolean isPointer;
    boolean isDoublePointer;
    int sizeof;
    boolean isConst;

    String getEmulatedType() throws Exception {
        if (isPointer || isDoublePointer) {
            return "U32";
        }
        return paramType.getEmulatedType();
    }
}
