package boxedwine.org;

import boxedwine.org.marshal.*;
import org.w3c.dom.*;

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
    static Vector<VkType> orderedTypes = new Vector<>();
    static Hashtable<String, String> defs = new Hashtable<>();
    static Hashtable<String, String> constants = new Hashtable<>();
    static HashSet<String> blacklistedExtensions = new HashSet<>();
    static Hashtable<String, String> typeExtensions = new Hashtable<>();

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
        blacklistedExtensions.add("VK_EXT_debug_report");
        blacklistedExtensions.add("VK_EXT_debug_utils");

        // # has void pointer in structure
        blacklistedExtensions.add("VK_NV_device_diagnostic_checkpoints");
        blacklistedExtensions.add("VK_INTEL_performance_query");
        blacklistedExtensions.add("VK_KHR_pipeline_executable_properties");
        blacklistedExtensions.add("VK_NV_device_diagnostic_checkpoints");

        // # has a more complicated count structure I need to work on
        blacklistedExtensions.add("VK_KHR_acceleration_structure");

        // # Deprecated extensions
        blacklistedExtensions.add("VK_NV_external_memory_capabilities");
        blacklistedExtensions.add("VK_NV_external_memory_win32");

        typeExtensions.put("VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES", "VkPhysicalDeviceIDProperties");
        typeExtensions.put("VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_FEATURES_EXT", "VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT");
        typeExtensions.put("VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TRANSFORM_FEEDBACK_FEATURES_EXT", "VkPhysicalDeviceTransformFeedbackFeaturesEXT");
        typeExtensions.put("VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES", "VkPhysicalDeviceHostQueryResetFeatures");

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
            types.put("VK_DEFINE_HANDLE", new VkType("VK_DEFINE_HANDLE", "uint32_t", "define", 4));
            types.put("VisualID", new VkType("VisualID", "uint64_t", "define", 8));
            types.put("xcb_visualid_t", new VkType("xcb_visualid_t", "uint32_t", "define", 4));
            types.put("HANDLE", new VkType("HANDLE", "uint32_t", "define", 4));
            types.put("HINSTANCE", new VkType("HINSTANCE", "uint32_t", "define", 4));
            types.put("HWND", new VkType("HWND", "uint32_t", "define", 4));
            types.put("zx_handle_t", new VkType("zx_handle_t", "uint32_t", "define", 4));
            types.put("RROutput", new VkType("RROutput", "uint32_t", "define", 4));
            types.put("void", new VkType("void", "void", "platform", 0));
            types.put("PFN_vkVoidFunction", new VkType("PFN_vkVoidFunction", "void*", "platform", 4));

            parseConstants(doc.getElementsByTagName("enums"));
            parseTypes(typesNode, typeList, true);
            parseTypes(typesNode, typeList, false);
            Element commandsNode = (Element)doc.getElementsByTagName("commands").item(0);
            NodeList commands = commandsNode.getElementsByTagName("command");
            parseCommands(commandsNode, commands);

            Element extensionsNode = (Element)doc.getElementsByTagName("extensions").item(0);
            NodeList extensions = extensionsNode.getChildNodes();
            parseExtensions(extensionsNode, extensions);
            parseExtensions(extensionsNode, extensions);

            hostFunctions = (Vector<VkFunction>)functions.clone();
            for (VkFunction fn : functions) {
                if (fn.name.equals("vkGetPhysicalDeviceSurfaceSupportKHR")) {
                    int ii=0;
                }
                if (fn.name.equals("vkGetDeviceProcAddr") || fn.name.equals("vkGetInstanceProcAddr") || fn.name.toLowerCase().contains("android") || fn.name.equals("VkDebugReportCallbackCreateInfoEXT")) {
                    hostFunctions.remove(fn);
                }
                if (fn.extension != null && (blacklistedExtensions.contains(fn.extension) || fn.name.contains("Win32"))) {
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
            VkHost.write(fosPassThrough, hostFunctions);
            fosPassThrough.close();

            fosPassThrough = new FileOutputStream(hostSource+"vk_host.h");
            VkHost.writeHeader(fosPassThrough, hostFunctions);
            fosPassThrough.close();

        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static void applyDefs() {
        int lastValue = 0;

        String s = defs.get("VK_LAST_VALUE");
        if (s != null) {
            lastValue = Integer.parseInt(s);
        }
        for (VkFunction fn : functions ) {
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
            if (fn.def == null) {
                fn.def = defs.get(fn.name.substring(2));
            }
            if (fn.def == null) {
                fn.def = String.valueOf(defs.size());
                defs.put(fn.name, fn.def);
            }
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
            if (fn.params.elementAt(0).paramType.type.equals("VK_DEFINE_HANDLE")) {
                out.append("VKFUNC_INSTANCE(");
            } else {
                out.append("VKFUNC(");
            }
            out.append(fn.name.substring(2));
            out.append(")\n");
        }
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
            if (fn.returnType.getSize() == 0) {
                voidCalls[fn.params.size()] = true;
            } else if (fn.returnType.getSize() == 4) {
                returnCalls32[fn.params.size()] = true;
            } else if (fn.returnType.getSize() == 8) {
                returnCalls64[fn.params.size()] = true;
            } else {
                throw new Exception("Unhandled call return size: " + fn.returnType.getSize());
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
            if (fn.returnType.getSize() != 0) {
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
                if (fn.returnType.getSize() == 0) {
                    callType = "";
                } else if (fn.returnType.getSize() == 4) {
                    callType = "_R32";
                } else if (fn.returnType.getSize() == 8) {
                    callType = "_R64";
                } else {
                    throw new Exception("Unhandled call return size: " + fn.returnType.getSize());
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
        fos.write(out.toString().getBytes());
    }
    public static void parseTypes(Node parent, NodeList nList, boolean ignoreMissingTypes) throws Exception {
        for (int temp = 0; temp < nList.getLength(); temp++) {
            Node command = nList.item(temp);
            if (command.getParentNode().isSameNode(parent)) {
                parseType(command, ignoreMissingTypes);
            }
        }
    }

    public static void parseType(Node command, boolean ignoreMissingTypes) throws Exception {
        if (command.getNodeType() == Node.ELEMENT_NODE) {
            Element eElement = (Element) command;
            if (eElement.hasAttribute("alias")) {
                String alias = eElement.getAttribute("alias");
                VkType t = types.get(alias);
                if (t != null) {
                    types.put(eElement.getAttribute("name"), t);
                    return;
                }
            }
            VkType t = new VkType();
            if (eElement.hasAttribute("parent")) {
                t.parent = types.get(eElement.getAttribute("parent"));
            }
            if (eElement.hasAttribute("returnedonly")) {
                t.returnedonly = eElement.hasAttribute("returnedonly");
            }
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
                if (t.parent == null) {
                    t.parent = types.get(t.type);
                }
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
                }
                if (t.category.equals("union") || t.category.equals("struct")) {
                    NodeList members = eElement.getElementsByTagName("member");
                    t.members = new Vector<>();
                    for (int i = 0;i<members.getLength();i++) {
                        Node member = members.item(i);
                        VkParam vkParam = parseParam(member, ignoreMissingTypes);
                        if (vkParam != null) {
                            t.members.add(vkParam);
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
            if (t.type == null) {
                return;
            }
            types.put(t.name, t);
            orderedTypes.add(t);
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
            if (eElement.hasAttribute("alias") && eElement.hasAttribute("name")) {
                String name = eElement.getAttribute("name");
                String alias = eElement.getAttribute("alias");

                for (VkFunction f : functions) {
                    if (f.name.equals(alias)) {
                        VkFunction a = f.clone();
                        a.name = name;
                        functions.add(a);
                        break;
                    }
                }
            }
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
            if (platform != null && !platform.equals("win32") && !platform.equals("")) {
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
    static vkHostMarshalNotImplemented updateDescriptorSetWithTemplate = new vkHostMarshalNotImplemented();

    static void findMarshals(VkFunction fn) throws Exception {
        for (VkParam param : fn.params) {
            if ((fn.name.equals("vkUpdateDescriptorSetWithTemplate") || fn.name.equals("vkUpdateDescriptorSetWithTemplateHKR") || fn.name.equals("vkCmdPushDescriptorSetWithTemplateKHR")) && param.name.equals("pData")) {
                param.marshal = updateDescriptorSetWithTemplate;
            } else if (fn.name.equals("vkGetMemoryHostPointerPropertiesEXT") && param.name.equals("pHostPointer")) {
                param.marshal = updateDescriptorSetWithTemplate;
            } else if (!param.isPointer && param.arrayLen == 0) {
                if (param.paramType.type.equals("VK_DEFINE_HANDLE")) {
                    param.marshal = inHandle;
                } else {
                    param.marshal = none;
                }
            } else {
                if (param.isConst) {
                    if (param.len != null || param.arrayLen != 0) {
                        if (!param.paramType.needsMarshaling() || param.paramType.type.equals("void")) {
                            param.marshal = inMemory;
                        } else if (param.paramType.type.equals("VK_DEFINE_HANDLE")) {
                            param.marshal = inHandleArray;
                        } else if (param.paramType.category.equals("struct") || param.paramType.category.equals("union")){
                            param.marshal = inStructureArray;
                        } else if (param.paramType.category.equals("enum")){
                            param.marshal = inEnumArray;
                        } else {
                            throw new Exception("Unhandled param type: " + fn.name + ":" + param.name);
                        }
                    } else if ((param.paramType.category.equals("struct") || param.paramType.category.equals("union")) && param.paramType.needsMarshaling()) {
                        param.marshal = inStructure;
                    } else if (!param.paramType.needsMarshaling() || param.paramType.type.equals("void")) {
                        param.marshal = inMemory;
                    } else if (param.paramType.type.equals("")) {
                        System.out.println("Unhandled param type: " + fn.name + ":" + param.full);
                    } else {
                        throw new Exception("Unhandled param type: " + fn.name + ":" + param.name);
                    }
                } else {
                    boolean isCountParam = false;
                    for (VkParam p : fn.params) {
                        if (p.len != null && p.len.equals(param.name)) {
                            isCountParam = true;
                        }
                    }
                    if (fn.name.equals("vkMapMemory") && param.name.equals("ppData")) {
                        param.marshal = new VkHostMarshalMapMemory();
                    } else if (isCountParam) {
                        param.marshal = inOut;
                    } else if (param.len != null || param.arrayLen != 0) {
                        if (!param.paramType.needsMarshaling() || param.paramType.type.equals("void")) {
                            param.marshal = inMemory;
                        } else if (param.paramType.type.equals("VK_DEFINE_HANDLE")) {
                            param.marshal = outHandleArray;
                        } else if (param.paramType.category.equals("struct") || param.paramType.category.equals("union")){
                            param.marshal = outStructureArray;
                        } else if (param.len != null && param.paramType.category.equals("enum")) {
                            param.marshal = outEnumArray;
                        } else {
                            throw new Exception("Unhandled param type: " + fn.name + ":" + param.name);
                        }
                    } else if (!param.paramType.needsMarshaling() || (param.len != null && param.paramType.type.equals("void"))) {
                        param.marshal = outData;
                    } else if (param.paramType.type.equals("VK_DEFINE_HANDLE")) {
                        param.marshal = outHandle;
                    } else if (param.paramType.category.equals("struct")) {
                        param.marshal = outStructure;
                    } else if (param.paramType.category.equals("enum")) {
                        param.marshal = outEnum;
                    } else if (param.paramType.type.equals("")) {
                        System.out.println("Unhandled param type: " + fn.name + ":" + param.full);
                    } else {
                        throw new Exception("Unhandled param type: " + fn.name + ":" + param.name);
                    }
                }
            }
        }
    }

    public static void parseParams(Element parent, NodeList params, VkFunction fn) throws Exception {
        for (int p = 0; p < params.getLength(); p++) {
            Node param = params.item(p);
            if (param.getParentNode().isSameNode(parent)) {
                VkParam vkParam = parseParam(param, false);
                if (vkParam != null) {
                    fn.params.add(vkParam);
                }
            }
        }
        findMarshals(fn);
    }

    public static void parseConstants(NodeList enums) throws Exception {
        for (int temp = 0; temp < enums.getLength(); temp++) {
            Node e = enums.item(temp);
            String n = e.getAttributes().getNamedItem("name").getTextContent();
            if (n.equals("API Constants")) {
                NodeList children = e.getChildNodes();
                for (int c = 0; c < children.getLength(); c++) {
                    Node child = children.item(c);
                    NamedNodeMap attributes = child.getAttributes();
                    if (attributes != null) {
                        String name = attributes.getNamedItem("name").getTextContent();
                        String value = null;
                        Node valueNode = attributes.getNamedItem("value");
                        if (valueNode != null) {
                            value = valueNode.getTextContent();
                        }
                        String alias = null;
                        Node aliasNode = attributes.getNamedItem("alias");
                        if (aliasNode != null) {
                            alias = aliasNode.getTextContent();
                        }
                        if (alias != null) {
                            constants.put(name, constants.get(alias));
                        } else {
                            constants.put(name, value);
                        }
                    }
                }
            }
        }
    }

    public static VkParam parseParam(Node paramNode, boolean ignoreMissingTypes) throws Exception {
        if (paramNode.getNodeType() == Node.ELEMENT_NODE) {
            Element eElement = (Element) paramNode;
            VkParam param = new VkParam();
            param.paramType = types.get(eElement.getElementsByTagName("type").item(0).getTextContent());
            if (param.paramType == null) {
                param.paramType = new VkType();
                param.paramType.category = "";
                param.paramType.type = "";
            }
            param.name = eElement.getElementsByTagName("name").item(0).getTextContent();
            if (eElement.hasAttribute("altlen")) {
                param.len = eElement.getAttribute("altlen");
            } else if (eElement.hasAttribute("len")) {
                param.len = eElement.getAttribute("len");
            }
            param.full = eElement.getTextContent().replaceAll("\\s{2,}", " ").trim();
            param.isConst = param.full.contains("const ");
            if (param.full.contains("[")) {
                int pos = param.full.indexOf('[');
                int pos2 = param.full.indexOf(']');
                String len = param.full.substring(pos+1, pos2);
                try {
                    param.arrayLen = Integer.parseInt(len);
                } catch (Exception e) {
                    String constant = constants.get(len);
                    if (constant != null) {
                        param.arrayLen = Integer.parseInt(constant);
                    }
                }
            } else {
                param.isDoublePointer = param.full.chars().filter(ch -> ch == '*').count() == 2;
                param.isPointer = param.full.contains("*");
            }
            return param;
        }
        return null;
    }
}
