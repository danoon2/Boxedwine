package boxedwine.org.marshal;

import boxedwine.org.VkParam;
import boxedwine.org.VkType;

public class VkHostMarshalType {
    public static void write(VkType t, StringBuilder out) throws Exception {
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
            out.append(") {\n");
            boolean createdParamAddress = false;
            if (t.name.equals("VkComputePipelineCreateInfo")) {
                int ii=0;
            }
            for (VkParam p : t.members) {
                if (p.isPointer) {
                    if (createdParamAddress) {
                        out.append("        paramAddress = memory->readd(address);address+=4;\n");
                    } else {
                        out.append("        U32 paramAddress = memory->readd(address);address+=4;\n");
                        createdParamAddress = true;
                    }
                    out.append("        if (paramAddress == 0) {\n");
                    out.append("            s->");
                    out.append(p.name);
                    out.append(" = NULL;\n        } else {\n");
                    if (p.name.equals("pNext")) {
                        out.append("            s->pNext = vulkanGetNextPtr(memory, paramAddress);\n");
                    } else if (p.len != null || p.arrayLen != 0) {
                        if (p.isDoublePointer) {
                            if (p.len != null) {
                                String[] parts = p.len.split(",");
                                if (parts.length == 2) {
                                    if (p.paramType.needsMarshaling()) {
                                        throw new Exception("oops");
                                    }
                                    out.append("            ");
                                    out.append(p.paramType.name);
                                    out.append("** ");
                                    out.append(p.name);
                                    out.append(" = new ");
                                    out.append(p.paramType.name);
                                    out.append("*[s->");
                                    out.append(parts[0]);
                                    out.append("];\n");
                                    out.append("            for (int i=0;i<(int)s->");
                                    out.append(parts[0]);
                                    out.append(";i++) {\n");
                                    out.append("                U32 size = ");
                                    if (parts[1].equals("null-terminated")) {
                                        out.append("memory->strlen(paramAddress + i*4)+1;\n");
                                    } else {
                                        throw new Exception("oops");
                                    }
                                    out.append("                ");
                                    out.append(p.name);
                                    out.append("[i] = new ");
                                    out.append(p.paramType.name);
                                    out.append("[size];\n");
                                    out.append("                ");
                                    out.append("memory->memcpy(");
                                    out.append(p.name);
                                    out.append("[i], paramAddress + i*4, size);\n");
                                    out.append("            }\n");
                                } else {
                                    throw new Exception("oops");
                                }
                            } else {
                                throw new  Exception("oops");
                            }
                        } else if (!p.paramType.needsMarshaling() || (p.isPointer && !p.isDoublePointer && p.paramType.type.equals("void")) || p.paramType.category.equals("enum")) {
                            if (p.paramType.type.equals("VK_DEFINE_HANDLE")) {
                                throw new  Exception("oops"); // would need to be marshalled since this is 4-bytes in win32 and 8-bytes on 64-bit host
                            }
                            out.append("            s->");
                            out.append(p.name);
                            out.append(" = new ");
                            if (p.paramType.name.equals("void")) {
                                if (p.isPointer) {
                                    out.append("char");
                                } else {
                                    throw new Exception("oops");
                                }
                            } else {
                                out.append(p.paramType.name);
                            }
                            out.append("[");
                            String size;

                            if (p.arrayLen != 0) {
                                size = String.valueOf(p.arrayLen);
                            } else if (p.len.startsWith("(")) {
                                size = "(s->";
                                size += String.valueOf(p.len.substring(1));
                            } else if (!p.len.equals("null-terminated")) {
                                size = "s->";
                                size += String.valueOf(p.len);
                            } else {
                                size = "0";
                            }
                            if (!p.paramType.type.equals("void")) {
                                size += " * sizeof(";
                                size += p.paramType.name;
                                size += ")";
                            }
                            out.append(size);
                            out.append("];\n");
                            out.append("            memory->memcpy((");
                            out.append(p.paramType.name);
                            out.append("*)s->");
                            out.append(p.name);
                            out.append(", paramAddress, ");
                            out.append(size);
                            out.append(");\n");
                        } else if (p.paramType.type.equals("VK_DEFINE_HANDLE") && p.len != null && p.len.length() > 0) {
                            out.append("            ");
                            out.append(p.paramType.name);
                            out.append("* ");
                            out.append(p.name);
                            out.append(" = new ");
                            out.append(p.paramType.name);
                            out.append("[s->");
                            out.append(p.len);
                            out.append("];\n");
                            out.append("            for (int i=0;i<(int)s->");
                            out.append(p.len);
                            out.append(";i++) {\n");
                            out.append("                ");
                            out.append(p.name);
                            out.append("[i] = (");
                            out.append(p.paramType.name);
                            out.append(")getVulkanPtr(memory, paramAddress);\n            }\n");
                            out.append("            s->");
                            out.append(p.name);
                            out.append(" = ");
                            out.append(p.name);
                            out.append(";\n");
                        } else if (p.paramType.category.equals("struct")) {
                            if (p.len != null && p.len.length() > 0) {
                                out.append("            ");
                                out.append(p.paramType.name);
                                out.append("* ");
                                out.append(p.name);
                                out.append(" = new ");
                                out.append(p.paramType.name);
                                out.append("[s->");
                                out.append(p.len);
                                out.append("];\n");
                                out.append("            for (U32 i = 0; i < s->");
                                out.append(p.len);
                                out.append("; i++) {\n");

                                out.append("                Marshal");
                                out.append(p.paramType.name);
                                out.append("::read(memory, paramAddress, &");
                                out.append(p.name);
                                out.append("[i]);\n");

                                out.append("            }\n");
                                out.append("            s->");
                                out.append(p.name);
                                out.append(" = ");
                                out.append(p.name);
                                out.append(";\n");
                            } else {
                                out.append("            ");
                                out.append(p.paramType.name);
                                out.append("* ");
                                out.append(p.name);
                                out.append(" = new ");
                                out.append(p.paramType.name);
                                out.append("();\n");
                                out.append("            Marshal");
                                out.append(p.paramType.name);
                                out.append("::read(memory, paramAddress, ");
                                out.append(p.name);
                                out.append(");\n");
                                out.append("            s->");
                                out.append(p.name);
                                out.append(" = ");
                                out.append(p.name);
                                out.append(";\n");
                            }
                            p.paramType.needMarshalIn = true;
                        } else {
                            throw new Exception("oops");
                        }
                    } else if (p.paramType.category.equals("struct")) {
                        p.paramType.needMarshalIn = true;
                        out.append("            ");
                        out.append(p.paramType.name);
                        out.append("* ");
                        out.append(p.name);
                        out.append(" = new ");
                        out.append(p.paramType.name);
                        out.append("();\n");
                        out.append("            Marshal");
                        out.append(p.paramType.name);
                        out.append("::read(memory, paramAddress, ");
                        out.append(p.name);
                        out.append(");\n");
                        out.append("            s->");
                        out.append(p.name);
                        out.append(" = ");
                        out.append(p.name);
                        out.append(";\n");
                    } else {
                        throw new Exception("oops");
                    }
                    out.append("        }\n");
                } else {
                    if (p.paramType.category.equals("struct") && p.paramType.needsMarshaling()) {
                        p.paramType.needMarshalIn = true;
                        out.append("        Marshal");
                        out.append(p.paramType.name);
                        out.append("::read(memory, address, &s->");
                        out.append(p.name);
                        out.append("); address+=");
                        out.append(p.paramType.getSize());
                         out.append(";\n");
                    } else if (p.arrayLen > 0) {
                        out.append("        memory->memcpy(&s->");
                        out.append(p.name);
                        out.append(", address, ");
                        out.append(p.arrayLen);
                        out.append(");address+=");
                        out.append(p.arrayLen);
                        out.append(";\n");
                    } else {
                        int width = p.paramType.getSize();
                        if (width > 8 || p.arrayLen > 0 || p.paramType.category.equals("struct")) {
                            out.append("        memory->memcpy(&s->");
                            out.append(p.name);
                            out.append(", address, ");
                            out.append(width);
                            out.append(");address+=");
                            out.append(width);
                            out.append(";\n");
                        } else if (p.paramType.type.equals("VK_DEFINE_HANDLE")) {
                            out.append("        s->");
                            out.append(p.name);
                            out.append(" = (");
                            out.append(p.paramType.name);
                            out.append(")getVulkanPtr(memory, memory->readd(address));address+=4;\n");
                        } else {
                            out.append("        s->");
                            out.append(p.name);
                            out.append(" = (");
                            out.append(p.paramType.name);
                            out.append(")memory->read");
                            if (width == 8) {
                                out.append("q(address);address+=8;\n");
                            } else if (width == 4) {
                                out.append("d(address);address+=4;\n");
                            } else if (width == 2) {
                                out.append("w(address);address+=2;\n");
                            } else {
                                throw new Exception("Unknown width");
                            }
                        }
                    }
                }
            }
            out.append("    }\n");
        }
        if (t.needMarshalOut) {
            out.append("    static void write(KMemory* memory, U32 address, ");
            out.append(t.name);
            out.append("* s");
            out.append(") {\n");
            boolean createdParamAddress = false;
            for (VkParam p : t.members) {
                if (p.isPointer) {
                    if (createdParamAddress) {
                        out.append("        paramAddress = memory->readd(address);address+=4;\n");
                    } else {
                        out.append("        U32 paramAddress = memory->readd(address);address+=4;\n");
                        createdParamAddress = true;
                    }
                    out.append("        if (paramAddress != 0) {\n");
                    if (p.name.equals("pNext")) {
                        out.append("            vulkanWriteNextPtr(memory, paramAddress, s->pNext);\n");
                        out.append("            delete s->pNext;\n");
                    } else if (p.paramType.category.equals("struct")) {
                        p.paramType.needMarshalIn = true;
                        out.append("            ");
                        out.append(p.paramType.name);
                        out.append("* ");
                        out.append(p.name);
                        out.append(" = new ");
                        out.append(p.paramType.name);
                        out.append("();\n");
                        out.append("            Marshal");
                        out.append(p.paramType.name);
                        out.append("::read(memory, paramAddress, ");
                        out.append(p.name);
                        out.append(");\n");
                        out.append("            s->");
                        out.append(p.name);
                        out.append(" = ");
                        out.append(p.name);
                        out.append(";\n");
                    } else {
                        out.append("kpanic(\"        Can't marshal void*\");\n");
                    }
                    out.append("        }\n");
                } else {
                    int width = p.getSize();
                    if (width > 8 || p.arrayLen > 0 || p.paramType.category.equals("struct")) {
                        out.append("        memory->memcpy(address, ");
                        if (p.arrayLen == 0) {
                            out.append("&");
                        }
                        out.append("s->");
                        out.append(p.name);
                        out.append(", ");
                        out.append(width);
                        out.append("); address+=");
                        out.append(width);
                        out.append(";\n");
                    } else {
                        out.append("        memory->write");
                        if (width == 8) {
                            out.append("q(address, s->");
                            out.append(p.name);
                            out.append(");address+=8;\n");
                        } else if (width == 4) {
                            out.append("d(address, s->");
                            out.append(p.name);
                            out.append(");address+=4;\n");
                        } else if (width == 2) {
                            out.append("w(address, s->");
                            out.append(p.name);
                            out.append(");address+=2;\n");
                        } else {
                            throw new Exception("Unknown width");
                        }
                    }
                }
            }
            out.append("    }\n");
        }
        out.append("};\n\n");
    }
}
