package boxedwine.org.marshal;

import boxedwine.org.data.VkData;
import boxedwine.org.data.VkFunction;
import boxedwine.org.data.VkParam;

/**
 * Created by James on 8/22/2021.
 */
public class VkHostMarshalInMemory extends VkHostMarshal {
    // uint32_t* pPhysicalDeviceCount = (uint32_t*)getPhysicalAddress(ARG2, 4);
    public void before(VkData data, VkFunction fn, StringBuilder out, VkParam param) throws Exception {
        String len = null;
        String getLen = null;

        if (param.stride != null) {
            throw new Exception();
        }
        if (param.paramType.getType().equals("char") && param.len.equals("null-terminated")) {
            len = "len";
            getLen = "U32 len = cpu->memory->strlen("+param.paramArg+")";
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
            if (param.isDoublePointer) {
                out.append("*");
            }
            out.append("* ");
            out.append(param.name);
            out.append(" = nullptr;\n");
            out.append("    if (");
            out.append(param.paramArg);
            out.append(") {\n");
            if (getLen != null) {
                out.append("        ");
                out.append(getLen);
                out.append(";\n");
            }
            out.append("        ");
            out.append(param.name);
            out.append(" = new ");
            if (param.paramType.name.equals("void")) {
                out.append("char");
            } else {
                out.append(param.paramType.name);
            }
            if (param.isDoublePointer) {
                out.append("*");
            }
            out.append("[");
            out.append(len);
            out.append("];\n");
            if (param.isDoublePointer) {
                String count = param.name + "Count";
                out.append("        ");
                out.append("U32 ");
                out.append(count);
                out.append(" = ");
                out.append(len);
                out.append(";\n");
                out.append("        ");
                out.append("for (U32 i = 0; i < ");
                out.append(count);
                out.append("; i++) {\n");
                out.append("            ");
                out.append(param.name);
                out.append("[i] = new ");
                out.append(param.paramType.name);
                if (param.secondArrayLen == null) {
                    // would be better to parse this out
                    // throw new Exception("oops");
                    param.secondArrayLen = ":TODO: 6";
                }
                out.append("[");
                out.append(param.secondArrayLen);
                out.append("];\n");
                out.append("            ");
                String paramSecondAddress = param.name + "Address";
                out.append("U32 ");
                out.append(paramSecondAddress);
                out.append(" = cpu->memory->readd(");
                out.append(param.paramArg);
                out.append(" + i * 4);\n");
                if (param.paramType.getCategory().equals("struct")) {
                    out.append("            ");
                    out.append("for (U32 j = 0; j < ");
                    out.append(param.secondArrayLen);
                    out.append("; j++) {\n");
                    out.append("                Marshal");
                    out.append(param.paramType.name);
                    out.append("::read(pBoxedInfo, cpu->memory, ");
                    out.append(paramSecondAddress);
                    out.append(" + j * sizeof(");
                    out.append(param.paramType.name);
                    out.append("), ");
                    out.append(param.name);
                    out.append("[j]);\n");
                    out.append("            }\n");
                    out.append("        }\n");
                    param.paramType.setNeedMarshalIn(data, true);
                } else {
                    out.append("            cpu->memory->memcpy(");
                    out.append(param.name);
                    out.append("[i], ");
                    out.append(paramSecondAddress);
                    out.append(", (U32)");
                    out.append(param.secondArrayLen);
                    out.append(" * sizeof(");
                    if (param.paramType.name.equals("void")) {
                        out.append("char");
                    } else {
                        out.append(param.paramType.name);
                    }
                    out.append("));\n");
                    out.append("        }\n");
                }
                out.append("    }\n");
            } else {
                out.append("        cpu->memory->memcpy(");
                out.append(param.name);
                out.append(", ");
                out.append(param.paramArg);
                out.append(", (U32)");
                out.append(len);
                out.append(" * sizeof(");
                if (param.paramType.name.equals("void")) {
                    out.append("char");
                } else {
                    out.append(param.paramType.name);
                }
                out.append("));\n    }\n");
            }
        } else {
            out.append("    ");
            if (param.isDoublePointer) {
                throw new Exception("oops");
            }
            if (param.paramType.getCategory().equals("struct") || param.paramType.getCategory().equals("union") || param.paramType.getType().equals("void")) {
                out.append(param.paramType.name);
                out.append(" tmp_");
                out.append(param.name);
                out.append(";\n");
                out.append("    cpu->memory->memcpy(&tmp_");
                out.append(param.name);
                out.append(", ");
                out.append(param.paramArg);
                out.append(", ");
                out.append(param.paramType.sizeof);
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
                switch (param.paramType.sizeof) {
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

        if (param.paramType.getType().equals("char") && param.len.equals("null-terminated")) {
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
            boolean skipDelete = false;
            if (!param.isConst) {
                if (param.isDoublePointer) {
                    throw new Exception("oops");
                }
                out.append("    if (");
                out.append(param.name);
                out.append(") {\n");
                if (fn.name.equals("vkGetRayTracingCaptureReplayShaderGroupHandlesKHR") && param.name.equals("pData")) {
                    out.append("        pBoxedInfo->rayTracingCaptureReplayShaderGroupHandles[");
                    out.append(param.paramArg);
                    out.append("] = pData;\n");
                    skipDelete = true;
                }

                out.append("        cpu->memory->memcpy(");
                out.append(param.paramArg);
                out.append(", ");
                out.append(param.name);
                out.append(", (U32)");
                out.append(len);
                out.append(" * sizeof(");
                if (param.paramType.name.equals("void")) {
                    out.append("char");
                } else {
                    out.append(param.paramType.name);
                }
                out.append("));\n    }\n");
            }
            if (param.isDoublePointer) {
                out.append("    if (");
                out.append(param.name);
                out.append(") {\n");
                out.append("        for (U32 i = 0; i < ");
                out.append(param.len);
                out.append("; i++) {\n");
                out.append("            delete[] ");
                out.append(param.name);
                out.append("[i];\n");
                out.append("        }\n");
                out.append("    }\n");
            }
            if (!skipDelete) {
                out.append("    delete[] ");
                out.append(param.name);
                out.append(";\n");
            }
        } else {
            if (!param.isConst) {
                out.append("    ");
                if (param.paramType.getCategory().equals("struct") || param.paramType.getCategory().equals("union") || param.paramType.getType().equals("void")) {
                    out.append("cpu->memory->memcpy(");
                    out.append(param.paramArg);
                    out.append(", &tmp_");
                    out.append(param.name);
                    out.append(", ");
                    out.append(param.paramType.sizeof);
                    out.append(");\n");
                } else {
                    out.append("cpu->memory->write");
                    switch (param.paramType.sizeof) {
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
                    if (param.paramType.sizeof == 4) {
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