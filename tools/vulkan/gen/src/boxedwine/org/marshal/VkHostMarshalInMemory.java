package boxedwine.org.marshal;

import boxedwine.org.VkFunction;
import boxedwine.org.VkParam;

/**
 * Created by James on 8/22/2021.
 */
public class VkHostMarshalInMemory extends VkHostMarshal {
    // uint32_t* pPhysicalDeviceCount = (uint32_t*)getPhysicalAddress(ARG2, 4);
    public void before(VkFunction fn, StringBuilder out, VkParam param) throws Exception {

        if (param.isDoublePointer) {
            throw new Exception("oops");
        }

        String len = null;

        if (param.paramType.type.equals("char") && param.len.equals("null-terminated")) {
            out.append("U32 len = cpu->memory->strlen(");
            out.append(param.paramArg);
            out.append(");\n");
            len = "len";
        } else if (param.arrayLen == 0 && param.countParam != null) {
            len = param.len;
            if (!param.countInStructure && param.countParam.isPointer) {
                len = "*" + len;
            }
        } else if (param.arrayLen != 0) {
            len = String.valueOf(param.arrayLen);
        }

        if (len != null) {
            out.append("    ");
            out.append(param.paramType.name);
            out.append("* ");
            out.append(param.name);
            out.append(" = new ");
            if (param.paramType.name.equals("void")) {
                out.append("char");
            } else {
                out.append(param.paramType.name);
            }
            out.append("[");
            out.append(len);
            out.append("];\n");

            out.append("    cpu->memory->memcpy(");
            out.append(param.name);
            out.append(", ");
            out.append(param.paramArg);
            out.append(", ");
            out.append(len);
            out.append(" * sizeof(");
            if (param.paramType.name.equals("void")) {
                out.append("char");
            } else {
                out.append(param.paramType.name);
            }
            out.append("));\n");
        } else {
            out.append("    ");
            if (param.isDoublePointer) {
                throw new Exception("oops");
            }
            if (param.paramType.category.equals("struct") || param.paramType.category.equals("union") || param.paramType.type.equals("void")) {
                out.append(param.paramType.name);
                out.append(" tmp_");
                out.append(param.name);
                out.append(";\n");
                out.append("    cpu->memory->memcpy(&tmp_");
                out.append(param.name);
                out.append(", ");
                out.append(param.paramArg);
                out.append(", ");
                out.append(param.paramType.getSize());
                out.append(");\n");

                out.append("    ");
                out.append(param.paramType.name);
                out.append("* ");
                out.append(param.name);
                out.append(" = &tmp_");
                out.append(param.name);
                out.append(";\n");
            } else {
                out.append(param.paramType.name);
                out.append(" tmp_");
                out.append(param.name);
                out.append(" = (");
                out.append(param.paramType.name);
                out.append(") cpu->memory->read");
                switch (param.paramType.getSize()) {
                    case 4:
                        out.append("d");
                        break;
                    case 8:
                        out.append("q");
                        break;
                    default:
                        throw new Exception("oops");
                }
                out.append("(");
                out.append(param.paramArg);
                out.append(");\n");

                out.append("    ");
                out.append(param.paramType.name);
                out.append("* ");
                out.append(param.name);
                out.append(" = &tmp_");
                out.append(param.name);
                out.append(";\n");
            }
        }
    }

    public void after(VkFunction fn, StringBuilder out, VkParam param) throws Exception {
        if (fn.name.equals("vkAllocateMemory") && param.name.equals("pMemory")) {
            out.append("    if (EAX == 0 && pMemory) {\n        registerVkMemoryAllocation(*pMemory, pAllocateInfo->allocationSize);\n    }\n");
        }

        String len = null;

        if (param.paramType.type.equals("char") && param.len.equals("null-terminated")) {
            len = "len";
        } else if (param.arrayLen == 0 && param.countParam != null) {
            len = param.len;
            if (!param.countInStructure && param.countParam.isPointer) {
                len = "*" + len;
            }
        } else if (param.arrayLen != 0) {
            len = String.valueOf(param.arrayLen);
        }

        if (len != null) {
            if (!param.isConst) {
                out.append("    cpu->memory->memcpy(");
                out.append(param.paramArg);
                out.append(", ");
                out.append(param.name);
                out.append(", ");
                out.append(len);
                out.append(" * sizeof(");
                if (param.paramType.name.equals("void")) {
                    out.append("char");
                } else {
                    out.append(param.paramType.name);
                }
                out.append("));\n");
            }
            out.append("    delete[] ");
            out.append(param.name);
            out.append(";\n");
        } else {
            if (!param.isConst) {
                out.append("    ");
                if (param.paramType.category.equals("struct") || param.paramType.category.equals("union") || param.paramType.type.equals("void")) {
                    out.append("cpu->memory->memcpy(");
                    out.append(param.paramArg);
                    out.append(", &tmp_");
                    out.append(param.name);
                    out.append(", ");
                    out.append(param.paramType.getSize());
                    out.append(");\n");
                } else {
                    out.append("cpu->memory->write");
                    switch (param.paramType.getSize()) {
                        case 4:
                            out.append("d");
                            break;
                        case 8:
                            out.append("q");
                            break;
                        default:
                            throw new Exception("oops");
                    }
                    out.append("(");
                    out.append(param.paramArg);
                    if (param.paramType.getSize() == 4) {
                        out.append(", (U32)tmp_");
                    } else {
                        out.append(", (U64)tmp_");
                    }
                    out.append(param.name);
                    out.append(");\n");
                }
            }
        }
    }
}