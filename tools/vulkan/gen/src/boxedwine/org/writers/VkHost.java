package boxedwine.org.writers;

import boxedwine.org.data.VkData;
import boxedwine.org.data.VkFunction;
import boxedwine.org.data.VkParam;
import boxedwine.org.data.VkType;
import boxedwine.org.marshal.VkHostMarshal;
import boxedwine.org.marshal.VkHostMarshalType;

import java.io.FileOutputStream;
import java.util.Hashtable;
import java.util.Vector;

/**
 * Created by James on 8/21/2021.
 */
public class VkHost {
    static public void write(VkData data, String dir) throws Exception {
        FileOutputStream fosPassThrough = new FileOutputStream(dir+"vk_host.cpp");
        FileOutputStream fosMarshal = new FileOutputStream(dir+"vk_host_marshal.cpp");
        FileOutputStream fosMarshalHeader = new FileOutputStream(dir+"vk_host_marshal.h");
        write(data, fosPassThrough, fosMarshal, fosMarshalHeader);
        fosPassThrough.close();
        fosMarshal.close();
        fosMarshalHeader.close();

        fosPassThrough = new FileOutputStream(dir+"vk_host.h");
        writeHeader(data, fosPassThrough, data.functions);
        fosPassThrough.close();
    }

    static private void hostCall(VkFunction fn, StringBuilder out) throws Exception {
        if (fn.name.equals("vkCreateDebugReportCallbackEXT")) {
            out.append("    pBoxedInfo->debugReportCallbacks[(U64)tmp_pCallback] = local_pCreateInfo.s.pUserData;\n");
        } else if (fn.name.equals("vkDestroyDebugReportCallbackEXT")) {
            out.append("    delete pBoxedInfo->debugReportCallbacks[(U64)callback];\n");
            out.append("    pBoxedInfo->debugReportCallbacks.erase((U64)callback);\n");
        } else if (fn.name.equals("vkCreateDebugUtilsMessengerEXT")) {
            out.append("    pBoxedInfo->debugUtilsCallbacks[(U64)tmp_pMessenger] = local_pCreateInfo.s.pUserData;\n");
        } else if (fn.name.equals("vkDestroyDebugUtilsMessengerEXT")) {
            out.append("    delete pBoxedInfo->debugUtilsCallbacks[(U64)messenger];\n");
            out.append("    pBoxedInfo->debugUtilsCallbacks.erase((U64)messenger);\n");
        }
        String pfn = "";
        if (fn.params.elementAt(0).paramType.getType().equals("VK_DEFINE_HANDLE")) {
            pfn += "pBoxedInfo->";
        }
        pfn += "p";
        pfn += fn.name;
        out.append("    ");
        if (fn.returnType.sizeof == 0) {
            out.append(pfn);
        } else if (fn.returnType.sizeof == 4) {
            out.append("EAX = (U32)");;
            out.append(pfn);
        } else if (fn.returnType.sizeof == 8) {
            out.append(fn.returnType.name);
            out.append(" result = ");
            out.append(pfn);
        } else {
            throw new Exception("Unhandled call return size: " + fn.returnType.sizeof);
        }
        boolean first = true;
        out.append("(");
        for (VkParam param : fn.params) {
            if (!first) {
                out.append(", ");
            }
            first = false;
            out.append(param.nameInFunction);
        }
        out.append(");\n");
        if (fn.returnType.sizeof == 8) {
            out.append("    EAX = (U32)result;\n");
            out.append("    EDX = (U32)(result >> 32);\n");
        }
    }

    static private void setCountParam(VkFunction fn, VkParam param) {
        if (param.len != null) {
            for (VkParam p : fn.params) {
                if (p.name.equals(param.len)) {
                    param.countParam = p;
                    return;
                }
                if (param.len.startsWith(p.name + "->")) {
                    param.countParam = p;
                    param.countInStructure = true;
                    return;
                }
            }
        }
    }

    static private void writeFunction(VkData data, VkFunction fn, StringBuilder out) throws Exception {
        int stackPos = 1;

        // start of function
        if (fn.returnType.sizeof != 0) {
            out.append("// return type: "+fn.returnType.name+"("+fn.returnType.sizeof+" bytes)\n");
        }
        out.append("void vk_" + fn.name.substring(2) + "(CPU* cpu) {\n");
        if (!fn.params.elementAt(0).paramType.getType().equals("VK_DEFINE_HANDLE")) {
            out.append("    initVulkan();\n");
        }/*
        out.append("    klog(\"");
        out.append(fn.name);
        out.append("\");\n");
        */

        Hashtable<String, VkHostMarshal> paramMarshals = new Hashtable<>();

        // marshal data if needed from emulated 32-bit linux to host system

        // find if a stride references another param
        VkParam strideParam = null;
        for (VkParam param : fn.params) {
            if (param.stride == null) {
                continue;
            }
            for (VkParam p : fn.params) {
                if (p.name.equals(param.stride)) {
                    strideParam = p;
                    break;
                }
            }
        }
        // if a stride does reference another param, lets write that param first so the param that uses the stride can use it
        for (VkParam param : fn.params) {
            if (param != strideParam) {
                stackPos++;
                continue;
            }
            param.nameInFunction = param.name;
            param.paramArg = "ARG" + String.valueOf(stackPos);
            if (param.isPointer) {
                setCountParam(fn, param);
            }
            VkHostMarshal marshal = VkHostMarshalType.getMarshal(fn, param);
            paramMarshals.put(param.paramArg , marshal);
            marshal.before(fn, out, param);
            if (stackPos == 1 && fn.params.elementAt(0).paramType.getType().equals("VK_DEFINE_HANDLE")) {
                out.append("    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(cpu->memory, ARG1);\n");
            }
            stackPos++;
        }
        stackPos = 1;
        if (fn.name.equals("vkEnumerateDeviceExtensionProperties")) {
            int ii=0;
        }
        for (VkParam param : fn.params) {
            if (param == strideParam) {
                stackPos++;
                continue;
            }
            param.nameInFunction = param.name;
            param.paramArg = "ARG" + String.valueOf(stackPos);
            if (param.isPointer) {
                setCountParam(fn, param);
            }
            VkHostMarshal marshal = VkHostMarshalType.getMarshal(fn, param);
            paramMarshals.put(param.paramArg , marshal);
            marshal.before(fn, out, param);
            if (stackPos == 1 && fn.params.elementAt(0).paramType.getType().equals("VK_DEFINE_HANDLE")) {
                out.append("    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(cpu->memory, ARG1);\n");
            }
            stackPos++;
        }
        // make actual vulkan call on host
        hostCall(fn, out);
        // marshal data if needed from host system back to emulated 32-bit linux
        for (VkParam param : fn.params) {
            paramMarshals.get(param.paramArg).after(fn, out, param);
        }
        out.append("}\n");
    }

    static private void writeHeader(VkData data, FileOutputStream fos, Vector<VkFunction> hostFunctions) throws Exception {
        StringBuilder out = new StringBuilder();
        out.append("#ifndef __VK_HOST__H__\n");
        out.append("#define __VK_HOST__H__\n");
        out.append("#define VK_NO_PROTOTYPES\n");
        out.append("#include \"vk/vulkan.h\"\n");
        out.append("#include \"vk/vulkan_core.h\"\n");
        out.append("#ifndef BOXED_VK_EXTERN\n");
        out.append("#define BOXED_VK_EXTERN extern\n");
        out.append("#endif\n");
        out.append("class BoxedVulkanInfo;\n");
        out.append("VkBaseOutStructure* vulkanGetNextPtr(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address);\n");
        out.append("void vulkanWriteNextPtr(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, const void* pNext);\n");
        out.append("void* getVulkanPtr(KMemory* memory, U32 address);\n");
        for (VkFunction fn : hostFunctions ) {
            if (!fn.params.elementAt(0).paramType.getType().equals("VK_DEFINE_HANDLE")) {
                out.append("BOXED_VK_EXTERN PFN_");
                out.append(fn.name);
                out.append(" p");
                out.append(fn.name);
                out.append(";\n");
            }
        }
        for (VkFunction fn : hostFunctions ) {
            out.append("void vk_");
            out.append(fn.name.substring(2));
            out.append("(CPU* cpu);\n");
        }
        out.append("class BoxedVulkanInfo {\npublic:\n");
        out.append("    VkInstance instance;\n");
        for (VkFunction fn : hostFunctions ) {
            if (data.manuallyHandledFunctions.contains(fn.name)) {
                continue;
            }
            if (fn.params.elementAt(0).paramType.getType().equals("VK_DEFINE_HANDLE")) {
                out.append("    PFN_");
                out.append(fn.name);
                out.append(" p");
                out.append(fn.name);
                out.append(";\n");
            }
        }
        out.append("    std::unordered_map<BString, U32> functionAddressByName;\n");
        out.append("    std::unordered_map<U32, void*> rayTracingCaptureReplayShaderGroupHandles;\n");
        out.append("    std::unordered_map<U64, void*> debugReportCallbacks;\n");
        out.append("    std::unordered_map<U64, void*> debugUtilsCallbacks;\n");
        out.append("    VkDebugUtilsMessengerEXT debugMessenger;\n");
        out.append("};\n");

        out.append("#endif\n");
        fos.write(out.toString().getBytes());
    }

    static public void write(VkData data, FileOutputStream fos, FileOutputStream fosMarshal, FileOutputStream fosMarshalHeader) throws Exception {
        StringBuilder out = new StringBuilder();
        out.append("// DON'T MODIFY, this is autogenerated\n");
        out.append("#include \"boxedwine.h\"\n");
        out.append("#ifdef BOXEDWINE_VULKAN\n");
        out.append("#include <SDL.h>\n");
        out.append("#include <SDL_vulkan.h>\n");
        out.append("#define VK_NO_PROTOTYPES\n");
        out.append("#include \"vk/vulkan.h\"\n");
        out.append("#include \"vk/vulkan_core.h\"\n");
        out.append("#define BOXED_VK_EXTERN\n");
        out.append("#include \"vk_host.h\"\n");
        out.append("#include \"vk_host_marshal.h\"\n\n");
        out.append("void initVulkan();\n");
        out.append("U32 createVulkanPtr(KMemory* memory, U64 value, BoxedVulkanInfo* info);\n");
        out.append("BoxedVulkanInfo* getInfoFromHandle(KMemory* memory, U32 address);\n");
        out.append("void freeVulkanPtr(KMemory* memory, U32 p);\n");
        out.append("void registerVkMemoryAllocation(VkDeviceMemory memory, VkDeviceSize size);\n");
        out.append("void unregisterVkMemoryAllocation(VkDeviceMemory memory);\n");
        out.append("U32 mapVkMemory(VkDeviceMemory memory, void* pData, VkDeviceSize len);\n");
        out.append("void unmapVkMemory(VkDeviceMemory memory);\n\n");
        for (int i=0;i<26;i++) {
            out.append("#define ARG");
            out.append(i+1);
            out.append(" cpu->peek32(");
            out.append(i+1);
            out.append(")\n");
        }
        StringBuilder part2 = new StringBuilder();

        for (VkFunction fn : data.functions ) {
            if (data.manuallyHandledFunctions.contains(fn.name)) {
                continue;
            }
            writeFunction(data, fn, part2);
        }

        StringBuilder tmp = new StringBuilder();

        Hashtable<String, String> structTypes = new Hashtable<>();
        for (VkType t : data.types.values()) {
            if (t.structType != null && t.structType.equals("VK_STRUCTURE_TYPE_FAULT_DATA")) {
                int ii=0;
            }
             if (t.structType != null && data.orderedTypes.contains(t) && !data.ignoreStructTypes.contains(t.structType)) {
                 structTypes.put(t.structType, t.name);
                 t.needMarshalOut = true;
                 t.setNeedMarshalIn(true);
             }
        }
        for (String ext : structTypes.values()) {
            VkType extType = data.types.get(ext);
            if (extType != null) {
                extType.setNeedMarshalIn(true);
                extType.needMarshalOut = true;
            }
        }
        // pass 1 will add more types that need marshaling, like VkApplicationInfo
        for (int i=0;i<3;i++) {
            for (VkType t : data.orderedTypes) {
                if (t.isNeedMarshalIn() || t.needMarshalOut) {
                    VkHostMarshalType.write(data, t, tmp);
                }
            }
        }

        // pass 4
        StringBuilder outMarshal = new StringBuilder();
        StringBuilder outHeader = new StringBuilder();

        // vk_host_marshal.cpp
        outMarshal.append("// DON'T MODIFY, this is autogenerated\n");
        outMarshal.append("#include \"boxedwine.h\"\n");
        outMarshal.append("#ifdef BOXEDWINE_VULKAN\n");
        outMarshal.append("#include <SDL.h>\n");
        outMarshal.append("#include <SDL_vulkan.h>\n");
        outMarshal.append("#define VK_NO_PROTOTYPES\n");
        outMarshal.append("#include \"vk/vulkan.h\"\n");
        outMarshal.append("#include \"vk/vulkan_core.h\"\n");
        outMarshal.append("#include \"vk_host.h\"\n\n");
        outMarshal.append("#include \"vk_host_marshal.h\"\n\n");

        outHeader.append("struct MarshalFloat {\n");
        outHeader.append("    union {\n");
        outHeader.append("        float f;\n");
        outHeader.append("        U32   i;\n");
        outHeader.append("    };\n");
        outHeader.append("};\n");

        outHeader.append("class MarshalCallbackData {\n");
        outHeader.append("public:\n");
        outHeader.append("    U32 callbackAddress;\n");
        outHeader.append("    U32 userData;\n");
        outHeader.append("};\n");
        outHeader.append("VkBool32 VKAPI_PTR boxed_vkDebugReportCallbackEXT(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData);\n");
        outHeader.append("VkBool32 VKAPI_PTR boxed_vkDebugUtilsMessengerCallbackEXT(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);\n");
        for (VkType t : data.orderedTypes) {
            if (t.isNeedMarshalIn() || t.needMarshalOut) {
                VkHostMarshalType.write(data, t, outMarshal);
                VkHostMarshalType.writeHeader(t, outHeader);
            }
        }
        outMarshal.append("#endif\n\n");
        fosMarshal.write(outMarshal.toString().getBytes());
        fosMarshalHeader.write(outHeader.toString().getBytes());

        part2.append("VkBaseOutStructure* vulkanGetNextPtr(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {\n");
        part2.append("    if (address == 0) {\n");
        part2.append("        return NULL;\n");
        part2.append("    }\n");
        part2.append("    VkStructureType type = (VkStructureType)memory->readd(address);\n");
        part2.append("    switch (type) {\n");
        for (String key : structTypes.keySet()) {
            part2.append("        case ");
            part2.append(key);
            part2.append(": {\n");
            part2.append("            ");
            part2.append(structTypes.get(key));
            part2.append("* p = new ");
            part2.append(structTypes.get(key));
            part2.append("();\n");
            part2.append("            Marshal");
            part2.append(structTypes.get(key));
            part2.append("::read(pBoxedInfo, memory, address, p);\n");
            part2.append("            return (VkBaseOutStructure*)p;\n");
            part2.append("        }\n");
        }
        part2.append("       default:\n");
        part2.append("            kpanic(\"vulkanGetNextPtr not implemented for %d\", type);\n");
        part2.append("    }\n");
        part2.append("}\n");

        part2.append("void vulkanWriteNextPtr(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, const void* p) {\n");
        part2.append("    if (address == 0) {\n");
        part2.append("        return;\n");
        part2.append("    }\n");
        part2.append("    VkStructureType type = (VkStructureType)memory->readd(address);\n");
        part2.append("    switch (type) {\n");
        for (String key : structTypes.keySet()) {
            part2.append("        case ");
            part2.append(key);
            part2.append(": {\n");
            part2.append("            Marshal");
            part2.append(structTypes.get(key));
            part2.append("::write(pBoxedInfo, memory, address, (");
            part2.append(structTypes.get(key));
            part2.append("*)p);\n");
            part2.append("            break;\n");
            part2.append("        }\n");
        }
        part2.append("       default:\n");
        part2.append("            kpanic(\"vulkanWriteNextPtr not implemented for %d\", type);\n");
        part2.append("    }\n");
        part2.append("}\n");

        /*
        for (VkType t : Main.orderedTypes) {
            if (!t.needMarshalIn && !t.needMarshalOut) {
                continue;
            }
            out.append("static_assert(sizeof(");
            out.append(t.name);
            out.append(") == ");
            out.append(t.getSize());
            out.append(", \"false\");\n");
        }
         */
        part2.append("#endif\n\n");
        fos.write(out.toString().getBytes());
        fos.write(part2.toString().getBytes());
    }
}
