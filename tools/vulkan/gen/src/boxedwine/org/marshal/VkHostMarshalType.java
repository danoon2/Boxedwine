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
            out.append("    Marshal");
            out.append(t.name);
            out.append("(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}\n");

            out.append("    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, ");
            out.append(t.name);
            out.append("* s");
            out.append(");\n");
        }

        if (t.needMarshalOut) {
            out.append("    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, ");
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
                out.append(":TODO: 3\n");
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
            size += (" * sizeof(");
            size += param.paramType.name;
            size += ")";
            if (needsMarshaling(param.paramType)) {
                throw new Exception();
            }
        }
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
        out.append("::read(pBoxedInfo, memory, paramAddress + i");
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
        param.paramType.setNeedMarshalIn(true);
    }

    private static void marshalInPointerToStruct(VkParam param, StringBuilder out, Vector<MarshalParamData> paramData) {
        param.paramType.setNeedMarshalIn(true);
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
            out.append("        s->pNext = vulkanGetNextPtr(pBoxedInfo, memory, paramAddress);\n");
            paramData.add(new MarshalParamData("s.pNext", false));
        } else if (param.isArray()) {
            if (param.isDoublePointer) {
                marshalInArrayOfPointers(param, out, paramData);
            } else if (param.paramType.getType().equals("VK_DEFINE_HANDLE")) {
                marshalInArrayOfHandles(param, out, paramData);
            } else if (!needsMarshaling(param.paramType) || (param.isPointer && !param.isDoublePointer && param.paramType.getType().equals("void")) || param.paramType.getCategory().equals("enum")) {
                marshalInArrayOfData(param, out, paramData);
            } else if (param.paramType.getCategory().equals("struct")) {
                marshalInArrayOfStructs(param, out, paramData);
            } else {
                throw new Exception("oops");
            }
        } else if (param.paramType.getCategory().equals("struct")) {
            marshalInPointerToStruct(param, out, paramData);
        } else {
            if (param.name.equals("pShaderGroupCaptureReplayHandle")) {
                out.append("        s->");
                out.append(param.name);
                out.append(" = pBoxedInfo->rayTracingCaptureReplayShaderGroupHandles[paramAddress];\n");
                out.append("        if (!s->pShaderGroupCaptureReplayHandle) {\n            kpanic(\"MarshalVkRayTracingShaderGroupCreateInfoKHR::read oops\");\n        }\n");
            } else {
                out.append(":TODO: 4\n");
            }
            //throw new Exception("oops");
        }
        out.append("    }\n");
    }

    private static void marshalInParam(VkParam param, StringBuilder out) throws Exception {
        if (param.paramType.getCategory().equals("struct") && needsMarshaling(param.paramType)) {
            param.paramType.setNeedMarshalIn(true);
            out.append("    Marshal");
            out.append(param.paramType.name);
            out.append("::read(pBoxedInfo, memory, address, &s->");
            out.append(param.name);
            out.append("); address+=");
            out.append(param.paramType.sizeof);
            out.append(";\n");
        } else if (param.arrayLen > 0) {
            if (param.paramType.getCategory().equals("struct")) {
                for (VkParam member : param.paramType.members) {
                    if (member.paramType.isNeedMarshalIn() || member.paramType.needMarshalOut) {
                        throw new Exception("oops");
                    }
                }
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
            } else if (param.paramType.getType().equals("VK_DEFINE_HANDLE")) {
                out.append("    s->");
                out.append(param.name);
                out.append(" = (");
                out.append(param.paramType.name);
                out.append(")getVulkanPtr(memory, memory->readd(address));address+=4;\n");
            } else {
                if (param.paramType.getType().equals("union")) {
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
                    out.append(" = (");
                    out.append(param.paramType.name);
                    if (param.paramType.name.equals("void")) {
                        out.append("*");
                    }
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

    private static void marshalIn(VkData data, VkType t, StringBuilder out, Vector<MarshalParamData> paramsData) throws Exception {
        out.append("void Marshal");
        out.append(t.name);
        out.append("::read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, ");
        out.append(t.name);
        out.append("* s");
        out.append(") {\n");
        PointerData pointerData = new PointerData();

        if (data.stubMarshalRead.contains(t.name)) {
            out.append("    kpanic(\"Marshal");
            out.append(t.name);
            out.append("::read\");\n");
            out.append("}\n");
            return;
        }
        int offset = 0;
        for (VkParam param : t.members) {
            if (param.name.equals("pQueuePriorities")) {
                int ii=0;
            }
            if (t.name.equals("VkDebugReportCallbackCreateInfoEXT") && param.name.equals("pfnCallback")) {
                out.append("    s->pfnCallback = boxed_vkDebugReportCallbackEXT;\n");
                out.append("    MarshalCallbackData* pData = new MarshalCallbackData();\n");
                out.append("    pData->callbackAddress = memory->readd(address); address += 4;\n");
                out.append("    pData->userData = memory->readd(address);address+=4;\n");
                out.append("    s->pUserData = pData;\n");
                break;
            }
            if (t.name.equals("VkDebugUtilsMessengerCreateInfoEXT") && param.name.equals("pfnUserCallback")) {
                out.append("    s->pfnUserCallback = boxed_vkDebugUtilsMessengerCallbackEXT;\n");
                out.append("    MarshalCallbackData* pData = new MarshalCallbackData();\n");
                out.append("    pData->callbackAddress = memory->readd(address); address += 4;\n");
                out.append("    pData->userData = memory->readd(address);address+=4;\n");
                out.append("    s->pUserData = pData;\n");
                break;
            }
            int alignment = param.getAlignment();
            if ((offset % alignment) != 0) {
                out.append("    address+=");
                out.append(alignment - (offset % alignment));
                out.append("; // structure padding\n");
                offset += alignment - (offset % alignment);
            }
            if (param.isPointer) {
                marshalInPointer(param, out, pointerData, paramsData);
            } else {
                marshalInParam(param, out);
            }
            offset += param.getSize();
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
            out.append("        vulkanWriteNextPtr(pBoxedInfo, memory, paramAddress, s->pNext);\n");
        } else if (param.paramType.getCategory().equals("struct")) {
            param.paramType.setNeedMarshalIn(true);
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
        } else {
            out.append("        kpanic(\"Can't marshal void*\");\n");
        }
        out.append("    }\n");
    }
    private static void marshalOutParam(VkType type, VkParam param, StringBuilder out) throws Exception {
        int width = param.getSize();

        if (type.name.equals("VkPipelineExecutableStatisticKHR") && param.name.equals("value")) {
            int ii=0;
        }
        if (width > 8 || param.arrayLen > 0 || param.paramType.getCategory().equals("struct") || param.paramType.getType().equals("union")) {
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
            if (width == 4 && param.paramType.getType().equals("VK_DEFINE_HANDLE")) {
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
                if (param.paramType.name.equals("size_t") || param.paramType.name.equals("void")) {
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
    private static void marshalOut(VkData data, VkType t, StringBuilder out, Vector<MarshalParamData> paramsData) throws Exception {
        out.append("void Marshal");
        out.append(t.name);
        out.append("::write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, ");
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
            }
            if (param.isPointer) {
                marshalOutPointer(param, out, pointerData, paramsData);
            } else {
                marshalOutParam(t, param, out);
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

        for (MarshalParamData data : paramsData) {
            if (alreadyAdded.contains(data.name)) {
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
            out.append(data.name);
            out.append(";\n");
            alreadyAdded.add(data.name);
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

        if (type.members != null) {
            for (VkParam member : type.members) {
                if (needsMarshaling(member.paramType)) {
                    return true;
                }
            }
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
    private static HashSet<String> simpleTypes;

    static {
        simpleTypes = new HashSet<>();
        simpleTypes.add("int");
        simpleTypes.add("uint16_t");
        simpleTypes.add("uint32_t");
        simpleTypes.add("uint64_t");
        simpleTypes.add("char");
        simpleTypes.add("VkFlags64");
        simpleTypes.add("VkFlags");
        simpleTypes.add("float");
        simpleTypes.add("int32_t");
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
