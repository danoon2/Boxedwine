package boxedwine.org;

import boxedwine.org.marshal.VkHostMarshalType;

import java.io.FileOutputStream;
import java.util.Vector;

/**
 * Created by James on 8/21/2021.
 */
public class VkHost {
    static private void hostStart(VkFunction fn, StringBuilder out) throws Exception {
        if (fn.returnType.getSize() != 0) {
            out.append("// return type: "+fn.returnType.name+"("+fn.returnType.getSize()+" bytes)\n");
        }
        out.append("void vk_" + fn.name.substring(2) + "(CPU* cpu) {\n");
        if (!fn.params.elementAt(0).paramType.type.equals("VK_DEFINE_HANDLE")) {
            out.append("    initVulkan();\n");
        }/*
        out.append("    klog(\"");
        out.append(fn.name);
        out.append("\");\n");
        */
    }

    static private void hostStartParam(VkFunction fn, StringBuilder out, VkParam param, int stackPos) throws Exception {
        param.nameInFunction = param.name;
        param.paramArg = "ARG" + String.valueOf(stackPos);
        if (param.isDoubleSizeOnStack()) {
            param.paramArg2 = "ARG" + String.valueOf(stackPos+1);
        }
        if (param.isPointer) {
            fn.getCountParam(param);
        }
        param.marshal.before(fn, out, param);
    }

    static private void hostCall(VkFunction fn, StringBuilder out) throws Exception {
        String pfn = "";
        if (fn.params.elementAt(0).paramType.type.equals("VK_DEFINE_HANDLE")) {
            pfn += "pBoxedInfo->";
        }
        pfn += "p";
        pfn += fn.name;
        out.append("    ");
        if (fn.returnType.getSize() == 0) {
            out.append(pfn);
        } else if (fn.returnType.getSize() == 4) {
            out.append("EAX = ");;
            out.append(pfn);
        } else if (fn.returnType.getSize() == 8) {
            out.append(fn.returnType.name);
            out.append(" result = ");
            out.append(pfn);
        } else {
            throw new Exception("Unhandled call return size: " + fn.returnType.getSize());
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
        if (fn.returnType.getSize() == 8) {
            out.append("    EAX = (U32)result;\n");
            out.append("    EDX = (U32)(result >> 32);\n");
        }
    }

    static private void writeFunction(VkFunction fn, StringBuilder out) throws Exception {
        int stackPos = 1;
        hostStart(fn, out);
        if (fn.name.equals("vkAllocateDescriptorSets")) {
            int ii=0;
        }
        for (VkParam param : fn.params) {
            hostStartParam(fn, out, param, stackPos);
            if (stackPos == 1 && fn.params.elementAt(0).paramType.type.equals("VK_DEFINE_HANDLE")) {
                out.append("    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(cpu->memory, ARG1);\n");
            }
            stackPos++;
            if (param.isDoubleSizeOnStack()) {
                stackPos++;
            }
        }
        hostCall(fn, out);
        for (VkParam param : fn.params) {
            if (param.marshal != null) {
                param.marshal.after(fn, out, param);
            }
        }
        out.append("}\n");
    }

    public static void writeHeader(FileOutputStream fos, Vector<VkFunction> hostFunctions) throws Exception {
        StringBuilder out = new StringBuilder();
        out.append("#ifndef __VK_HOST__H__\n");
        out.append("#define __VK_HOST__H__\n");
        out.append("#define VK_NO_PROTOTYPES\n");
        out.append("#include \"vk/vulkan.h\"\n");
        out.append("#include \"vk/vulkan_core.h\"\n");
        out.append("#ifndef BOXED_VK_EXTERN\n");
        out.append("#define BOXED_VK_EXTERN extern\n");
        out.append("#endif\n");
        for (VkFunction fn : hostFunctions ) {
            if (!fn.params.elementAt(0).paramType.type.equals("VK_DEFINE_HANDLE")) {
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
            if (fn.params.elementAt(0).paramType.type.equals("VK_DEFINE_HANDLE")) {
                out.append("    PFN_");
                out.append(fn.name);
                out.append(" p");
                out.append(fn.name);
                out.append(";\n");
            }
        }
        out.append("    std::unordered_map<BString, U32> functionAddressByName;\n");
        out.append("};\n");

        out.append("#endif\n");
        fos.write(out.toString().getBytes());
    }

    static public void write(FileOutputStream fos, Vector<VkFunction> hostFunctions) throws Exception {
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
        out.append("#include \"vk_host.h\"\n\n");
        out.append("void initVulkan();\n");
        out.append("void* getVulkanPtr(KMemory* memory, U32 address);\n");
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
        out.append("#define ARG64(lo, hi) ((U64)lo | ((U64)hi << 32))\n");
        StringBuilder part2 = new StringBuilder();

        for (VkFunction fn : hostFunctions ) {
            VkHost.writeFunction(fn, part2);
        }

        out.append("void* vulkanGetNextPtr(KMemory* memory, U32 address);\n");
        out.append("void vulkanWriteNextPtr(KMemory* memory, U32 address, const void* pNext);\n");
        StringBuilder tmp = new StringBuilder();

        for (String ext : Main.typeExtensions.values()) {
            VkType extType = Main.types.get(ext);
            if (extType != null) {
                extType.needMarshalIn = true;
                extType.needMarshalOut = true;
            }
        }
        // pass 1 will add more types that need marshaling, like VkApplicationInfo
        for (VkType t : Main.orderedTypes) {
            if (t.needMarshalIn || t.needMarshalOut) {
                VkHostMarshalType.write(t, tmp);
            }
        }
        // pass 2 will had 2nd level structures
        for (VkType t : Main.orderedTypes) {
            if (t.needMarshalIn || t.needMarshalOut) {
                VkHostMarshalType.write(t, tmp);
            }
        }
        // pass 3
        for (VkType t : Main.orderedTypes) {
            if (t.needMarshalIn || t.needMarshalOut) {
                VkHostMarshalType.write(t, tmp);
            }
        }
        // pass 4
        for (VkType t : Main.orderedTypes) {
            if (t.needMarshalIn || t.needMarshalOut) {
                VkHostMarshalType.write(t, out);
            }
        }
        part2.append("void* vulkanGetNextPtr(KMemory* memory, U32 address) {\n");
        part2.append("    if (address == 0) {\n");
        part2.append("        return NULL;\n");
        part2.append("    }\n");
        part2.append("    VkStructureType type = (VkStructureType)memory->readd(address);\n");
        part2.append("    switch (type) {\n");
        for (String key : Main.typeExtensions.keySet()) {
            part2.append("        case ");
            part2.append(key);
            part2.append(": {\n");
            part2.append("            ");
            part2.append(Main.typeExtensions.get(key));
            part2.append("* p = new ");
            part2.append(Main.typeExtensions.get(key));
            part2.append("();\n");
            part2.append("            Marshal");
            part2.append(Main.typeExtensions.get(key));
            part2.append("::read(memory, address, p);\n");
            part2.append("            return p;\n");
            part2.append("        }\n");
        }
        part2.append("       default:\n");
        part2.append("            kpanic(\"vulkanGetNextPtr not implemented for %d\", type);\n");
        part2.append("    }\n");
        part2.append("}\n");

        part2.append("void vulkanWriteNextPtr(KMemory* memory, U32 address, const void* p) {\n");
        part2.append("    if (address == 0) {\n");
        part2.append("        return;\n");
        part2.append("    }\n");
        part2.append("    VkStructureType type = (VkStructureType)memory->readd(address);\n");
        part2.append("    switch (type) {\n");
        for (String key : Main.typeExtensions.keySet()) {
            part2.append("        case ");
            part2.append(key);
            part2.append(": {\n");
            part2.append("            Marshal");
            part2.append(Main.typeExtensions.get(key));
            part2.append("::write(memory, address, (");
            part2.append(Main.typeExtensions.get(key));
            part2.append("*)p);\n");
            part2.append("            break;\n");
            part2.append("        }\n");
        }
        part2.append("       default:\n");
        part2.append("            kpanic(\"vulkanWriteNextPtr not implemented for %d\", type);\n");
        part2.append("    }\n");
        part2.append("}\n");
        part2.append("#endif\n\n");
        fos.write(out.toString().getBytes());
        fos.write(part2.toString().getBytes());
    }
}
