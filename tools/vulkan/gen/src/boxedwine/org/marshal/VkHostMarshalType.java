package boxedwine.org.marshal;

import boxedwine.org.VkParam;
import boxedwine.org.VkType;

import java.util.HashSet;
import java.util.Vector;

public class VkHostMarshalType {
    private static class PointerData {
        boolean createdParamAddress = false;
    }

    private static class MarshalParamData {
        MarshalParamData() {}
        MarshalParamData(String name, boolean isArray) {
            this.name = name;
            this.isArray = isArray;
            this.isArray = isArray;
        }
        boolean isArray = false;
        String name;
    }

    public static void writeHeader(VkType t, StringBuilder out) throws Exception {
        out.append("class Marshal");
        out.append(t.name);
        out.append(" {\n");
        out.append("public:\n");
        out.append("    Marshal");
        out.append(t.name);
        out.append("() {}\n");
        out.append("    ");
        out.append(t.name);
        out.append(" s;\n");

        if (t.needMarshalIn) {
            out.append("    Marshal");
            out.append(t.name);
            out.append("(KMemory* memory, U32 address) {read(memory, address, &this->s);}\n");

            out.append("    static void read(KMemory* memory, U32 address, ");
            out.append(t.name);
            out.append("* s");
            out.append(");\n");
        }

        if (t.needMarshalOut) {
            out.append("    static void write(KMemory* memory, U32 address, ");
            out.append(t.name);
            out.append("* s");
            out.append(");\n");
        }

        if (t.needDestructor) {
            out.append("    ~Marshal");
            out.append(t.name);
            out.append("();\n");
        }
        out.append("};\n\n");
    }
    private static void marshalInArrayOfPointers(VkParam param, StringBuilder out, Vector<MarshalParamData> paramData) throws Exception {
        if (param.len != null) {
            String[] parts = param.len.split(",");
            if (parts.length == 2) {
                if (param.paramType.needsMarshaling()) {
                    throw new Exception("oops");
                }
                out.append("        ");
                out.append(param.paramType.name);
                out.append("** ");
                out.append(param.name);
                out.append(" = new ");
                paramData.add(new MarshalParamData("s."+param.name, true));
                out.append(param.paramType.name);
                out.append("*[s->");
                out.append(parts[0]);
                out.append("];\n");
                out.append("        for (int i=0;i<(int)s->");
                out.append(parts[0]);
                out.append(";i++) {\n");
                out.append("            U32 itemAddress = memory->readd(paramAddress + i*4);\n");
                out.append("            U32 size = ");
                if (parts[1].equals("null-terminated")) {
                    out.append("memory->strlen(itemAddress)+1;\n");
                } else {
                    throw new Exception("oops");
                }
                out.append("            ");
                out.append(param.name);
                out.append("[i] = new ");
                paramData.add(new MarshalParamData("s."+param.name, true));
                out.append(param.paramType.name);
                out.append("[size];\n");
                out.append("            ");
                out.append("memory->memcpy(");
                out.append(param.name);
                out.append("[i], itemAddress, size);\n");
                out.append("        }\n");
                out.append("        s->");
                out.append(param.name);
                out.append(" = ");
                out.append(param.name);
                out.append(";\n");
            } else {
                throw new Exception("oops");
            }
        } else {
            throw new Exception("oops");
        }
    }
    private static String getParamLen(VkParam param) throws Exception {
        if (param.len != null && param.len.length()>0) {
            return "s->" + param.len;
        }
        if (param.arrayLen == 0) {
            throw new Exception("oops");
        }
        return String.valueOf(param.arrayLen);
    }
    private static void marshalInArrayOfHandles(VkParam param, StringBuilder out, Vector<MarshalParamData> paramData) throws Exception {
        out.append("        ");
        out.append(param.paramType.name);
        out.append("* ");
        out.append(param.name);
        out.append(" = new ");
        paramData.add(new MarshalParamData("s."+param.name, false));
        out.append(param.paramType.name);
        out.append("[");
        out.append(getParamLen(param));
        out.append("];\n");
        out.append("        for (int i=0;i<(int)s->");
        out.append(param.len);
        out.append(";i++) {\n");
        out.append("            U32 itemAddress = memory->readd(paramAddress + i*4);\n");
        out.append("            ");
        out.append(param.name);
        out.append("[i] = (");
        out.append(param.paramType.name);
        out.append(")getVulkanPtr(memory, itemAddress);\n        }\n");
        out.append("        s->");
        out.append(param.name);
        out.append(" = ");
        out.append(param.name);
        out.append(";\n");
    }

    private static void marshalInArrayOfData(VkParam param, StringBuilder out, Vector<MarshalParamData> paramData) throws Exception {
        if (param.len.equals("null-terminated")) {
            out.append("        U32 ");
            out.append(param.name);
            out.append("Len = memory->strlen(paramAddress)+1;\n");
        }
        out.append("        s->");
        out.append(param.name);
        out.append(" = new ");
        paramData.add(new MarshalParamData("s."+param.name, true));
        if (param.paramType.name.equals("void")) {
            if (param.isPointer) {
                out.append("char");
            } else {
                throw new Exception("oops");
            }
        } else {
            out.append(param.paramType.name);
        }
        out.append("[");
        String size;

        if (param.arrayLen != 0) {
            size = String.valueOf(param.arrayLen);
        } else if (param.len.startsWith("(")) {
            size = "(s->";
            size += String.valueOf(param.len.substring(1));
        } else if (param.len.equals("null-terminated")) {
            size = param.name + "Len";
        } else {
            if (Character.isDigit(param.len.charAt(0))) {
                size = "";
            } else {
                size = "(U32)s->";
            }
            size += String.valueOf(param.len);
        }
        if (!param.paramType.type.equals("void")) {
            size += " * sizeof(";
            size += param.paramType.name;
            size += ")";
        }
        out.append(size);
        out.append("];\n");
        out.append("        memory->memcpy((");
        out.append(param.paramType.name);
        out.append("*)s->");
        out.append(param.name);
        out.append(", paramAddress, ");
        out.append(size);
        out.append(");\n");
    }
    private static void marshalInArrayOfStructs(VkParam param, StringBuilder out, Vector<MarshalParamData> paramData) throws Exception {
        out.append("        ");
        out.append(param.paramType.name);
        out.append("* ");
        out.append(param.name);
        out.append(" = new ");
        paramData.add(new MarshalParamData("s."+param.name, true));
        out.append(param.paramType.name);
        out.append("[");
        out.append(getParamLen(param));
        out.append("];\n");
        out.append("        for (U32 i = 0; i < s->");
        out.append(param.len);
        out.append("; i++) {\n");

        out.append("            Marshal");
        out.append(param.paramType.name);
        out.append("::read(memory, paramAddress + i");
        out.append("*");
        out.append(param.paramType.getSize());
        out.append(", &");
        out.append(param.name);
        out.append("[i]);\n");

        out.append("        }\n");
        out.append("        s->");
        out.append(param.name);
        out.append(" = ");
        out.append(param.name);
        out.append(";\n");
        param.paramType.needMarshalIn = true;
    }

    private static void marshalInPointerToStruct(VkParam param, StringBuilder out, Vector<MarshalParamData> paramData) {
        param.paramType.needMarshalIn = true;
        out.append("        ");
        out.append(param.paramType.name);
        out.append("* ");
        out.append(param.name);
        out.append(" = new ");
        paramData.add(new MarshalParamData("s."+param.name, false));
        out.append(param.paramType.name);
        out.append("();\n");
        out.append("        Marshal");
        out.append(param.paramType.name);
        out.append("::read(memory, paramAddress, ");
        out.append(param.name);
        out.append(");\n");
        out.append("        s->");
        out.append(param.name);
        out.append(" = ");
        out.append(param.name);
        out.append(";\n");
    }

    private static void marshalInPointer(VkParam param, StringBuilder out, PointerData data, Vector<MarshalParamData> paramData) throws Exception {
        if (data.createdParamAddress) {
            out.append("    paramAddress = memory->readd(address);address+=4;\n");
        } else {
            out.append("    U32 paramAddress = memory->readd(address);address+=4;\n");
            data.createdParamAddress = true;
        }
        out.append("    if (paramAddress == 0) {\n");
        out.append("        s->");
        out.append(param.name);
        out.append(" = NULL;\n    } else {\n");
        if (param.name.equals("pNext")) {
            out.append("        s->pNext = vulkanGetNextPtr(memory, paramAddress);\n");
            paramData.add(new MarshalParamData("s.pNext", false));
        } else if (param.isArray()) {
            if (param.isDoublePointer) {
                marshalInArrayOfPointers(param, out, paramData);
            } else if (param.paramType.type.equals("VK_DEFINE_HANDLE")) {
                marshalInArrayOfHandles(param, out, paramData);
            } else if (!param.paramType.needsMarshaling() || (param.isPointer && !param.isDoublePointer && param.paramType.type.equals("void")) || param.paramType.category.equals("enum")) {
                marshalInArrayOfData(param, out, paramData);
            } else if (param.paramType.category.equals("struct")) {
                marshalInArrayOfStructs(param, out, paramData);
            } else {
                throw new Exception("oops");
            }
        } else if (param.paramType.category.equals("struct")) {
            marshalInPointerToStruct(param, out, paramData);
        } else {
            throw new Exception("oops");
        }
        out.append("    }\n");
    }

    private static void marshalInParam(VkParam param, StringBuilder out) throws Exception {
        if (param.paramType.category.equals("struct") && param.paramType.needsMarshaling()) {
            param.paramType.needMarshalIn = true;
            out.append("    Marshal");
            out.append(param.paramType.name);
            out.append("::read(memory, address, &s->");
            out.append(param.name);
            out.append("); address+=");
            out.append(param.paramType.getSize());
            out.append(";\n");
        } else if (param.arrayLen > 0) {
            out.append("    memory->memcpy(&s->");
            out.append(param.name);
            out.append(", address, ");
            out.append(param.arrayLen);
            out.append(");address+=");
            out.append(param.arrayLen);
            out.append(";\n");
        } else {
            int width = param.paramType.getSize();
            if (width > 8 || param.arrayLen > 0 || param.paramType.category.equals("struct")) {
                out.append("    memory->memcpy(&s->");
                out.append(param.name);
                out.append(", address, ");
                out.append(width);
                out.append(");address+=");
                out.append(width);
                out.append(";\n");
            } else if (param.paramType.type.equals("VK_DEFINE_HANDLE")) {
                out.append("    s->");
                out.append(param.name);
                out.append(" = (");
                out.append(param.paramType.name);
                out.append(")getVulkanPtr(memory, memory->readd(address));address+=4;\n");
            } else {
                if (param.paramType.type.equals("union")) {
                    String memberName = null;
                    for (VkParam member : param.paramType.members) {
                        if (member.paramType.type.equals("uint64_t") && width == 8) {
                            memberName = member.name;
                        }
                    }
                    if (memberName == null) {
                        throw new Exception("oops");
                    }
                    out.append("    s->");
                    out.append(param.name);
                    out.append(".");
                    out.append(memberName);
                    out.append(" = ");
                } else if (param.paramType.name.equals("float")) {
                    out.append("    MarshalFloat ");
                    out.append(param.name);
                    out.append("Float;\n");
                    out.append("    ");
                    out.append(param.name);
                    out.append("Float.i = ");
                } else {
                    out.append("    s->");
                    out.append(param.name);
                    out.append(" = (");
                    out.append(param.paramType.name);
                    out.append(")");
                }
                out.append("memory->read");
                if (width == 8) {
                    out.append("q(address);address+=8;\n");
                } else if (width == 4) {
                    out.append("d(address);address+=4;\n");
                } else if (width == 2) {
                    out.append("w(address);address+=2;\n");
                } else if (width == 1) {
                    out.append("b(address);address+=1;\n");
                } else {
                    throw new Exception("Unknown width");
                }
                if (param.paramType.name.equals("float")) {
                    out.append("    s->");
                    out.append(param.name);
                    out.append(" = ");
                    out.append(param.name);
                    out.append("Float.f;\n");
                }
            }
        }
    }

    private static void marshalIn(VkType t, StringBuilder out, Vector<MarshalParamData> paramsData) throws Exception {
        out.append("void Marshal");
        out.append(t.name);
        out.append("::read(KMemory* memory, U32 address, ");
        out.append(t.name);
        out.append("* s");
        out.append(") {\n");
        PointerData pointerData = new PointerData();

        for (VkParam param : t.members) {
            if (param.isPointer) {
                marshalInPointer(param, out, pointerData, paramsData);
            } else {
                marshalInParam(param, out);
            }
        }
        out.append("}\n");
    }

    private static void marshalOutPointer(VkParam param, StringBuilder out, PointerData data, Vector<MarshalParamData> paramData) {
        if (data.createdParamAddress) {
            out.append("    paramAddress = memory->readd(address);address+=4;\n");
        } else {
            out.append("    U32 paramAddress = memory->readd(address);address+=4;\n");
            data.createdParamAddress = true;
        }
        out.append("    if (paramAddress != 0) {\n");
        if (param.name.equals("pNext")) {
            out.append("        vulkanWriteNextPtr(memory, paramAddress, s->pNext);\n");
        } else if (param.paramType.category.equals("struct")) {
            param.paramType.needMarshalIn = true;
            out.append("        ");
            out.append(param.paramType.name);
            out.append("* ");
            out.append(param.name);
            out.append(" = new ");
            paramData.add(new MarshalParamData("s."+param.name, false));
            out.append(param.paramType.name);
            out.append("();\n");
            out.append("        Marshal");
            out.append(param.paramType.name);
            out.append("::read(memory, paramAddress, ");
            out.append(param.name);
            out.append(");\n");
            out.append("        s->");
            out.append(param.name);
            out.append(" = ");
            out.append(param.name);
            out.append(";\n");
        } else {
            out.append("        kpanic(\"Can't marshal void*\");\n");
        }
        out.append("    }\n");
    }
    private static void marshalOutParam(VkType type, VkParam param, StringBuilder out) throws Exception {
        int width = param.getSize();
        if (width > 8 || param.arrayLen > 0 || param.paramType.category.equals("struct")) {
            out.append("    memory->memcpy(address, ");
            if (param.arrayLen == 0) {
                // for example
                // typedef struct VkPhysicalDeviceLimits {
                // ...
                // uint32_t              maxComputeWorkGroupCount[3];
                out.append("&");
            }
            out.append("s->");
            out.append(param.name);
            out.append(", ");
            out.append(width);
            out.append("); address+=");
            out.append(width);
            out.append(";\n");
        } else {
            if (width == 4 && param.paramType.type.equals("VK_DEFINE_HANDLE")) {
                out.append("    kpanic(\"");
                out.append(type.name);
                out.append(".");
                out.append(param.name);
                out.append(" did not marshal write correctly\");\n");
                return;
            }
            if (param.paramType.name.equals("float")) {
                out.append("    MarshalFloat ");
                out.append(param.name);
                out.append("Float;\n");
                out.append("    ");
                out.append(param.name);
                out.append("Float.f = s->");
                out.append(param.name);
                out.append(";\n    memory->writed(address, ");
                out.append(param.name);
                out.append("Float.i);address+=4;\n");
                return;
            }
            out.append("    memory->write");
            if (width == 8) {
                boolean skipName = false;
                out.append("q(address, ");
                if (param.paramType.type.equals("VK_DEFINE_NON_DISPATCHABLE_HANDLE")) {
                    out.append("(U64)");
                } else if (param.paramType.type.equals("union")) {
                    for (VkParam member : param.paramType.members) {
                        if (member.paramType.getSize() == 8) {
                            out.append("s->");
                            out.append(param.name);
                            out.append(".");
                            out.append(member.name);
                            skipName = true;
                            break;
                        }
                    }
                }
                if (!skipName) {
                    out.append("s->");
                    out.append(param.name);
                }
                out.append(");address+=8;\n");
            } else if (width == 4) {
                if (param.paramType.name.equals("size_t")) {
                    // size_t is 32-bit on win32, but host might be 64-bit, this will remove the warning
                    out.append("d(address, (U32)s->");
                } else {
                    out.append("d(address, s->");
                }
                out.append(param.name);
                out.append(");address+=4;\n");
            } else if (width == 2) {
                out.append("w(address, s->");
                out.append(param.name);
                out.append(");address+=2;\n");
            } else if (width == 1) {
                out.append("b(address, s->");
                out.append(param.name);
                out.append(");address+=1;\n");
            } else {
                throw new Exception("Unknown width");
            }
        }
    }
    private static void marshalOut(VkType t, StringBuilder out, Vector<MarshalParamData> paramsData) throws Exception {
        out.append("void Marshal");
        out.append(t.name);
        out.append("::write(KMemory* memory, U32 address, ");
        out.append(t.name);
        out.append("* s");
        out.append(") {\n");
        PointerData pointerData = new PointerData();
        for (VkParam param : t.members) {
            if (param.isPointer) {
                marshalOutPointer(param, out, pointerData, paramsData);
            } else {
                marshalOutParam(t, param, out);
            }
        }
        out.append("}\n");
    }

    private static void marshalDestructor(VkType t, StringBuilder out, Vector<MarshalParamData> paramsData) {
        out.append("Marshal");
        out.append(t.name);
        out.append("::~Marshal");
        out.append(t.name);
        out.append("() {\n");
        HashSet<String> alreadyAdded = new HashSet<>();

        for (MarshalParamData data : paramsData) {
            if (alreadyAdded.contains(data.name)) {
                continue;
            }
            out.append("    delete");
            if (data.isArray) {
                out.append("[]");
            }
            out.append(" ");
            out.append(data.name);
            out.append(";\n");
            alreadyAdded.add(data.name);
        }
        out.append("}\n");
    }
    public static void write(VkType t, StringBuilder out) throws Exception {
        Vector<MarshalParamData> paramsData = new Vector<>();

        if (t.needMarshalIn) {
            marshalIn(t, out, paramsData);
        }
        if (t.needMarshalOut) {
            marshalOut(t, out, paramsData);
        }
        if (paramsData.size() > 0) {
            t.needDestructor = true;
            marshalDestructor(t, out, paramsData);
        }
    }
}
