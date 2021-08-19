package boxedwine.org;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import java.io.*;
import java.nio.charset.Charset;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Vector;

public class Main {
    static Vector<VkFunction> functions = new Vector<>();
    static Vector<VkFunction> hostFunctions = new Vector<>();
    static Hashtable<String, VkType> types = new Hashtable<>();
    static Hashtable<String, String> defs = new Hashtable<>();
    static HashSet<String> blacklistedExtensions = new HashSet<>();

    static String hostSource = "source/vulkan/";
    static String fsSource = "tools/vulkan/";
    public static void main(String[] args) {
        // # Instance extensions
        blacklistedExtensions.add("VK_KHR_display"); // # Needs WSI work.
        blacklistedExtensions.add("VK_KHR_surface_protected_capabilities");

        // # Device extensions
        blacklistedExtensions.add("VK_AMD_display_native_hdr");
        blacklistedExtensions.add("VK_EXT_display_control"); // # Requires VK_EXT_display_surface_counter
        blacklistedExtensions.add("VK_EXT_full_screen_exclusive");
        blacklistedExtensions.add("VK_EXT_hdr_metadata"); // # Needs WSI work.
        blacklistedExtensions.add("VK_EXT_pipeline_creation_feedback");
        blacklistedExtensions.add("VK_GOOGLE_display_timing");
        blacklistedExtensions.add("VK_KHR_external_fence_win32");
        blacklistedExtensions.add("VK_KHR_external_memory_win32");
        blacklistedExtensions.add("VK_KHR_external_semaphore_win32");
        // # Relates to external_semaphore and needs type conversions in bitflags.
        blacklistedExtensions.add("VK_KHR_shared_presentable_image"); // # Needs WSI work.
        blacklistedExtensions.add("VK_KHR_win32_keyed_mutex");

        // # Extensions for other platforms
        blacklistedExtensions.add("VK_EXT_external_memory_dma_buf");
        blacklistedExtensions.add("VK_EXT_image_drm_format_modifier");
        blacklistedExtensions.add("VK_KHR_external_fence_fd");
        blacklistedExtensions.add("VK_KHR_external_memory_fd");
        blacklistedExtensions.add("VK_KHR_external_semaphore_fd");

        // # Extensions which require callback handling
        blacklistedExtensions.add("VK_EXT_device_memory_report");

        // # Deprecated extensions
        blacklistedExtensions.add("VK_NV_external_memory_capabilities");
        blacklistedExtensions.add("VK_NV_external_memory_win32");

        try {
            File inputFile = new File("lib/mesa/vkRegistry/vk.xml");
            DocumentBuilderFactory dbFactory = DocumentBuilderFactory.newInstance();
            DocumentBuilder dBuilder = dbFactory.newDocumentBuilder();
            Document doc = dBuilder.parse(inputFile);
            doc.getDocumentElement().normalize();
            System.out.println("Root element :" + doc.getDocumentElement().getNodeName());
            Element typesNode = (Element)doc.getElementsByTagName("types").item(0);
            NodeList typeList = typesNode.getElementsByTagName("type");
            types.put("char", new VkType("char", "char", "platform", 1));
            types.put("float", new VkType("float", "float", "platform", 4));
            types.put("double", new VkType("double", "double", "platform", 8));
            types.put("int8_t", new VkType("int8_t", "int8_t", "platform", 1));
            types.put("uint8_t", new VkType("uint8_t", "uint8_t", "platform", 1));
            types.put("int16_t", new VkType("int16_t", "int16_t", "platform", 2));
            types.put("uint16_t", new VkType("uint16_t", "uint16_t", "platform", 2));
            types.put("int32_t", new VkType("int32_t", "int32_t", "platform", 4));
            types.put("int", new VkType("int", "int", "platform", 4));
            types.put("uint32_t", new VkType("uint32_t", "uint32_t", "platform", 4));
            types.put("int64_t", new VkType("int64_t", "int64_t", "platform", 8));
            types.put("uint64_t", new VkType("uint64_t", "uint64_t", "platform", 8));
            types.put("size_t", new VkType("size_t", "size_t", "platform", 4));
            types.put("VK_DEFINE_NON_DISPATCHABLE_HANDLE", new VkType("VK_DEFINE_NON_DISPATCHABLE_HANDLE", "uint64_t", "define", 8));
            types.put("VK_DEFINE_HANDLE", new VkType("VK_DEFINE_HANDLE", "uint64_t", "define", 8));
            types.put("VisualID", new VkType("VisualID", "uint64_t", "define", 8));
            types.put("xcb_visualid_t", new VkType("xcb_visualid_t", "uint32_t", "define", 4));
            types.put("HANDLE", new VkType("HANDLE", "uint32_t", "define", 4));
            types.put("zx_handle_t", new VkType("zx_handle_t", "uint32_t", "define", 4));
            types.put("RROutput", new VkType("RROutput", "uint32_t", "define", 4));
            types.put("void", new VkType("void", "void", "platform", 0));

            parseTypes(typesNode, typeList);
            Element commandsNode = (Element)doc.getElementsByTagName("commands").item(0);
            NodeList commands = commandsNode.getElementsByTagName("command");
            parseCommands(commandsNode, commands);

            Element extensionsNode = (Element)doc.getElementsByTagName("extensions").item(0);
            NodeList extensions = extensionsNode.getChildNodes();
            parseExtensions(extensionsNode, extensions);
            parseExtensions(extensionsNode, extensions);

            hostFunctions = (Vector<VkFunction>)functions.clone();
            for (VkFunction fn : functions) {
                if (fn.name.equals("vkGetDeviceProcAddr") || fn.name.equals("vkGetInstanceProcAddr") || fn.name.toLowerCase().contains("android")) {
                    hostFunctions.remove(fn);
                }
                if (fn.extension != null && blacklistedExtensions.contains(fn.extension)) {
                    hostFunctions.remove(fn);
                }
            }
            // in case we get a new vk.xml, we don't want to re-order the defines, this will maintain backwards compatibility with file systems
            try {
                FileInputStream fis = new FileInputStream(hostSource+"vkdef.h");
                readDefs(fis);
                fis.close();
            } catch (Exception e) {

            }
            applyDefs();
            FileOutputStream fosDefs = new FileOutputStream(hostSource+"vkdef.h");
            writeDefs(fosDefs);
            fosDefs.close();

            fosDefs = new FileOutputStream(fsSource+"vkdef.h");
            writeDefs(fosDefs);
            fosDefs.close();

            FileOutputStream fosFuncs = new FileOutputStream(hostSource+"vkfuncs.h");
            writeFuncs(fosFuncs);
            fosFuncs.close();

            FileOutputStream fosPassThrough = new FileOutputStream(fsSource+"vk.c");
            writePassthroughFile(fosPassThrough);
            fosPassThrough.close();

            fosPassThrough = new FileOutputStream(hostSource+"vk_host.cpp");
            writeHostPassthroughFile(fosPassThrough);
            fosPassThrough.close();

            fosPassThrough = new FileOutputStream(hostSource+"vk_host.h");
            writeHostPassthroughHeader(fosPassThrough);
            fosPassThrough.close();

        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static void applyDefs() {
        int lastValue = 0;

        if (defs.contains("VK_LAST_VALUE")) {
            lastValue = Integer.parseInt(defs.get("VK_LAST_VALUE"));
        }
        for (VkFunction fn : hostFunctions ) {
            String key = fn.name.substring(2);
            String def = defs.get(key);
            if (def != null) {
                fn.def = def;
            } else {
                lastValue++;
                def = String.valueOf(lastValue);
                fn.def = def;
                defs.put(key, def);
            }
        }
        defs.put("LAST_VALUE", String.valueOf(lastValue));
    }

    public static void readDefs(FileInputStream fis) throws Exception {
        InputStreamReader isr = new InputStreamReader(fis, Charset.forName("UTF-8"));
        BufferedReader br = new BufferedReader(isr);
        String line;

        while ((line = br.readLine()) != null) {
            String[] parts = line.split(" ");
            defs.put(parts[1], parts[2]);
        }
    }

    public static void writeDefs(FileOutputStream fos) throws Exception {
        StringBuilder out = new StringBuilder();
        for (VkFunction fn : functions ) {
            out.append("#define ");
            out.append(fn.name.substring(2));
            out.append(" ");
            out.append(fn.def);
            out.append("\n");
        }
        out.append("#define VK_LAST_VALUE ");
        out.append(functions.size());
        out.append("\n");
        fos.write(out.toString().getBytes());
    }

    public static void writeFuncs(FileOutputStream fos) throws Exception {
        StringBuilder out = new StringBuilder();
        for (VkFunction fn : hostFunctions ) {
            out.append("VKFUNC(");
            out.append(fn.name.substring(2));
            out.append(")\n");
        }
        fos.write(out.toString().getBytes());
    }

    public static void writeHostPassthroughHeader(FileOutputStream fos) throws Exception {
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

    public static void writeHostPassthroughFile(FileOutputStream fos) throws Exception {
        StringBuilder out = new StringBuilder();
        out.append("#include \"boxedwine.h\"\n");
        out.append("#ifdef BOXEDWINE_VULKAN\n");
        out.append("#include <SDL.h>\n");
        out.append("#include <SDL_vulkan.h>\n");
        out.append("#define VK_NO_PROTOTYPES\n");
        out.append("#include \"vk/vulkan.h\"\n");
        out.append("#include \"vk/vulkan_core.h\"\n\n");

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
            out.append("PFN_");
            out.append(fn.name);
            out.append(" ");
            out.append(fn.name);
            out.append(";\n");
            if (fn.returnType.sizeof != 0) {
                out.append("// return type: "+fn.returnType.name+"("+fn.returnType.sizeof+" bytes)\n");
            }
            out.append("void vk_" + fn.name.substring(2) + "(CPU* cpu) {\n");
            int stackPos = 1;
            for (VkParam param : fn.params) {
                out.append("    ");
                if (param.isPointer) {
                    if (param.paramType == null) {
                        // undefined struct pointers
                        out.append(param.full.substring(0, param.full.length()-param.name.length()-1)+" "+param.name);
                    } else {
                        out.append(param.paramType.name);
                        out.append("*");
                        if (param.isDoublePointer) {
                            out.append("*");
                        }
                        out.append(" " + param.name);
                    }
                } else {
                    out.append(param.full);
                }
                out.append(" = ");
                if (param.isPointer) {
                    out.append("(");
                    if (param.paramType == null) {
                        out.append(param.full.substring(0, param.full.length()-param.name.length()-1)+" "+param.name);
                    } else {
                        out.append(param.paramType.name);
                    }
                    out.append("*");
                    if (param.isDoublePointer) {
                        out.append("*");
                    }
                    out.append(")getPhysicalAddress(");
                } else if (param.paramType != null) {
                    out.append("("+param.paramType.name+")");
                }
                if (param.isPointer || param.sizeof <= 4) {
                    out.append("ARG");
                    out.append(stackPos);
                    stackPos++;
                } else {
                    out.append("dARG");
                    out.append(stackPos);
                    stackPos+=2;
                }
                if (param.isPointer) {
                    out.append(", 0)");
                }
                out.append(";\n");
            }
            out.append("    ");
            if (fn.returnType.sizeof == 0) {
                out.append(fn.name);
            } else if (fn.returnType.sizeof == 4) {
                out.append("EAX = ");
                out.append(fn.name);
            } else if (fn.returnType.sizeof == 8) {
                out.append(fn.returnType.name);
                out.append(" result = ");
                out.append(fn.name);
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
                out.append(param.name);
            }
            out.append(");\n");
            if (fn.returnType.sizeof == 8) {
                out.append("    EAX = (U32)result;\n");
                out.append("    EDX = (U32)(result >> 32);\n");
            }
            out.append("}\n");
        }
        out.append("#endif\n\n");
        fos.write(out.toString().getBytes());
    }
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
        out.append("int $0x99\\n\\t");
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

    public static void writePassthroughFile(FileOutputStream fos) throws Exception {
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
        for (VkFunction fn : functions ) {
            if (fn.returnType.sizeof == 0) {
                voidCalls[fn.params.size()] = true;
            } else if (fn.returnType.sizeof == 4) {
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
        for (VkFunction fn : functions ) {
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
                    if (param.sizeof == 8) {
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
        fos.write(out.toString().getBytes());
    }
    public static void parseTypes(Node parent, NodeList nList) throws Exception {
        for (int temp = 0; temp < nList.getLength(); temp++) {
            Node command = nList.item(temp);
            if (command.getParentNode().isSameNode(parent)) {
                parseType(command);
            }
        }
    }

    public static void parseType(Node command) throws Exception {
        if (command.getNodeType() == Node.ELEMENT_NODE) {
            Element eElement = (Element) command;
            VkType t = new VkType();
            if (eElement.hasAttribute("category")) {
                t.category = eElement.getAttribute("category");
                if (t.category.equals("define")) {
                    return;
                }
            } else {
                return;
            }
            NodeList tt = eElement.getElementsByTagName("type");
            if (tt.getLength() > 0) {
                t.type = tt.item(0).getTextContent();
            }
            if (t.category.equals("struct") || t.category.equals("enum") || t.category.equals("union")) {
                if (eElement.hasAttribute("name")) {
                    t.name = eElement.getAttribute("name");
                } else {
                    throw new Exception("struct had no name");
                }
                if (t.category.equals("enum")) {
                    t.type = "int32_t";
                } else if (t.category.equals("union")) {
                    NodeList members = eElement.getElementsByTagName("member");
                    t.type = "union";
                    for (int i = 0;i<members.getLength();i++) {
                        if (members.item(i).getTextContent().contains("*")) {
                            if (t.sizeof < 4) {
                                t.sizeof = 4;
                            }
                            continue;
                        }
                        Element member = (Element)members.item(i);
                        NodeList unionType = member.getElementsByTagName("type");
                        VkType vkType = types.get(unionType.item(0).getTextContent());
                        if (vkType == null) {
                            throw new Exception("Failed to parse union: " + unionType.item(0).getTextContent());
                        }
                        if (t.sizeof < vkType.sizeof) {
                            t.sizeof = vkType.sizeof;
                        }
                    }
                }
            } else {
                NodeList name = eElement.getElementsByTagName("name");
                if (name.getLength() > 0) {
                    t.name = name.item(0).getTextContent();
                } else {
                    return;
                }
            }
            if (t.name.equals(t.type) && t.category.equals("basetype")) {
                return;
            }
            if (t.category.equals("funcpointer")) {
                t.sizeof = 4;
            } else if (t.type == null) {
                return;
            } else if (!t.category.equals("struct") && !t.category.equals("union")) {
                VkType k = types.get(t.type);
                if (k == null) {
                    throw new Exception("Could not find type: " + t.type);
                }
                t.sizeof = k.sizeof;
            }
            types.put(t.name, t);
        }
    }
    public static void parseCommands(Node parent, NodeList nList) throws Exception {
        for (int temp = 0; temp < nList.getLength(); temp++) {
            Node command = nList.item(temp);
            if (command.getParentNode().isSameNode(parent)) {
                parseCommand(command);
            }
        }
    }

    public static void parseExtensions(Node parent, NodeList nList) throws Exception {
        for (int temp = 0; temp < nList.getLength(); temp++) {
            Node command = nList.item(temp);
            if (command.getParentNode().isSameNode(parent)) {
                parseExtension(command);
            }
        }
    }

    public static void parseCommand(Node command) throws Exception {
        if (command.getNodeType() == Node.ELEMENT_NODE) {
            Element eElement = (Element) command;
            VkFunction fn = new VkFunction();
            NodeList protoNodeList = eElement.getElementsByTagName("proto");
            if (protoNodeList.getLength() == 0) {
                return;
            }
            Element proto = (Element)protoNodeList.item(0);
            fn.returnType = types.get(proto.getElementsByTagName("type").item(0).getTextContent());
            if (fn.returnType == null) {
                throw new Exception("Could not find param type: "+proto.getElementsByTagName("type").item(0).getTextContent());
            }
            fn.name = proto.getElementsByTagName("name").item(0).getTextContent();
            if (proto.getTextContent().contains("*")) {
                throw new Exception("Wasn't expecting a pointer return type: "+eElement.getTextContent());
            }
            NodeList params = eElement.getElementsByTagName("param");
            parseParams(eElement, params, fn);
            functions.add(fn);
        }
    }

    public static VkFunction getFunctionByName(String name) {
        for (VkFunction fn : functions) {
            if (fn.name.equals(name)) {
                return fn;
            }
        }
        return null;
    }

    public static void parseExtension(Node extension) throws Exception {
        if (extension.getNodeType() == Node.ELEMENT_NODE) {
            Element eElement = (Element) extension;
            String name = eElement.getAttribute("name");
            String requires = eElement.getAttribute("requires");
            String supported = eElement.getAttribute("supported");
            String platform = eElement.getAttribute("platform");

            if (supported != null && supported.equals("disabled")) {
                blacklistedExtensions.add(name);
            }
            if (name.toLowerCase().contains("khx") || name.toLowerCase().contains("nvx")) {
                blacklistedExtensions.add(name);
            }
            if (platform != null && !platform.equals("win32")) {
                blacklistedExtensions.add(name);
            }
            if (requires != null) {
                String[] parts = requires.split(",");
                for (String part : parts) {
                    if (blacklistedExtensions.contains(part)) {
                        blacklistedExtensions.add(name);
                    }
                }
            }
            NodeList protoNodeList = eElement.getElementsByTagName("command");
            for (int temp = 0; temp < protoNodeList.getLength(); temp++) {
                Node command = protoNodeList.item(temp);
                VkFunction fn = getFunctionByName(command.getAttributes().getNamedItem("name").getTextContent());
                if (fn != null) {
                    fn.extension = name;
                }
            }
        }
    }

    public static void parseParams(Element parent, NodeList params, VkFunction fn) throws Exception {
        for (int p = 0; p < params.getLength(); p++) {
            Node param = params.item(p);
            if (param.getParentNode().isSameNode(parent)) {
                parseParam(param, fn);
            }
        }
    }

    public static void parseParam(Node paramNode, VkFunction fn) throws Exception {
        if (paramNode.getNodeType() == Node.ELEMENT_NODE) {
            Element eElement = (Element) paramNode;
            VkParam param = new VkParam();
            param.paramType = types.get(eElement.getElementsByTagName("type").item(0).getTextContent());
            param.name = eElement.getElementsByTagName("name").item(0).getTextContent();
            if (eElement.hasAttribute("len")) {
                param.len = eElement.getAttribute("len");
            }
            param.full = eElement.getTextContent().replaceAll("\\s{2,}", " ").trim();
            param.isDoublePointer = param.full.contains("**");
            param.isPointer = param.full.contains("*");
            param.isConst = param.full.contains("const ");
            if (param.paramType == null && !param.isPointer) {
                throw new Exception("Could not find param type: "+eElement.getElementsByTagName("type").item(0).getTextContent());
            }
            if (param.isPointer) {
                param.sizeof = 4;
            } else {
                param.sizeof = param.paramType.sizeof;
                if (param.sizeof == 0) {
                    throw new Exception("Unknown size of param: " + param.full);
                }
            }
            fn.params.add(param);
        }
    }
}
