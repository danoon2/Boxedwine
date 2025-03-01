package boxedwine.org.marshal;

import boxedwine.org.data.VkData;
import boxedwine.org.data.VkFunction;
import boxedwine.org.data.VkParam;
import boxedwine.org.data.VkType;

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
        }
        boolean isArray = false;
        String name;
        boolean deleteItems = false;
        String itemCount;
        boolean unlock = false;
        boolean isVoid = false;
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

        if (t.isNeedMarshalIn()) {
            if (!t.name.equals("VkMemoryToImageCopy") && !t.name.equals("VkImageToMemoryCopy")) {
                out.append("    Marshal");
                out.append(t.name);
                out.append("(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}\n");
            }
            out.append("    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, ");
            if (t.name.equals("VkMemoryToImageCopy") || t.name.equals("VkImageToMemoryCopy")) {
                out.append("VkImage image, ");
            }
            out.append("U32 address, ");
            out.append(t.name);
            out.append("* s");
            out.append(");\n");
        }

        if (t.needMarshalOut) {
            out.append("    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, const ");
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
                if (needsMarshaling(param.paramType)) {
                    if (parts[1].equals("1")) {
                        out.append("        ");
                        out.append(param.paramType.name);
                        out.append("** ");
                        out.append(param.name);
                        out.append(" = new ");
                        MarshalParamData item = new MarshalParamData("s."+param.name, true);
                        item.itemCount = "s." + parts[0];
                        item.deleteItems = true;
                        paramData.add(item);
                        out.append(param.paramType.name);
                        out.append("*[s->");
                        out.append(parts[0]);
                        out.append("];\n");
                        out.append("        for (int i=0;i<(int)s->");
                        out.append(parts[0]);
                        out.append(";i++) {\n            ");
                        out.append(param.name);
                        out.append("[i] = new ");
                        out.append(param.paramType.name);
                        out.append("();\n");
                        out.append("            U32 readAddress = memory->readd(paramAddress + i * 4);\n");
                        out.append("            Marshal");
                        out.append(param.paramType.name);
                        out.append("::read(pBoxedInfo, memory, readAddress, ");
                        out.append(param.name);
                        out.append("[i]);\n");
                        out.append("        }\n");
                        out.append("        s->");
                        out.append(param.name);
                        out.append(" = ");
                        out.append(param.name);
                        out.append(";\n");
                        return;
                    }
                    out.append(":TODO: 2\n");
                    return;
                    //throw new Exception("oops");
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
                    //throw new Exception("oops");
                    out.append(":TODO: 1\n");
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
                //throw new Exception("oops");
                out.append("        kpanic(\"3\");\n");
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
        if (param.len != null && param.len.equals("null-terminated")) {
            out.append("        U32 ");
            out.append(param.name);
            out.append("Len = memory->strlen(paramAddress)+1;\n");
        }
        out.append("        s->");
        out.append(param.name);
        out.append(" = ");

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

        if (param.isConst) {
            out.append("(");
            if (param.paramType.name.equals("void")) {
                out.append("char");
            } else {
                out.append(param.paramType.name);
            }
            out.append("*)memory->lockReadOnlyMemory(paramAddress, ");
            if (param.paramType.name.equals("void")) {
                size += " * sizeof(char)";
            } else {
                size += " * sizeof(";
                size += param.paramType.name;
                size += ")";
            }
            out.append(size);
            out.append(");\n");
            MarshalParamData data = new MarshalParamData("s."+param.name, true);
            data.unlock = true;
            paramData.add(data);
            return;
        }

        out.append("new ");
        MarshalParamData data = new MarshalParamData("s."+param.name, true);
        paramData.add(data);
        if (param.paramType.name.equals("void")) {
            if (param.isPointer) {
                out.append("char");
                data.isVoid = true;
            } else {
                throw new Exception("oops");
            }
        } else {
            out.append(param.paramType.name);
        }
        out.append("[");

        out.append(size);
        out.append("];\n");
        if (param.paramType.name.equals("void")) {
            size += " * sizeof(char)";
        } else {
            size += " * sizeof(";
            size += param.paramType.name;
            size += ")";
        }
        out.append("        memory->memcpy((");
        out.append(param.paramType.name);
        out.append("*)s->");
        out.append(param.name);
        out.append(", paramAddress, ");
        if (param.paramType.getCategory().equals("struct")) {
            if (needsMarshaling(param.paramType)) {
                throw new Exception();
            }
        }
        out.append(size);
        out.append(");\n");
    }
    private static void marshalInArrayOfStructs(VkData data, VkParam param, StringBuilder out, Vector<MarshalParamData> paramData) throws Exception {
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
        out.append("        for (U32 i = 0; i < ");
        out.append(getParamLen(param));
        out.append("; i++) {\n");

        out.append("            Marshal");
        out.append(param.paramType.name);
        out.append("::read(pBoxedInfo, memory, ");
        if (param.paramType.name.equals("VkMemoryToImageCopy")) {
            out.append("s->dstImage, ");
        } else if (param.paramType.name.equals("VkImageToMemoryCopy")) {
            out.append("s->srcImage, ");
        }
        out.append("paramAddress + i");
        out.append("*");
        out.append(param.paramType.sizeof);
        out.append(", &");
        out.append(param.name);
        out.append("[i]);\n");

        out.append("        }\n");
        out.append("        s->");
        out.append(param.name);
        out.append(" = ");
        out.append(param.name);
        out.append(";\n");
        param.paramType.setNeedMarshalIn(data,true);
    }

    private static void marshalInPointerToStruct(VkData data, VkParam param, StringBuilder out, Vector<MarshalParamData> paramData) {
        param.paramType.setNeedMarshalIn(data, true);
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
        out.append("::read(pBoxedInfo, memory, paramAddress, ");
        out.append(param.name);
        out.append(");\n");
        out.append("        s->");
        out.append(param.name);
        out.append(" = ");
        out.append(param.name);
        out.append(";\n");
    }

    private static void marshalInPointer(VkData data, VkParam param, StringBuilder out, PointerData pdata, Vector<MarshalParamData> paramData) throws Exception {
        if (pdata.createdParamAddress) {
            out.append("    paramAddress = memory->readd(address);address+=4;\n");
        } else {
            out.append("    U32 paramAddress = memory->readd(address);address+=4;\n");
            pdata.createdParamAddress = true;
        }
        out.append("    if (paramAddress == 0) {\n");
        out.append("        s->");
        out.append(param.name);
        out.append(" = NULL;\n    } else {\n");
        if (param.name.equals("pNext")) {
            out.append("        s->pNext = vulkanGetNextPtr(pBoxedInfo, memory, paramAddress);\n");
            paramData.add(new MarshalParamData("s.pNext", false));
        } else if (param.isArray()) {
            if (param.isDoublePointer) {
                marshalInArrayOfPointers(param, out, paramData);
            } else if (param.paramType.getType().equals("VK_DEFINE_HANDLE")) {
                marshalInArrayOfHandles(param, out, paramData);
            } else if (!needsMarshaling(param.paramType) || (param.isPointer && !param.isDoublePointer && (param.paramType.getType().equals("void") || param.paramType.name.equals("void"))) || param.paramType.getCategory().equals("enum")) {
                marshalInArrayOfData(param, out, paramData);
            } else if (param.paramType.getCategory().equals("struct")) {
                marshalInArrayOfStructs(data, param, out, paramData);
            } else {
                out.append("oops\n");
                //throw new Exception("oops");
            }
        } else if (param.paramType.getCategory().equals("struct")) {
            marshalInPointerToStruct(data, param, out, paramData);
        } else {
            if (param.name.equals("pShaderGroupCaptureReplayHandle")) {
                out.append("        s->");
                out.append(param.name);
                out.append(" = pBoxedInfo->rayTracingCaptureReplayShaderGroupHandles[paramAddress];\n");
                out.append("        if (!s->pShaderGroupCaptureReplayHandle) {\n            kpanic(\"MarshalVkRayTracingShaderGroupCreateInfoKHR::read oops\");\n        }\n");
            } else {
                out.append("        kpanic(\"4\");\n");
            }
            //throw new Exception("oops");
        }
        out.append("    }\n");
    }

    private static void marshalInParam(VkData data, VkParam param, StringBuilder out, Vector<MarshalParamData> paramsData) throws Exception {
        if (param.paramType.getCategory().equals("union")) {
            for (VkParam p : param.paramType.members) {
                if (p.isPointer) {
                    if (param.paramType.name.equals("VkDeviceOrHostAddressConstKHR") || param.paramType.name.equals("VkDeviceOrHostAddressKHR")) {
                        out.append("    //assuming deviceAddress is used\n");
                    } else {
                        out.append("    kpanic(\"A\");\n");
                    }
                }
            }
        }
        if (param.paramType.getCategory().equals("struct") && needsMarshaling(param.paramType)) {
            param.paramType.setNeedMarshalIn(data, true);
            if (param.arrayLen > 0) {
                out.append("    for (U32 i = 0; i < ");
                out.append(param.arrayLen);
                out.append("; i++) {\n");
                out.append("        Marshal");
                out.append(param.paramType.name);
                out.append("::read(pBoxedInfo, memory, address, &s->");
                out.append(param.name);
                out.append("[i]); address+=");
                out.append(param.paramType.sizeof);
                out.append(";\n");
                out.append("    }\n");
            } else {
                out.append("    Marshal");
                out.append(param.paramType.name);
                out.append("::read(pBoxedInfo, memory, address, &s->");
                out.append(param.name);
                out.append("); address+=");
                out.append(param.paramType.sizeof);
                out.append(";\n");
            }
        } else if (param.paramType.getType().equals("VK_DEFINE_HANDLE")) {
            if (param.isArray()) {
                out.append("    paramAddress = memory->readd(address);address+=4;\n");
                out.append("    if (paramAddress) {\n");
                if (param.arrayLen == 0) {
                    out.append("        s->");
                    out.append(param.name);
                    out.append(" = new ");
                    out.append(param.paramType.name);
                    out.append("[");
                    out.append(param.len);
                    out.append("];\n");
                    MarshalParamData item = new MarshalParamData("s."+param.name, true);
                    paramsData.add(item);
                }
                out.append("        for (U32 i=0;i<");
                if (param.arrayLen > 0) {
                    out.append(param.arrayLen);
                } else {
                    out.append(param.len);
                }
                out.append(";i++) {\n            s->");
                out.append(param.name);
                out.append("[i] = (");
                out.append(param.paramType.name);
                out.append(")getVulkanPtr(memory, memory->readd(paramAddress + i*4));\n        }\n    }\n");
            } else {
                out.append("    s->");
                out.append(param.name);
                out.append(" = (");
                out.append(param.paramType.name);
                out.append(")getVulkanPtr(memory, memory->readd(address));address+=4;\n");
            }
        } else if (param.arrayLen > 0) {
            if (param.paramType.getCategory().equals("struct")) {
                for (VkParam member : param.paramType.members) {
                    if (member.paramType.isNeedMarshalIn() || member.paramType.needMarshalOut) {
                        throw new Exception("oops");
                    }
                }
            }
            if (!simpleTypes.contains(param.paramType.name)) {
                out.append("    static_assert(sizeof(");
                out.append(param.paramType.name);
                out.append(") == ");
                out.append(param.paramType.sizeof);
                out.append(");\n");
            }
            out.append("    memory->memcpy(&s->");
            out.append(param.name);
            out.append(", address, ");
            out.append(param.getSize());
            out.append(");address+=");
            out.append(param.getSize());
            out.append(";\n");
        } else {
            int width = param.getSize();

            if (width > 8 || param.paramType.getCategory().equals("struct")) {
                if (param.paramType.getCategory().equals("union")) {
                   if (param.paramType.name.equals("VkAccelerationStructureGeometryDataKHR")) {
                       out.append("    if (s->geometryType == VK_GEOMETRY_TYPE_TRIANGLES_KHR) {\n");
                       out.append("        MarshalVkAccelerationStructureGeometryTrianglesDataKHR::read(pBoxedInfo, memory, address, &s->geometry.triangles);\n");
                       out.append("    } else if (s->geometryType == VK_GEOMETRY_TYPE_AABBS_KHR) {\n");
                       out.append("        MarshalVkAccelerationStructureGeometryAabbsDataKHR::read(pBoxedInfo, memory, address, &s->geometry.aabbs);\n");
                       out.append("    } else if (s->geometryType == VK_GEOMETRY_TYPE_INSTANCES_KHR) {\n");
                       out.append("        MarshalVkAccelerationStructureGeometryInstancesDataKHR::read(pBoxedInfo, memory, address, &s->geometry.instances);\n    }\n");
                       out.append("    address+=");
                       out.append(width);
                       out.append(";\n");
                       return;
                   } else {
                       for (VkParam member : param.paramType.members) {
                           if (!member.paramType.getCategory().equals("platform")) {
                               boolean allPlatform = true;
                               if (member.paramType.getCategory().equals("union")) {
                                   for (VkParam unionMember : member.paramType.members) {
                                       if (!unionMember.paramType.getCategory().equals("platform")) {
                                           allPlatform = false;
                                       }
                                   }
                               }
                               //throw new Exception("oops");
                               if (!allPlatform) {
                                   out.append(":TODO: 5\n");
                                   break;
                               }
                           }
                       }
                   }
                }
                out.append("    memory->memcpy(&s->");
                out.append(param.name);
                out.append(", address, ");
                out.append(width);
                out.append(");address+=");
                out.append(width);
                out.append(";\n");
            } else {
                boolean needOutParen = false;
                if (param.paramType.getType().equals("union") || param.paramType.getCategory().equals("union")) {
                    String memberName = null;
                    for (VkParam member : param.paramType.members) {
                        if (member.paramType.getType().equals("uint64_t") && width == 8) {
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
                    out.append(" = ");
                    if (!param.paramType.name.equals("void")) {
                        out.append("(");
                        out.append(param.paramType.name);
                        out.append(")");
                    } else {
                        out.append("(void*)static_cast<intptr_t>(");
                        needOutParen = true;
                    }
                }
                out.append("memory->read");
                if (width == 8) {
                    out.append("q(address)");
                    if (needOutParen) {
                        out.append(")");
                    }
                    out.append(";address+=8;\n");
                } else if (width == 4) {
                    out.append("d(address)");
                    if (needOutParen) {
                        out.append(")");
                    }
                    out.append(";address+=4;\n");
                } else if (width == 2) {
                    out.append("w(address)");
                    if (needOutParen) {
                        out.append(")");
                    }
                    out.append(";address+=2;\n");
                } else if (width == 1) {
                    out.append("b(address)");
                    if (needOutParen) {
                        out.append(")");
                    }
                    out.append(";address+=1;\n");
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

    private static void marshalIn(VkData data, VkType t, StringBuilder out, Vector<MarshalParamData> paramsData) throws Exception {
        out.append("void Marshal");
        out.append(t.name);
        out.append("::read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, ");
        if (t.name.equals("VkMemoryToImageCopy") || t.name.equals("VkImageToMemoryCopy")) {
            out.append("VkImage image, ");
        }
        out.append("U32 address, ");
        out.append(t.name);
        out.append("* s");
        out.append(") {\n");
        PointerData pointerData = new PointerData();

        if (data.stubMarshalRead.contains(t.name)) {
            out.append("    kpanic(\"Marshal");
            out.append(t.name);
            out.append("::read\");\n");
            //out.append("}\n");

        }
        if (t.getCategory().equals("union")) {
            // at this point all members should be the same size and simple native types (no pointers)
            out.append("    memory->memcpy(s, address, ");
            out.append(t.sizeof);
            out.append(");\n");
            out.append("}\n");
            return;
        }
        int offset = 0;
        for (VkParam param : t.members) {
            int alignment = param.getAlignment();
            if ((offset % alignment) != 0) {
                out.append("    address+=");
                out.append(alignment - (offset % alignment));
                out.append("; // structure padding\n");
                offset += alignment - (offset % alignment);
            }
            if (t.name.equals("VkPerformanceValueINTEL") && param.name.equals("data")) {
                out.append("    switch (s->type) {\n");
                out.append("    case VK_PERFORMANCE_VALUE_TYPE_UINT32_INTEL:\n");
                out.append("        s->data.value32 = memory->readd(address);\n");
                out.append("        break;\n");
                out.append("    case VK_PERFORMANCE_VALUE_TYPE_UINT64_INTEL:\n");
                out.append("        s->data.value64 = memory->readq(address);\n");
                out.append("        break;\n");
                out.append("    case VK_PERFORMANCE_VALUE_TYPE_FLOAT_INTEL:\n");
                out.append("        int2Float i2f;\n");
                out.append("        i2f.i = memory->readd(address);\n");
                out.append("        s->data.valueFloat = i2f.f;\n");
                out.append("        break;\n");
                out.append("    case VK_PERFORMANCE_VALUE_TYPE_BOOL_INTEL:\n");
                out.append("        s->data.valueBool = memory->readd(address);\n");
                out.append("        break;\n");
                out.append("    case VK_PERFORMANCE_VALUE_TYPE_STRING_INTEL: {\n");
                out.append("        U32 strAddress = memory->readd(address);\n");
                out.append("        U32 len = memory->strlen(strAddress);\n");
                out.append("        s->data.valueString = new char[len + 1];\n");
                out.append("        memory->memcpy((U8*)s->data.valueString, strAddress, len + 1);\n");
                out.append("        break; }\n");
                out.append("    default:\n");
                out.append("        kpanic_fmt(\"MarshalVkPerformanceValueINTEL::read unknown s->type %d\", s->type);\n");
                out.append("    }\n");
                out.append("    address += 8;\n");
                paramsData.add(new MarshalParamData("s.data", false));
            } else if (t.name.equals("VkIndirectExecutionSetCreateInfoEXT") && param.name.equals("info")) {
                out.append("    if (s->type == VK_INDIRECT_EXECUTION_SET_INFO_TYPE_PIPELINES_EXT) {\n");
                out.append("        VkIndirectExecutionSetPipelineInfoEXT* p = new VkIndirectExecutionSetPipelineInfoEXT();\n");
                out.append("        s->info.pPipelineInfo = p;\n");
                out.append("        MarshalVkIndirectExecutionSetPipelineInfoEXT::read(pBoxedInfo, memory, address, p);\n");
                out.append("    } else if (s->type == VK_INDIRECT_EXECUTION_SET_INFO_TYPE_SHADER_OBJECTS_EXT) {\n");
                out.append("        VkIndirectExecutionSetShaderInfoEXT* p = new VkIndirectExecutionSetShaderInfoEXT();\n");
                out.append("        s->info.pShaderInfo = p;\n");
                out.append("        MarshalVkIndirectExecutionSetShaderInfoEXT::read(pBoxedInfo, memory, address, p);\n");
                out.append("    } else {\n");
                out.append("        kpanic_fmt(\"MarshalVkIndirectExecutionSetCreateInfoEXT::read unknown s->type %d\", s->type);\n");
                out.append("    }\n");
                out.append("    address += 4;\n");
            } else if (t.name.equals("VkIndirectCommandsLayoutTokenEXT") && param.name.equals("data")) {
                out.append("    paramAddress = memory->readd(address); address += 4;\n");
                out.append("    if (!paramAddress) {\n");
                out.append("        s->data.pPushConstant = nullptr;\n");
                out.append("    } else if (s->type == VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_CONSTANT_EXT || s->type == VK_INDIRECT_COMMANDS_TOKEN_TYPE_SEQUENCE_INDEX_EXT) {\n");
                out.append("        VkIndirectCommandsPushConstantTokenEXT* p = new VkIndirectCommandsPushConstantTokenEXT();\n");
                out.append("        memory->memcpy(&p->updateRange, paramAddress, 12);\n");
                out.append("        s->data.pPushConstant = p;\n");
                out.append("    } else if (s->type == VK_INDIRECT_COMMANDS_TOKEN_TYPE_VERTEX_BUFFER_EXT) {\n");
                out.append("        VkIndirectCommandsVertexBufferTokenEXT* p = new VkIndirectCommandsVertexBufferTokenEXT();\n");
                out.append("        p->vertexBindingUnit = memory->readd(paramAddress);\n");
                out.append("        s->data.pVertexBuffer = p;\n");
                out.append("    } else if (s->type == VK_INDIRECT_COMMANDS_TOKEN_TYPE_INDEX_BUFFER_EXT) {\n");
                out.append("        VkIndirectCommandsIndexBufferTokenEXT* p = new VkIndirectCommandsIndexBufferTokenEXT();\n");
                out.append("        p->mode = (VkIndirectCommandsInputModeFlagBitsEXT)memory->readd(paramAddress);\n");
                out.append("        s->data.pIndexBuffer = p;\n");
                out.append("    } else if (s->type == VK_INDIRECT_COMMANDS_TOKEN_TYPE_EXECUTION_SET_EXT) {\n");
                out.append("        VkIndirectCommandsExecutionSetTokenEXT* p = new VkIndirectCommandsExecutionSetTokenEXT();\n");
                out.append("        p->type = (VkIndirectExecutionSetInfoTypeEXT)memory->readd(paramAddress);\n");
                out.append("        p->shaderStages = memory->readd(paramAddress + 4);\n");
                out.append("        s->data.pExecutionSet = p;\n");
                out.append("    } else {\n");
                out.append("        // ignored\n");
                out.append("    }\n");
                out.append("    address += 4;\n");
            } else if (t.name.equals("VkDescriptorGetInfoEXT") && param.name.equals("data")) {
                out.append("    paramAddress = memory->readd(address); address += 8; // sizeof(VkDescriptorDataEXT) == 8\n");
                out.append("    if (s->type == VK_DESCRIPTOR_TYPE_SAMPLER) {\n");
                out.append("        VkSampler* p = new VkSampler();\n");
                out.append("        *p = (VkSampler)memory->readq(paramAddress);\n");
                out.append("        s->data.pSampler = p;\n");
                out.append("    } else if (s->type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {\n");
                out.append("        VkDescriptorImageInfo* p = new VkDescriptorImageInfo();\n");
                out.append("        p->sampler = (VkSampler)memory->readq(paramAddress);\n");
                out.append("        p->imageView = (VkImageView)memory->readq(paramAddress + 8);\n");
                out.append("        p->imageLayout = (VkImageLayout)memory->readd(paramAddress + 16);\n");
                out.append("        s->data.pCombinedImageSampler = p;\n");
                out.append("    } else if (s->type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT) {\n");
                out.append("        VkDescriptorImageInfo* p = new VkDescriptorImageInfo();\n");
                out.append("        p->sampler = (VkSampler)memory->readq(paramAddress);\n");
                out.append("        p->imageView = (VkImageView)memory->readq(paramAddress + 8);\n");
                out.append("        p->imageLayout = (VkImageLayout)memory->readd(paramAddress + 16);\n");
                out.append("        s->data.pInputAttachmentImage = p;\n");
                out.append("    } else if (s->type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE) {\n");
                out.append("        VkDescriptorImageInfo* p = new VkDescriptorImageInfo();\n");
                out.append("        p->sampler = (VkSampler)memory->readq(paramAddress);\n");
                out.append("        p->imageView = (VkImageView)memory->readq(paramAddress + 8);\n");
                out.append("        p->imageLayout = (VkImageLayout)memory->readd(paramAddress + 16);\n");
                out.append("        s->data.pSampledImage = p;\n");
                out.append("    } else if (s->type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) {\n");
                out.append("        VkDescriptorImageInfo* p = new VkDescriptorImageInfo();\n");
                out.append("        p->sampler = (VkSampler)memory->readq(paramAddress);\n");
                out.append("        p->imageView = (VkImageView)memory->readq(paramAddress + 8);\n");
                out.append("        p->imageLayout = (VkImageLayout)memory->readd(paramAddress + 16);\n");
                out.append("        s->data.pStorageImage = p;\n");
                out.append("    } else if (s->type == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER) {\n");
                out.append("        VkDescriptorAddressInfoEXT* p = new VkDescriptorAddressInfoEXT();\n");
                out.append("        MarshalVkDescriptorAddressInfoEXT::read(pBoxedInfo, memory, paramAddress, p);\n");
                out.append("        s->data.pUniformTexelBuffer = p;\n");
                out.append("    } else if (s->type == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER) {\n");
                out.append("        VkDescriptorAddressInfoEXT* p = new VkDescriptorAddressInfoEXT();\n");
                out.append("        MarshalVkDescriptorAddressInfoEXT::read(pBoxedInfo, memory, paramAddress, p);\n");
                out.append("        s->data.pStorageTexelBuffer = p;\n");
                out.append("    } else if (s->type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {\n");
                out.append("        VkDescriptorAddressInfoEXT* p = new VkDescriptorAddressInfoEXT();\n");
                out.append("        MarshalVkDescriptorAddressInfoEXT::read(pBoxedInfo, memory, paramAddress, p);\n");
                out.append("        s->data.pUniformBuffer = p;\n");
                out.append("    } else if (s->type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) {\n");
                out.append("        VkDescriptorAddressInfoEXT* p = new VkDescriptorAddressInfoEXT();\n");
                out.append("        MarshalVkDescriptorAddressInfoEXT::read(pBoxedInfo, memory, paramAddress, p);\n");
                out.append("        s->data.pStorageBuffer = p;\n");
                out.append("    } else {\n");
                out.append("        kpanic_fmt(\"MarshalVkDescriptorGetInfoEXT::read unknown s->type %d\", s->type);\n");
                out.append("    }\n");
                out.append("    address += 8;\n");
            } else if (t.name.equals("VkDebugReportCallbackCreateInfoEXT") && param.name.equals("pfnCallback")) {
                out.append("    s->pfnCallback = boxed_vkDebugReportCallbackEXT;\n");
                out.append("    MarshalCallbackData* pData = new MarshalCallbackData();\n");
                out.append("    pData->callbackAddress = memory->readd(address); address += 4;\n");
                out.append("    pData->userData = memory->readd(address);address+=4;\n");
                out.append("    s->pUserData = pData;\n");
                break; // pfnCallback and pUserData were handled
            } else if (t.name.equals("VkDebugUtilsMessengerCreateInfoEXT") && param.name.equals("pfnUserCallback")) {
                out.append("    s->pfnUserCallback = boxed_vkDebugUtilsMessengerCallbackEXT;\n");
                out.append("    MarshalCallbackData* pData = new MarshalCallbackData();\n");
                out.append("    pData->callbackAddress = memory->readd(address); address += 4;\n");
                out.append("    pData->userData = memory->readd(address);address+=4;\n");
                out.append("    s->pUserData = pData;\n");
                break; // pfnCallback and pUserData were handled
            } else if ((t.name.equals("VkMemoryToImageCopy") || t.name.equals("VkImageToMemoryCopy")) && param.name.equals("pHostPointer")) {
                out.append("    paramAddress = memory->readd(address);address+=4;\n");
            } else if (param.isPointer) {
                marshalInPointer(data, param, out, pointerData, paramsData);
            } else {
                marshalInParam(data, param, out, paramsData);
            }
            offset += param.getSize();
        }
        for (VkParam param : t.members) {
            if (t.name.equals("VkMemoryToImageCopy") && param.name.equals("pHostPointer")) {
                out.append("    if (paramAddress == 0) {\n");
                out.append("        s->pHostPointer = NULL;\n");
                out.append("    } else {\n");
                out.append("        uint64_t copy_size;\n");
                out.append("        const uint32_t element_size = vkuFormatElementSize(pBoxedInfo->imageCreateInfo[(U64)image]->s.format);\n");
                out.append("        if (s->memoryRowLength != 0 && s->memoryImageHeight != 0) {\n");
                out.append("            copy_size = ((s->memoryRowLength * s->memoryImageHeight) * element_size);\n");
                out.append("        } else {\n");
                out.append("            copy_size = ((s->imageExtent.width * s->imageExtent.height * s->imageExtent.depth) * element_size);\n");
                out.append("        }\n");
                out.append("        s->pHostPointer = memory->lockReadOnlyMemory(paramAddress, (U32)copy_size);\n");
                out.append("    }\n");
            } else if (t.name.equals("VkImageToMemoryCopy") && param.name.equals("pHostPointer")) {
                out.append("    if (paramAddress == 0) {\n");
                out.append("        s->pHostPointer = NULL;\n");
                out.append("    } else {\n");
                out.append("        uint64_t copy_size;\n");
                out.append("        const uint32_t element_size = vkuFormatElementSize(pBoxedInfo->imageCreateInfo[(U64)image]->s.format);\n");
                out.append("        if (s->memoryRowLength != 0 && s->memoryImageHeight != 0) {\n");
                out.append("            copy_size = ((s->memoryRowLength * s->memoryImageHeight) * element_size);\n");
                out.append("        } else {\n");
                out.append("            copy_size = ((s->imageExtent.width * s->imageExtent.height * s->imageExtent.depth) * element_size);\n");
                out.append("        }\n");
                out.append("        s->pHostPointer = new char[copy_size];\n");
                out.append("        kpanic(\"need to sync image back to emulated memory\");\n");
                out.append("    }\n");
            }
        }
        out.append("}\n");
    }

    private static void marshalOutPointer(VkData data, VkParam param, StringBuilder out, PointerData pdata, Vector<MarshalParamData> paramData) {
        if (pdata.createdParamAddress) {
            out.append("    paramAddress = memory->readd(address);address+=4;\n");
        } else {
            out.append("    U32 paramAddress = memory->readd(address);address+=4;\n");
            pdata.createdParamAddress = true;
        }
        out.append("    if (paramAddress != 0) {\n");
        if (param.name.equals("pNext")) {
            out.append("        vulkanWriteNextPtr(pBoxedInfo, memory, paramAddress, s->pNext);\n");
        } else if (param.paramType.getCategory().equals("struct")) {
            param.paramType.needMarshalOut = true;
            out.append("        Marshal");
            out.append(param.paramType.name);
            out.append("::write(pBoxedInfo, memory, paramAddress, s->");
            out.append(param.name);
            out.append(");\n");
        } else if (param.paramType.getCategory().equals("enum")) {
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
            size += " * sizeof(";
            size += param.paramType.name;
            size += ")";
            out.append("        memory->memcpy((");
            out.append(param.paramType.name);
            out.append("*)s->");
            out.append(param.name);
            out.append(", paramAddress, ");
            out.append(size);
            out.append(");\n");
        } else {
            out.append("        kpanic(\"Can't marshal void*\");\n");
        }
        out.append("    }\n");
    }
    private static void marshalOutParam(VkData data, VkType type, VkParam param, StringBuilder out) throws Exception {
        int width = param.getSize();

        if (type.name.equals("VkPerformanceValueINTEL") && param.name.equals("data")) {
            out.append("    switch (s->type) {\n");
            out.append("    case VK_PERFORMANCE_VALUE_TYPE_UINT32_INTEL:\n");
            out.append("        memory->writed(address, s->data.value32);\n");
            out.append("        break;\n");
            out.append("    case VK_PERFORMANCE_VALUE_TYPE_UINT64_INTEL:\n");
            out.append("        memory->writeq(address, s->data.value64);\n");
            out.append("        break;\n");
            out.append("    case VK_PERFORMANCE_VALUE_TYPE_FLOAT_INTEL:\n");
            out.append("        int2Float i2f;\n");
            out.append("        i2f.f = s->data.valueFloat;\n");
            out.append("        memory->writed(address, i2f.i);\n");
            out.append("        break;\n");
            out.append("    case VK_PERFORMANCE_VALUE_TYPE_BOOL_INTEL:\n");
            out.append("        memory->writed(address, s->data.valueBool);\n");
            out.append("        break;\n");
            out.append("    case VK_PERFORMANCE_VALUE_TYPE_STRING_INTEL:\n");
            out.append("        kpanic(\"MarshalVkPerformanceValueINTEL::write unhandled types: VK_PERFORMANCE_VALUE_TYPE_STRING_INTEL\");\n");
            out.append("        break;\n");
            out.append("    default:\n");
            out.append("        kpanic_fmt(\"MarshalVkPerformanceValueINTEL::write unknown s->type %d\", s->type);\n");
            out.append("    }\n");
            out.append("    address += 8;\n");
            return;
        }
        if (type.name.equals("VkIndirectExecutionSetCreateInfoEXT") && param.name.equals("info")) {
            out.append("    if (s->type == VK_INDIRECT_EXECUTION_SET_INFO_TYPE_PIPELINES_EXT) {\n");
            out.append("        MarshalVkIndirectExecutionSetPipelineInfoEXT::write(pBoxedInfo, memory, address, (VkIndirectExecutionSetPipelineInfoEXT*)s->info.pPipelineInfo);\n");
            out.append("    } else if (s->type == VK_INDIRECT_EXECUTION_SET_INFO_TYPE_SHADER_OBJECTS_EXT) {\n");
            out.append("        MarshalVkIndirectExecutionSetShaderInfoEXT::write(pBoxedInfo, memory, address, (VkIndirectExecutionSetShaderInfoEXT*)s->info.pShaderInfo);\n");
            out.append("    } else {\n");
            out.append("        kpanic_fmt(\"MarshalVkIndirectExecutionSetCreateInfoEXT::write unknown s->type %d\", s->type);\n");
            out.append("    }\n");
            return;
        }
        if (param.paramType.getCategory().equals("struct") && needsMarshaling(param.paramType)) {
            param.paramType.needMarshalOut = true;
            if (param.arrayLen > 0) {
                out.append("    for (U32 i = 0; i < ");
                out.append(param.arrayLen);
                out.append("; i++) {\n");
                out.append("        Marshal");
                out.append(param.paramType.name);
                out.append("::write(pBoxedInfo, memory, address, &s->");
                out.append(param.name);
                out.append("[i]); address+=");
                out.append(param.paramType.sizeof);
                out.append(";\n");
                out.append("    }\n");
            } else {
                out.append("    Marshal");
                out.append(param.paramType.name);
                out.append("::write(pBoxedInfo, memory, address, &s->");
                out.append(param.name);
                out.append("); address+=");
                out.append(param.paramType.sizeof);
                out.append(";\n");
            }
        } else if (param.paramType.getType().equals("VK_DEFINE_HANDLE")) {
            if (param.isArray()) {
                out.append("    paramAddress = memory->readd(address);address+=4;\n");
                out.append("    if (paramAddress) {\n");
                out.append("        for (U32 i=0;i<");
                if (param.arrayLen > 0) {
                    out.append(param.arrayLen);
                } else {
                    out.append(param.len);
                }
                out.append(";i++) {\n            memory->writed(paramAddress + i*4, createVulkanPtr(memory, s->");
                out.append(param.name);
                out.append("[i], pBoxedInfo));\n        }\n");
                out.append("    }\n");
            } else {
                out.append("    memory->writed(address, createVulkanPtr(memory, s->");
                out.append(param.name);
                out.append(", ");
                out.append("pBoxedInfo");
                out.append(")); address += 4;\n");
            }
        } else if (width > 8 || param.arrayLen > 0 || param.paramType.getCategory().equals("struct") || param.paramType.getType().equals("union")) {
            if (!simpleTypes.contains(param.paramType.name)) {
                out.append("    static_assert(sizeof(");
                out.append(param.paramType.name);
                out.append(") == ");
                if (param.arrayLen > 0) {
                    out.append(param.paramType.sizeof);
                } else {
                    out.append(width);
                }
                out.append(", \"false\");\n");
            }
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
                if (param.paramType.getType().equals("VK_DEFINE_NON_DISPATCHABLE_HANDLE")) {
                    out.append("(U64)");
                } else if (param.paramType.getCategory().equals("union")) {
                    for (VkParam member : param.paramType.members) {
                        if (member.paramType.sizeof == 8) {
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
                if (param.paramType.name.equals("size_t") || param.paramType.name.equals("void") || param.paramType.getCategory().isEmpty()) {
                    // size_t is 32-bit on win32, but host might be 64-bit, this will remove the warning
                    if (param.paramType.getCategory().isEmpty()) {
                        out.append("d(address, *(U32*)&s->");
                    } else {
                        out.append("d(address, (U32)s->");
                    }
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
    private static void marshalOut(VkData data, VkType t, StringBuilder out, Vector<MarshalParamData> paramsData) throws Exception {
        out.append("void Marshal");
        out.append(t.name);
        out.append("::write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, const ");
        out.append(t.name);
        out.append("* s");
        out.append(") {\n");
        if (data.stubMarshalWrite.contains(t.name)) {
            out.append("    kpanic(\"Marshal");
            out.append(t.name);
            out.append("::write\");\n");
            out.append("}\n");
            return;
        }
        PointerData pointerData = new PointerData();
        int offset = 0;
        for (VkParam param : t.members) {
            int alignment = param.getAlignment();
            if ((offset % alignment) != 0) {
                out.append("    address+=");
                out.append(alignment - (offset % alignment));
                out.append("; // structure padding\n");
                offset += alignment - (offset % alignment);
            }if (param.paramType.name.equals("VkPhysicalDevice") && param.name.equals("physicalDevices")) {
                int ii=0;
            }
            if (param.isPointer) {
                marshalOutPointer(data, param, out, pointerData, paramsData);
            } else {
                marshalOutParam(data, t, param, out);
            }
            offset += param.getSize();
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

        if (t.name.equals("VkPerformanceValueINTEL")) {
            out.append("    if (s.type == VK_PERFORMANCE_VALUE_TYPE_STRING_INTEL) {\n");
            out.append("        delete[] s.data.valueString;\n");
            out.append("    }\n");
            out.append("}\n");
            return;
        }
        for (MarshalParamData data : paramsData) {
            if (alreadyAdded.contains(data.name)) {
                continue;
            }
            if (data.unlock) {
                out.append("    KThread::currentThread()->memory->unlockMemory((U8*)");
                out.append(data.name);
                out.append(");\n");
                continue;
            }
            if (data.deleteItems) {
                out.append("    if (");
                out.append(data.name);
                out.append(") {\n");
                out.append("        for (U32 i = 0; i < ");
                out.append(data.itemCount);
                out.append("; i++) {\n");
                out.append("            delete ");
                out.append(data.name);
                out.append("[i];\n        }\n    }\n");
            }
            out.append("    delete");
            if (data.isArray) {
                out.append("[]");
            }
            out.append(" ");
            if (data.isVoid) {
                out.append("(char*)");
            }
            if (data.name.equals("s.pNext")) {
                out.append("(VkBaseOutStructure*)");
            }
            out.append(data.name);
            out.append(";\n");
            alreadyAdded.add(data.name);
        }
        if (t.name.equals("VkIndirectExecutionSetCreateInfoEXT")) {
            out.append("    if (s.type == VK_INDIRECT_EXECUTION_SET_INFO_TYPE_PIPELINES_EXT) {\n");
            out.append("        delete s.info.pPipelineInfo;\n");
            out.append("    } else if (s.type == VK_INDIRECT_EXECUTION_SET_INFO_TYPE_SHADER_OBJECTS_EXT) {\n");
            out.append("        delete s.info.pShaderInfo;\n");
            out.append("    } else {\n");
            out.append("        kpanic_fmt(\"MarshalVkIndirectExecutionSetCreateInfoEXT::MarshalVkIndirectExecutionSetCreateInfoEXT unknown s.type %d\", s.type);\n");
            out.append("    }\n");
        }
        if (t.name.equals("VkIndirectCommandsLayoutTokenEXT")) {
            out.append("    if (s.type == VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_CONSTANT_EXT || s.type == VK_INDIRECT_COMMANDS_TOKEN_TYPE_SEQUENCE_INDEX_EXT) {\n");
            out.append("        delete s.data.pPushConstant;\n");
            out.append("    } else if (s.type == VK_INDIRECT_COMMANDS_TOKEN_TYPE_VERTEX_BUFFER_EXT) {\n");
            out.append("        delete s.data.pVertexBuffer;\n");
            out.append("    } else if (s.type == VK_INDIRECT_COMMANDS_TOKEN_TYPE_INDEX_BUFFER_EXT) {\n");
            out.append("        delete s.data.pIndexBuffer;\n");
            out.append("    } else if (s.type == VK_INDIRECT_COMMANDS_TOKEN_TYPE_EXECUTION_SET_EXT) {\n");
            out.append("        delete s.data.pExecutionSet;\n");
            out.append("    }\n");
        }
        if (t.name.equals("VkDescriptorGetInfoEXT")) {
            out.append("    if (s.type == VK_DESCRIPTOR_TYPE_SAMPLER) {\n");
            out.append("        delete s.data.pSampler;\n");
            out.append("    } else if (s.type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {\n");
            out.append("        delete s.data.pCombinedImageSampler;\n");
            out.append("    } else if (s.type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT) {\n");
            out.append("        delete s.data.pInputAttachmentImage;\n");
            out.append("    } else if (s.type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE) {\n");
            out.append("        delete s.data.pSampledImage;\n");
            out.append("    } else if (s.type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) {\n");
            out.append("        delete s.data.pStorageImage;\n");
            out.append("    } else if (s.type == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER) {\n");
            out.append("        delete s.data.pUniformTexelBuffer;\n");
            out.append("    } else if (s.type == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER) {\n");
            out.append("        delete s.data.pStorageTexelBuffer;\n");
            out.append("    } else if (s.type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {\n");
            out.append("        delete s.data.pUniformBuffer;\n");
            out.append("    } else if (s.type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) {\n");
            out.append("        delete s.data.pStorageBuffer;\n");
            out.append("    } else {\n");
            out.append("        kpanic_fmt(\"MarshalVkDescriptorGetInfoEXT::~MarshalVkDescriptorGetInfoEXT unknown s.type %d\", s.type);\n");
            out.append("    }\n");
        }
        out.append("}\n");
    }
    public static void write(VkData data, VkType t, StringBuilder out) throws Exception {
        Vector<MarshalParamData> paramsData = new Vector<>();

        if (t.isNeedMarshalIn()) {
            marshalIn(data, t, out, paramsData);
        }
        if (t.needMarshalOut) {
            marshalOut(data, t, out, paramsData);
        }
        if (paramsData.size() > 0) {
            t.needDestructor = true;
            marshalDestructor(t, out, paramsData);
        }
    }

    static boolean needsMarshaling(VkType type) {
        if (type.name.equals("VkClearValue")) {
            return false;
        }
        if (type.getCategory().equals("struct")) {
            // need to be careful of struct padding
            return true;
        }
        if (type.getCategory().equals("enum")) {
            // size if not guaranteed in C99, could be char, could be int
            return true;
        }
        if (type.getType().equals("size_t")) {
            return true;
        }
        if (type.getType().equals("VK_DEFINE_HANDLE")) {
            return true;
        }
        if (type.getType().equals("VK_DEFINE_NON_DISPATCHABLE_HANDLE")) {
            return false;
        }

        if (type.members.size() > 0) {
            for (VkParam param : type.members) {
                if (needsMarshaling(param.paramType)) {
                    return true;
                }
            }
            return false;
        }
        if (simpleTypes.contains(type.name)) {
            return false;
        }
        if (simpleTypes.contains(type.getType())) {
            return false;
        }
        return true;
    }

    static VkHostMarshalInOut inOut = new VkHostMarshalInOut();
    static VkHostMarshalOutHandleArray outHandleArray = new VkHostMarshalOutHandleArray();
    static VkHostMarshalOutData outData = new VkHostMarshalOutData();
    static VkHostMarshalOutStructure outStructure = new VkHostMarshalOutStructure();
    static VkHostMarshalOutHandle outHandle = new VkHostMarshalOutHandle();
    static VkHostMarshalInMemory inMemory = new VkHostMarshalInMemory();
    static VkHostMarshalInStructure inStructure = new VkHostMarshalInStructure();
    static VkHostMarshalInHandleArray inHandleArray = new VkHostMarshalInHandleArray();
    static VkHostMarshalOutEnumArray outEnumArray = new VkHostMarshalOutEnumArray();
    static VkHostMarshalInStructureArray inStructureArray = new VkHostMarshalInStructureArray();
    static VkHostMarshalOutStructureArray outStructureArray = new VkHostMarshalOutStructureArray();
    static VkHostMarshalOutEnum outEnum = new VkHostMarshalOutEnum();
    static VkHostMarshalInEnumArray inEnumArray = new VkHostMarshalInEnumArray();
    static VkHostMarshalInHandle inHandle = new VkHostMarshalInHandle();
    static VkHostMarshalNone none = new VkHostMarshalNone();
    static VkHostMarshalInUnion inUnion = new VkHostMarshalInUnion();
    static vkHostMarshalNotImplemented notImplemented = new vkHostMarshalNotImplemented();
    static VkUpdateDescriptorSetWithTemplateMarshal updateDescriptorSetWithTemplateMarshal = new VkUpdateDescriptorSetWithTemplateMarshal();
    private static HashSet<String> simpleTypes;

    static {
        simpleTypes = new HashSet<>();
        simpleTypes.add("int");
        simpleTypes.add("uint8_t");
        simpleTypes.add("uint16_t");
        simpleTypes.add("uint32_t");
        simpleTypes.add("uint64_t");
        simpleTypes.add("char");
        simpleTypes.add("VkFlags64");
        simpleTypes.add("VkFlags");
        simpleTypes.add("float");
        simpleTypes.add("int8_t");
        simpleTypes.add("int16_t");
        simpleTypes.add("int32_t");
        simpleTypes.add("int64_t");
        simpleTypes.add("VK_DEFINE_NON_DISPATCHABLE_HANDLE");
        simpleTypes.add("Display");
        simpleTypes.add("Window");
        simpleTypes.add("VisualID");
        simpleTypes.add("size_t");
    }

    public static VkHostMarshal getMarshal(VkFunction function, VkParam param) throws Exception {
        if (function.name.equals("vkMapMemory") && param.name.equals("ppData")) {
            return new VkHostMarshalMapMemory();
        }
        if ((function.name.equals("vkMapMemory2") || function.name.equals("vkMapMemory2KHR")) && param.name.contains("pData")) {
            return new VkHostMarshalMapMemory2();
        }
        if (param.paramType.getCategory().equals("struct")) {
            if (param.isConst) {
                if (param.len != null || param.arrayLen > 0) {
                    return inStructureArray;
                }
                return inStructure;
            }
            if (param.len != null || param.arrayLen > 0) {
                return outStructureArray;
            }
            return outStructure;
        }
        if (param.paramType.getCategory().equals("union")) {
            int sizeof = param.paramType.members.get(0).sizeof;
            for (VkParam member : param.paramType.members) {
                if (member.sizeof != sizeof || !simpleTypes.contains(member.paramType.getType())) {
                    throw new Exception("Unhandled param type: " + function.name + ":" + param.name);
                }
            }
            if (param.isConst) {
                if (param.len != null || param.arrayLen > 0) {
                    throw new Exception("Unhandled param type: " + function.name + ":" + param.name);
                }
                return inUnion;
            }
            throw new Exception("Unhandled param type: " + function.name + ":" + param.name);
        }
        if (param.paramType.getType().equals("VK_DEFINE_HANDLE")) {
            if (param.isConst) {
                if (param.len != null || param.arrayLen != 0) {
                    return inHandleArray;
                } else {
                    return inHandle;
                }
            }
            if (param.len != null || param.arrayLen != 0) {
                return outHandleArray;
            }
            if (param.isPointer) {
                return outHandle;
            }
            return inHandle;
        }
        if (param.paramType.getCategory().equals("enum")) {
            if (!param.isPointer && param.len == null && param.arrayLen == 0) {
                return none;
            }
            if (param.isConst) {
                if (param.len != null || param.arrayLen != 0) {
                    return inEnumArray;
                }
                throw new Exception("Unhandled param type: " + function.name + ":" + param.name);
            }
            if (param.len != null || param.arrayLen != 0) {
                return outEnumArray;
            }
            return inOut;
        }
        if (simpleTypes.contains(param.paramType.name) || simpleTypes.contains(param.paramType.getType())) {
            if (!param.isPointer && param.len == null && param.arrayLen == 0) {
                return none;
            }
            if (param.isConst) {
                if (param.len != null || param.arrayLen != 0) {
                    return inMemory;
                }
                throw new Exception("Unhandled param type: " + function.name + ":" + param.name);
            }
            return inOut;
        }
        if (param.paramType.getType().equals("size_t")) {
            if (param.isPointer && (param.len != null || param.arrayLen != 0)) {
                throw new Exception("Unhandled param type: " + function.name + ":" + param.name);
            }
            return none;
        }
        if (param.paramType.name.equals("void")) {
            if (!param.isPointer) {
                throw new Exception("Unhandled param type: " + function.name + ":" + param.name);
            }
            if ((function.name.equals("vkUpdateDescriptorSetWithTemplateKHR") || function.name.equals("vkUpdateDescriptorSetWithTemplate")) && param.name.equals("pData")) {
                return updateDescriptorSetWithTemplateMarshal;
            }
            if (param.len == null && param.arrayLen == 0) {
                return notImplemented;
            }
            return inMemory;
        }
        int ii=0;
        /*
        if ((function.name.equals("vkUpdateDescriptorSetWithTemplate") || function.name.equals("vkUpdateDescriptorSetWithTemplateHKR") || function.name.equals("vkCmdPushDescriptorSetWithTemplateKHR")) && param.name.equals("pData")) {
            return updateDescriptorSetWithTemplate;
        } else if (function.name.equals("vkGetMemoryHostPointerPropertiesEXT") && param.name.equals("pHostPointer")) {
            return updateDescriptorSetWithTemplate;
        } else if (!param.isPointer && param.arrayLen == 0) {
            if (param.paramType.type.equals("VK_DEFINE_HANDLE")) {
                return inHandle;
            } else {
                return none;
            }
        } else {
            if (param.isConst) {
                if (param.len != null || param.arrayLen != 0) {
                    if (!needsMarshaling(param.paramType) || param.paramType.type.equals("void")) {
                        return inMemory;
                    } else if (param.paramType.type.equals("VK_DEFINE_HANDLE")) {
                        return inHandleArray;
                    } else if (param.paramType.category.equals("struct") || param.paramType.category.equals("union")){
                        return inStructureArray;
                    } else if (param.paramType.category.equals("enum")){
                        return inEnumArray;
                    } else {
                        throw new Exception("Unhandled param type: " + function.name + ":" + param.name);
                    }
                } else if ((param.paramType.category.equals("struct") || param.paramType.category.equals("union")) && needsMarshaling(param.paramType)) {
                    return inStructure;
                } else if (!needsMarshaling(param.paramType) || param.paramType.type.equals("void")) {
                    return inMemory;
                } else if (param.paramType.type.equals("")) {
                    System.out.println("Unhandled param type: " + function.name + ":" + param.full);
                } else {
                    throw new Exception("Unhandled param type: " + function.name + ":" + param.name);
                }
            } else {
                boolean isCountParam = false;
                for (VkParam p : function.params) {
                    if (p.len != null && p.len.equals(param.name)) {
                        isCountParam = true;
                    }
                }
                if (function.name.equals("vkMapMemory") && param.name.equals("ppData")) {
                    return new VkHostMarshalMapMemory();
                } else if (function.name.equals("vkMapMemory2") && param.name.equals("ppData")) {
                    return new VkHostMarshalMapMemory2();
                } else if (isCountParam) {
                    return inOut;
                } else if (param.len != null || param.arrayLen != 0) {
                    if (!needsMarshaling(param.paramType) || param.paramType.type.equals("void")) {
                        return inMemory;
                    } else if (param.paramType.type.equals("VK_DEFINE_HANDLE")) {
                        return outHandleArray;
                    } else if (param.paramType.category.equals("struct") || param.paramType.category.equals("union")){
                        return outStructureArray;
                    } else if (param.len != null && param.paramType.category.equals("enum")) {
                        return outEnumArray;
                    } else {
                        throw new Exception("Unhandled param type: " + function.name + ":" + param.name);
                    }
                } else if (!needsMarshaling(param.paramType) || (param.len != null && param.paramType.type.equals("void"))) {
                    return outData;
                } else if (param.paramType.type.equals("VK_DEFINE_HANDLE")) {
                    return outHandle;
                } else if (param.paramType.category.equals("struct")) {
                    return outStructure;
                } else if (param.paramType.category.equals("enum")) {
                    //param.marshal = outEnum;
                    return inOut;
                } else if (param.paramType.type.equals("")) {
                    System.out.println("Unhandled param type: " + function.name + ":" + param.full);
                } else {
                    throw new Exception("Unhandled param type: " + function.name + ":" + param.name);
                }
            }
        }
         */
        return null;
    }
}
