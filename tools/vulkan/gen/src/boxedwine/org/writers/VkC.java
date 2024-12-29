package boxedwine.org.writers;

import boxedwine.org.data.VkData;
import boxedwine.org.data.VkFunction;
import boxedwine.org.data.VkParam;

import java.io.FileOutputStream;

public class VkC {
    // This is the vulkan library in the emulated file system, so this will write out each function
    // that is exported, and that function will can into Boxedwine using "int 9a"
    public static void write(VkData data, String dir) throws Exception {
        FileOutputStream fosPassThrough = new FileOutputStream(dir+"vk.c");

        StringBuilder out = new StringBuilder();
        out.append("#include \"vkdef.h\"\n");
        out.append("#include <dlfcn.h>\n");
        out.append("#include <stdint.h>\n");
        out.append("#include <stdio.h>\n");
        out.append("#define U32 uint32_t\n");
        out.append("#define U64 uint64_t\n");

        boolean[] returnCalls32 = new boolean[20];
        boolean[] returnCalls64 = new boolean[20];
        boolean[] voidCalls = new boolean[20];
        for (VkFunction fn : data.functions ) {
            if (fn.returnType.sizeof == 0) {
                voidCalls[fn.params.size()] = true;
            } else if (fn.returnType.sizeof== 4) {
                returnCalls32[fn.params.size()] = true;
            } else if (fn.returnType.sizeof == 8) {
                returnCalls64[fn.params.size()] = true;
            } else {
                throw new Exception("Unhandled call return size: " + fn.returnType.sizeof);
            }
        }
        out.append("\n");
        for (int i=0;i<20;i++) {
            if (voidCalls[i]) {
                defineCall(out, i, 0);
            }
        }
        out.append("\n");
        for (int i=0;i<20;i++) {
            if (returnCalls32[i]) {
                defineCall(out, i, 4);
            }
        }
        out.append("\n");
        for (int i=0;i<20;i++) {
            if (returnCalls64[i]) {
                defineCall(out, i, 8);
            }
        }
        out.append("\n");
        //#define CALL_0_R(index) __asm__("push %0\n\tint $0x99\n\taddl $4, %%esp"::"i"(index):"%eax");
        for (VkFunction fn : data.functions ) {
            out.append(fn.returnType.getEmulatedType());
            if (fn.returnType.sizeof != 0) {
                out.append(" /* " + fn.returnType.name + " */");
            }
            out.append(" " + fn.name + "(");
            boolean first = true;
            for (VkParam param : fn.params) {
                if (!first) {
                    out.append(", ");
                }
                first = false;
                out.append(param.getEmulatedType());
                out.append("/* ");
                out.append(param.full.substring(0, param.full.length()-param.name.length()));
                out.append("*/ ");
                out.append(param.name);
            }
            out.append(") {\n");
            if (fn.name.equals("vkGetDeviceProcAddr") || fn.name.equals("vkGetInstanceProcAddr")) {
                out.append("    U32 result = (U32)dlsym((void*)0,(const char*) pName);\n");
                out.append("    if (!result) {printf(\""+ fn.name + " : Failed to load function %s\\n\", (const char*)pName);}\n");
                out.append("    return result;\n");
            } else {
                String callType = "";
                if (fn.returnType.sizeof == 0) {
                    callType = "";
                } else if (fn.returnType.sizeof == 4) {
                    callType = "_R32";
                } else if (fn.returnType.sizeof == 8) {
                    callType = "_R64";
                } else {
                    throw new Exception("Unhandled call return size: " + fn.returnType.sizeof);
                }
                out.append("    CALL_" + fn.params.size() + callType + "(" + fn.name.substring(2));
                for (VkParam param : fn.params) {
                    out.append(", ");
                    if (param.getEmulatedType().equals("U64")) {
                        out.append("&");
                    } else if (param.sizeof > 8) {
                        throw new Exception("Param has size more than 8");
                    }
                    out.append(param.name);
                }
                out.append(");\n");
            }
            out.append("}\n");
        }
        fosPassThrough.write(out.toString().getBytes());
        fosPassThrough.close();
    }

    // Asm code that runs in the emulated vulkan library to call into the boxedwine code
    static void defineCall(StringBuilder out, int i, int returnSize) {
        out.append("#define CALL_");
        out.append(i);
        if (returnSize == 4) {
            out.append("_R32");
        } else if (returnSize == 8) {
            out.append("_R64");
        }
        out.append("(index");
        for (int j=0;j<i;j++) {
            out.append(", arg");
            out.append(j+1);
        }
        out.append(") __asm__(\"");
        for (int j=i;j>=0;j--) {
            out.append("push %");
            out.append(j);
            out.append("\\n\\t");
        }
        out.append("int $0x9a\\n\\t");
        out.append("addl $");
        out.append(4*(i+1));
        out.append(", %%esp\"::\"i\"(index)");
        for (int j=1;j<=i;j++) {
            out.append(", \"g\"(arg");
            out.append(j);
            out.append(")");
        }
        if (returnSize == 4) {
            out.append(":\"%eax\"");
        } else if (returnSize == 8) {
            out.append(":\"%eax\", \"%edx\"");
        }
        out.append(");\n");
    }
}
