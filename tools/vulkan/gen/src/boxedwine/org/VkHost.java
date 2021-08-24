package boxedwine.org;

import boxedwine.org.marshal.VkHostMarshalType;

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

    static private void hostStartParam(VkFunction fn, StringBuilder out, VkParam param, int stackPos) throws Exception {
        param.nameInFunction = param.name;
        param.paramArg = "ARG" + String.valueOf(stackPos);
        if (param.isPointer) {
            fn.getCountParam(param);
        }
        param.marshal.before(fn, out, param);
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
            hostStartParam(fn, out, param, stackPos);
            stackPos++;
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
        StringBuilder part2 = new StringBuilder();

        for (VkFunction fn : hostFunctions ) {
            VkHost.writeFunction(fn, part2);
        }

        out.append("void* vulkanGetNextPtr(U32 address);\n");
        out.append("void vulkanWriteNextPtr(U32 address, void* pNext);\n");
        StringBuilder tmp = new StringBuilder();
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
                VkHostMarshalType.write(t, out);
            }
        }
        part2.append("#endif\n\n");
        fos.write(out.toString().getBytes());
        fos.write(part2.toString().getBytes());
    }
}
