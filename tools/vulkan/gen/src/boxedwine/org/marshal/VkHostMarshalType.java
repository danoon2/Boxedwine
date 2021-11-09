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
            out.append("(U32 address) {read(address, &this->s);}\n");
            out.append("    static void read(U32 address, ");
            out.append(t.name);
            out.append("* s");
            out.append(") {\n");
            boolean createdParamAddress = false;
            for (VkParam p : t.members) {
                if (p.isPointer) {
                    if (createdParamAddress) {
                        out.append("        paramAddress = readd(address);address+=4;\n");
                    } else {
                        out.append("        U32 paramAddress = readd(address);address+=4;\n");
                        createdParamAddress = true;
                    }
                    out.append("        if (paramAddress == 0) {\n");
                    out.append("            s->");
                    out.append(p.name);
                    out.append(" = NULL;\n        } else {\n");
                    if (p.name.equals("pNext")) {
                        out.append("            s->pNext = vulkanGetNextPtr(paramAddress);\n");
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
                                    out.append("                ");
                                    out.append(p.name);
                                    out.append("[i] = (");
                                    out.append(p.paramType.name);
                                    out.append("*)getPhysicalAddress(readd(paramAddress + i*4), 0);\n");
                                    out.append("            }\n");
                                    out.append("            s->");
                                    out.append(p.name);
                                    out.append(" = ");
                                    out.append(p.name);
                                    out.append(";\n");
                                } else {
                                    throw new Exception("oops");
                                }
                            } else {
                                throw new  Exception("oops");
                            }
                        } else if (!p.paramType.needsMarshaling() || (p.isPointer && !p.isDoublePointer && p.paramType.type.equals("void")) || p.paramType.category.equals("enum")) {
                            out.append("            s->");
                            out.append(p.name);
                            out.append(" = (");
                            out.append(p.paramType.name);
                            out.append("*)getPhysicalAddress(paramAddress, ");
                            if (p.arrayLen != 0) {
                                out.append(p.arrayLen);
                            } else if (!p.len.equals("null-terminated")) {
                                out.append("s->");
                                out.append(p.len);
                            } else {
                                out.append(0);
                            }
                            if (!p.paramType.type.equals("void")) {
                                out.append(" * sizeof(");
                                out.append(p.paramType.name);
                                out.append(")");
                            }
                            out.append(");\n");
                        } else if (p.isPointer && p.paramType.category.equals("struct")) {
                            out.append("            ");
                            out.append(p.paramType.name);
                            out.append("* ");
                            out.append(p.name);
                            out.append(" = new ");
                            out.append(p.paramType.name);
                            out.append("();\n");
                            out.append("            Marshal");
                            out.append(p.paramType.name);
                            out.append("::read(paramAddress, ");
                            out.append(p.name);
                            out.append(");\n");
                            out.append("            s->");
                            out.append(p.name);
                            out.append(" = ");
                            out.append(p.name);
                            out.append(";\n");
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
                        out.append("::read(paramAddress, ");
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
                    if (p.arrayLen > 0) {
                        out.append("        memcopyToNative(address, &s->");
                        out.append(p.name);
                        out.append(", ");
                        out.append(p.arrayLen);
                        out.append(");address+=");
                        out.append(p.arrayLen);
                        out.append(";\n");
                    } else {
                        int width = p.paramType.getSize();
                        if (width > 8 || p.arrayLen > 0 || p.paramType.category.equals("struct")) {
                            out.append("        memcopyToNative(address, &s->");
                            out.append(p.name);
                            out.append(", ");
                            out.append(width);
                            out.append(");address+=");
                            out.append(width);
                            out.append(";\n");
                        } else {
                            out.append("        s->");
                            out.append(p.name);
                            out.append(" = (");
                            out.append(p.paramType.name);
                            out.append(")read");
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
            out.append("    static void write(U32 address, ");
            out.append(t.name);
            out.append("* s");
            out.append(") {\n");
            boolean createdParamAddress = false;
            for (VkParam p : t.members) {
                if (p.isPointer) {
                    if (createdParamAddress) {
                        out.append("        paramAddress = readd(address);address+=4;\n");
                    } else {
                        out.append("        U32 paramAddress = readd(address);address+=4;\n");
                        createdParamAddress = true;
                    }
                    out.append("        if (paramAddress != 0) {\n");
                    if (p.name.equals("pNext")) {
                        out.append("            vulkanWriteNextPtr(paramAddress, s->pNext);\n");
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
                        out.append("::read(paramAddress, ");
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
                        out.append("        memcopyFromNative(address, ");
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
                        out.append("        write");
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
