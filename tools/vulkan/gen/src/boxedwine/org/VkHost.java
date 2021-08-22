package boxedwine.org;

import java.io.FileOutputStream;
import java.util.Vector;

/**
 * Created by James on 8/21/2021.
 */
public class VkHost {
    static private void hostStart(VkFunction fn, StringBuilder out) throws Exception {
        out.append("PFN_");
        out.append(fn.name);
        out.append(" ");
        out.append(fn.name);
        out.append(";\n");

        if (fn.returnType.getSize() != 0) {
            out.append("// return type: "+fn.returnType.name+"("+fn.returnType.getSize()+" bytes)\n");
        }
        out.append("void vk_" + fn.name.substring(2) + "(CPU* cpu) {\n");
    }

    static private int hostStartParam(VkFunction fn, StringBuilder out, VkParam param, int stackPos) throws Exception {
        param.nameInFunction = param.name;
        if (param.isPointer) {
            param.paramArg = "ARG" + String.valueOf(stackPos);
            fn.getCountParam(param);
            stackPos++;
        } else {
            if (param.sizeof <= 4) {
                param.paramArg = "ARG" + String.valueOf(stackPos);
                stackPos++;
            } else {
                param.paramArg = "dARG" + String.valueOf(stackPos);
                stackPos+=2;
            }
        }
        param.marshal.before(fn, out, param);
        return stackPos;
    }

    static private void hostCall(VkFunction fn, StringBuilder out) throws Exception {
        out.append("    ");
        if (fn.returnType.getSize() == 0) {
            out.append(fn.name);
        } else if (fn.returnType.getSize() == 4) {
            out.append("EAX = ");
            out.append(fn.name);
        } else if (fn.returnType.getSize() == 8) {
            out.append(fn.returnType.name);
            out.append(" result = ");
            out.append(fn.name);
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
        for (VkParam param : fn.params) {
            stackPos = hostStartParam(fn, out, param, stackPos);
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
        for (VkFunction fn : hostFunctions ) {
            out.append("void vk_");
            out.append(fn.name.substring(2));
            out.append("(CPU* cpu);\n");
        }
        out.append("#endif\n");
        fos.write(out.toString().getBytes());
    }

    static public void write(FileOutputStream fos, Vector<VkFunction> hostFunctions) throws Exception {
        StringBuilder out = new StringBuilder();
        out.append("#include \"boxedwine.h\"\n");
        out.append("#ifdef BOXEDWINE_VULKAN\n");
        out.append("#include <SDL.h>\n");
        out.append("#include <SDL_vulkan.h>\n");
        out.append("#define VK_NO_PROTOTYPES\n");
        out.append("#include \"vk/vulkan.h\"\n");
        out.append("#include \"vk/vulkan_core.h\"\n\n");
        out.append("void* getVulkanPtr(U32 address);\n");
        out.append("U32 createVulkanPtr(U64 value);\n");
        out.append("void freeVulkanPtr(U32 p);\n\n");
        for (int i=0;i<15;i++) {
            out.append("#define ARG");
            out.append(i+1);
            out.append(" cpu->peek32(");
            out.append(i+1);
            out.append(")\n");
        }
        for (int i=0;i<15;i++) {
            out.append("#define dARG");
            out.append(i+1);
            out.append(" (cpu->peek32(");
            out.append(i+1);
            out.append(") | ((U64)cpu->peek32(");
            out.append(i+2);
            out.append(")) << 32)\n");
        }

        for (VkFunction fn : hostFunctions ) {
            VkHost.writeFunction(fn, out);
        }
        out.append("#endif\n\n");
        fos.write(out.toString().getBytes());
    }
}
