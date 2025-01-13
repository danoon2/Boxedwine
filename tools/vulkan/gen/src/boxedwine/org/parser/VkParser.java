package boxedwine.org.parser;

import boxedwine.org.data.*;
import org.w3c.dom.*;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import java.io.File;
import java.util.Hashtable;
import java.util.Vector;

public class VkParser {
    VkData data;
    Hashtable<String, String> badAlias = new Hashtable<>();

    public static void parse(VkData data) throws Exception {
        VkParser parser = new VkParser();
        parser.data = data;

        File inputFile = new File("lib/mesa/vkRegistry/vk.xml");
        DocumentBuilderFactory dbFactory = DocumentBuilderFactory.newInstance();
        DocumentBuilder dBuilder = dbFactory.newDocumentBuilder();
        Document doc = dBuilder.parse(inputFile);
        doc.getDocumentElement().normalize();
        System.out.println("Root element :" + doc.getDocumentElement().getNodeName());
        Element typesNode = (Element) doc.getElementsByTagName("types").item(0);
        NodeList typeList = typesNode.getElementsByTagName("type");
        parser.parseConstants(doc.getElementsByTagName("enums"));

        parser.parseTypes(typesNode, typeList);

        Element commandsNode = (Element) doc.getElementsByTagName("commands").item(0);
        NodeList commands = commandsNode.getElementsByTagName("command");
        parser.parseCommands(commandsNode, commands);

        Element extensionsNode = (Element) doc.getElementsByTagName("extensions").item(0);
        NodeList extensions = extensionsNode.getChildNodes();
        parser.parseExtensions(extensionsNode, extensions);

        NodeList featuresNode = doc.getElementsByTagName("feature");
        parser.parseFeatures(featuresNode.item(0).getParentNode(), featuresNode);

        VkParserRules.run(data);
    }

    void parseTypes(Node parent, NodeList nList) throws Exception {
        Hashtable<VkType, Element> secondPassData = new Hashtable<>();

        for (int temp = 0; temp < nList.getLength(); temp++) {
            Node command = nList.item(temp);
            // filter out member nodes that also have a type node
            if (command.getParentNode().isSameNode(parent)) {
                parseType(command, secondPassData);
            }
        }
        parseTypeMembers(secondPassData);
        for (String alias : badAlias.keySet()) {
            VkType aliasType = data.types.get(alias);
            String typeName = badAlias.get(alias);
            if (aliasType != null) {
                data.types.put(typeName, aliasType);
            } else {
                throw new Exception("Unhandled alias: " + typeName);
            }
        }
    }

    void parseType(Node command, Hashtable<VkType, Element> secondPassData) throws Exception {
        if (command.getNodeType() == Node.ELEMENT_NODE) {
            Element eElement = (Element) command;
            Hashtable<String, String> attributes = new Hashtable<>();
            NamedNodeMap map = eElement.getAttributes();
            for (int i=0;i<map.getLength();i++) {
                String name = map.item(i).getNodeName();
                String value = map.item(i).getNodeValue();
                attributes.put(name, value);
            }

            VkType t = new VkType();
            if (attributes.containsKey("parent")) {
                t.parent = data.types.get(attributes.get("parent"));
                attributes.remove("parent");
            }
            if (attributes.containsKey("objtypeenum")) {
                t.objtypeenum = attributes.get("objtypeenum");
                attributes.remove("objtypeenum");
            }
            if (attributes.containsKey("returnedonly")) {
                t.returnedonly = true;
                attributes.remove("returnedonly");
            }
            if (attributes.containsKey("name")) {
                t.name = attributes.get("name");
                attributes.remove("name");
            }
            if (attributes.containsKey("requires")) {
                t.requires = attributes.get("requires");
                attributes.remove("requires");
            }
            if (attributes.containsKey("bitvalues")) {
                t.bitvalues = attributes.get("bitvalues");
                attributes.remove("bitvalues");
            }
            if (attributes.containsKey("deprecated")) {
                t.deprecated = true;
                attributes.remove("deprecated");
            }
            if (attributes.containsKey("structextends")) {
                String structs = attributes.get("structextends");
                t.structextends = structs.split(",");
                attributes.remove("structextends");
            }
            if (attributes.containsKey("comment")) {
                t.comment = attributes.get("comment");
                attributes.remove("comment");
            }
            if (attributes.containsKey("api")) {
                t.api = attributes.get("api");
                attributes.remove("api");
                if (t.api.equals("vulkansc")) {
                    return;
                }
            }
            if (attributes.containsKey("allowduplicate")) {
                t.allowduplicate = true;
                attributes.remove("allowduplicate");
            }
            if (attributes.containsKey("alias")) {
                String alias = attributes.get("alias");
                attributes.remove("alias");
                VkType aliasType = data.types.get(alias);
                if (aliasType != null) {
                    data.types.put(t.name, aliasType);
                } else {
                    badAlias.put(alias, t.name);
                }
                return;
            }
            if (attributes.containsKey("category")) {
                t.setCategory(attributes.get("category"));
                attributes.remove("category");
                if (t.getCategory().equals("define") || t.getCategory().equals("include")) {
                    if (attributes.size() > 0) {
                        throw new Exception("VkParser.parseType some attributes were not parsed");
                    }
                    return;
                }
            }
            if (attributes.size() > 0) {
                throw new Exception("VkParser.parseType some attributes were not parsed");
            }
            NodeList baseType = eElement.getElementsByTagName("type");
            if (baseType.getLength() > 0 && (t.getCategory() == null || !t.getCategory().equals("union"))) {
                t.setType(baseType.item(0).getTextContent());
            }

            if (t.name == null) {
                t.name = eElement.getElementsByTagName("name").item(0).getTextContent();
            }
            VkType prev = data.types.get(t.name);
            if (prev != null && prev.api != null && t.api != null && !prev.api.equals(t.api)) {
                return;
            }
            if (prev == null) {
                data.types.put(t.name, t);
                data.orderedTypes.add(t);
            } else {
                throw new Exception("oops");
            }
            // members after we added to data.types
            NodeList members = eElement.getElementsByTagName("member");
            if (members.getLength()>0) {
                secondPassData.put(t, eElement);
            }
        }
    }

    void parseTypeMembers(Hashtable<VkType, Element>secondPassData) throws Exception {
        for (VkType vkType : secondPassData.keySet()) {
            Element element = secondPassData.get(vkType);
            NodeList members = element.getElementsByTagName("member");
            vkType.members = new Vector<>();
            for (int i = 0;i<members.getLength();i++) {
                Node member = members.item(i);
                VkParam vkParam = parseParam(vkType, member);
                if (vkParam != null) {
                    vkType.members.add(vkParam);
                }
            }
        }
    }
    void parseCommands(Node parent, NodeList nList) throws Exception {
        for (int temp = 0; temp < nList.getLength(); temp++) {
            Node command = nList.item(temp);
            if (command.getParentNode().isSameNode(parent)) {
                parseCommand(command);
            }
        }
    }

    void parseExtensions(Node parent, NodeList nList) throws Exception {
        for (int temp = 0; temp < nList.getLength(); temp++) {
            Node extension = nList.item(temp);
            if (extension.getParentNode().isSameNode(parent)) {
                parseExtension(extension);
            }
        }
    }

    void parseFeatures(Node parent, NodeList nList) throws Exception {
        for (int temp = 0; temp < nList.getLength(); temp++) {
            Node feature = nList.item(temp);
            if (feature.getParentNode().isSameNode(parent)) {
                parseFeature(feature);
            }
        }
    }

    void parseCommand(Node command) throws Exception {
        if (command.getNodeType() == Node.ELEMENT_NODE) {
            Element eElement = (Element) command;
            VkFunction fn = new VkFunction();
            if (eElement.hasAttribute("alias") && eElement.hasAttribute("name")) {
                String name = eElement.getAttribute("name");
                String alias = eElement.getAttribute("alias");

                for (VkFunction f : data.functions) {
                    if (f.name.equals(alias)) {
                        VkFunction a = f.clone();
                        a.name = name;
                        data.functions.add(a);
                        return;
                    }
                }
            }
            NodeList protoNodeList = eElement.getElementsByTagName("proto");
            if (protoNodeList.getLength() == 0) {
                throw new Exception("command was expected to have proto");
            }
            Element proto = (Element)protoNodeList.item(0);
            fn.returnType = data.types.get(proto.getElementsByTagName("type").item(0).getTextContent());
            if (fn.returnType == null) {
                throw new Exception("Could not find param type: "+proto.getElementsByTagName("type").item(0).getTextContent());
            }
            fn.name = proto.getElementsByTagName("name").item(0).getTextContent();
            if (proto.getTextContent().contains("*")) {
                throw new Exception("Wasn't expecting a pointer return type: "+eElement.getTextContent());
            }
            String api = eElement.getAttribute("api");
            if (api.equals("vulkansc")) {
                return;
            }
            NodeList params = eElement.getElementsByTagName("param");
            parseParams(eElement, params, fn);
            data.functions.add(fn);
        }
    }

    void parseFeature(Node featureNode) throws Exception {
        if (featureNode.getNodeType() == Node.ELEMENT_NODE) {
            Element eElement = (Element) featureNode;

            Hashtable<String, String> attributes = new Hashtable<>();
            NamedNodeMap map = eElement.getAttributes();
            for (int i=0;i<map.getLength();i++) {
                String name = map.item(i).getNodeName();
                String value = map.item(i).getNodeValue();
                attributes.put(name, value);
            }
            VkFeature feature = new VkFeature();
            if (attributes.containsKey("name")) {
                feature.name = attributes.get("name");
                attributes.remove("name");
            }
            if (attributes.containsKey("number")) {
                feature.number = attributes.get("number");
                attributes.remove("number");
            }
            if (attributes.containsKey("depends")) {
                feature.depends = attributes.get("depends");
                attributes.remove("depends");
            }
            if (attributes.containsKey("api")) {
                feature.api = attributes.get("api");
                attributes.remove("api");
            }
            if (attributes.containsKey("comment")) {
                feature.comment = attributes.get("comment");
                attributes.remove("comment");
            }
            if (feature.api == null) {
                return;
            }
            if (attributes.size() > 0) {
                throw new Exception("VkParser.parseExtension some attributes were not parsed");
            }
            NodeList protoNodeList = eElement.getElementsByTagName("require");
            for (int temp = 0; temp < protoNodeList.getLength(); temp++) {
                Node requireNode = protoNodeList.item(temp);
                parseFeatureRequire(requireNode, feature);
            }
            data.features.add(feature);
        }
    }

    void parseExtension(Node extensionNode) throws Exception {
        if (extensionNode.getNodeType() == Node.ELEMENT_NODE) {
            Element eElement = (Element) extensionNode;

            Hashtable<String, String> attributes = new Hashtable<>();
            NamedNodeMap map = eElement.getAttributes();
            for (int i=0;i<map.getLength();i++) {
                String name = map.item(i).getNodeName();
                String value = map.item(i).getNodeValue();
                attributes.put(name, value);
            }
            VkExtension extension = new VkExtension();
            if (attributes.containsKey("name")) {
                extension.name = attributes.get("name");
                attributes.remove("name");
            }
            if (attributes.containsKey("supported")) {
                extension.supported = attributes.get("supported");
                attributes.remove("supported");
            }
            if (attributes.containsKey("platform")) {
                extension.platform = attributes.get("platform");
                attributes.remove("platform");
            }
            if (attributes.containsKey("number")) {
                String str = attributes.get("number");
                extension.number = Integer.parseInt(str);
                attributes.remove("number");
            }
            if (attributes.containsKey("type")) {
                extension.type = attributes.get("type");
                attributes.remove("type");
            }
            if (attributes.containsKey("author")) {
                extension.author = attributes.get("author");
                attributes.remove("author");
            }
            if (attributes.containsKey("depends")) {
                extension.depends = attributes.get("depends");
                attributes.remove("depends");
            }
            if (attributes.containsKey("contact")) {
                extension.contact = attributes.get("contact");
                attributes.remove("contact");
            }
            if (attributes.containsKey("ratified")) {
                extension.ratified = attributes.get("ratified");
                attributes.remove("ratified");
            }
            if (attributes.containsKey("nofeatures")) {
                extension.nofeatures = attributes.get("nofeatures").equals("true");
                attributes.remove("nofeatures");
            }
            if (attributes.containsKey("promotedto")) {
                extension.promotedto = attributes.get("promotedto");
                attributes.remove("promotedto");
            }
            if (attributes.containsKey("comment")) {
                extension.comment = attributes.get("comment");
                attributes.remove("comment");
            }
            if (attributes.containsKey("deprecatedby")) {
                extension.deprecatedby = attributes.get("deprecatedby");
                attributes.remove("deprecatedby");
            }
            if (attributes.containsKey("obsoletedby")) {
                extension.obsoletedby = attributes.get("obsoletedby");
                attributes.remove("obsoletedby");
            }
            if (attributes.containsKey("specialuse")) {
                extension.specialuse = attributes.get("specialuse");
                attributes.remove("specialuse");
            }
            if (attributes.containsKey("provisional")) {
                extension.provisional = attributes.get("provisional").equals("true");
                attributes.remove("provisional");
            }
            if (attributes.containsKey("sortorder")) {
                extension.sortorder = attributes.get("sortorder");
                attributes.remove("sortorder");
            }
            if (attributes.size() > 0) {
                throw new Exception("VkParser.parseExtension some attributes were not parsed");
            }
            NodeList protoNodeList = eElement.getElementsByTagName("require");
            for (int temp = 0; temp < protoNodeList.getLength(); temp++) {
                Node requireNode = protoNodeList.item(temp);
                parseExtensionRequire(requireNode, extension);
            }
            data.extensions.add(extension);
        }
    }

    void parseFeatureRequire(Node requireNode, VkFeature feature) throws Exception {
        if (requireNode.getNodeType() == Node.ELEMENT_NODE) {
            Element eElement = (Element) requireNode;

            Hashtable<String, String> attributes = new Hashtable<>();
            NamedNodeMap map = eElement.getAttributes();
            for (int i = 0; i < map.getLength(); i++) {
                String name = map.item(i).getNodeName();
                String value = map.item(i).getNodeValue();
                attributes.put(name, value);
            }
            if (attributes.containsKey("comment")) {
                attributes.remove("comment");
            }
            if (attributes.containsKey("depends")) {
                attributes.remove("depends");
            }
            if (attributes.size() > 0) {
                throw new Exception("VkParser.parseExtensionRequire some attributes were not parsed");
            }
            NodeList types = eElement.getElementsByTagName("type");
            for (int temp = 0; temp < types.getLength(); temp++) {
                Node typeNode = types.item(temp);
                if (typeNode.getNodeType() == Node.ELEMENT_NODE) {
                    Element typeElement = (Element) typeNode;
                    String name = typeElement.getAttribute("name");
                    feature.types.add(name);
                }
            }
            NodeList functions = eElement.getElementsByTagName("command");
            for (int temp = 0; temp < functions.getLength(); temp++) {
                Node functionNode = functions.item(temp);
                if (functionNode.getNodeType() == Node.ELEMENT_NODE) {
                    Element functionElement = (Element) functionNode;
                    String name = functionElement.getAttribute("name");
                    feature.functions.add(name);
                }
            }
            NodeList enums = eElement.getElementsByTagName("enum");
            for (int temp = 0; temp < enums.getLength(); temp++) {
                Node enumNode = enums.item(temp);
                if (enumNode.getNodeType() == Node.ELEMENT_NODE) {
                    Element enumElement = (Element) enumNode;
                    String name = enumElement.getAttribute("name");
                    String extendsTypes = enumElement.getAttribute("extends");
                    if (extendsTypes.equals("VkStructureType")) {
                        feature.extendsStructures.add(name);
                    }
                }
            }
        }
    }

    void parseExtensionRequire(Node requireNode, VkExtension extension) throws Exception {
        if (requireNode.getNodeType() == Node.ELEMENT_NODE) {
            Element eElement = (Element) requireNode;

            Hashtable<String, String> attributes = new Hashtable<>();
            NamedNodeMap map = eElement.getAttributes();
            for (int i = 0; i < map.getLength(); i++) {
                String name = map.item(i).getNodeName();
                String value = map.item(i).getNodeValue();
                attributes.put(name, value);
            }
            VkExtension.VkExtensionRequire require = new VkExtension.VkExtensionRequire();
            if (attributes.containsKey("depends")) {
                require.depends = attributes.get("depends");
                attributes.remove("depends");
            }
            if (attributes.containsKey("api")) {
                require.api = attributes.get("api");
                attributes.remove("api");
            }
            if (attributes.containsKey("comment")) {
                require.comment = attributes.get("comment");
                attributes.remove("comment");
            }
            if (attributes.size() > 0) {
                throw new Exception("VkParser.parseExtensionRequire some attributes were not parsed");
            }
            NodeList types = eElement.getElementsByTagName("type");
            for (int temp = 0; temp < types.getLength(); temp++) {
                Node typeNode = types.item(temp);
                if (typeNode.getNodeType() == Node.ELEMENT_NODE) {
                    Element typeElement = (Element) typeNode;
                    String name = typeElement.getAttribute("name");
                    VkType vkType = data.types.get(name);
                    if (vkType == null) {
                        throw new Exception(name + " didn't map to type");
                    }
                    require.types.add(vkType);
                }
            }
            NodeList functions = eElement.getElementsByTagName("command");
            for (int temp = 0; temp < functions.getLength(); temp++) {
                Node functionNode = functions.item(temp);
                if (functionNode.getNodeType() == Node.ELEMENT_NODE) {
                    Element functionElement = (Element) functionNode;
                    String name = functionElement.getAttribute("name");
                    VkFunction vkFunction = data.getFunctionByName(name);
                    if (vkFunction == null) {
                        throw new Exception(name + " didn't map to function");
                    }
                    require.functions.add(vkFunction);
                }
            }
            NodeList enums = eElement.getElementsByTagName("enum");
            for (int temp = 0; temp < enums.getLength(); temp++) {
                Node enumNode = enums.item(temp);
                if (enumNode.getNodeType() == Node.ELEMENT_NODE) {
                    Element enumElement = (Element) enumNode;
                    String name = enumElement.getAttribute("name");
                    String extendsTypes = enumElement.getAttribute("extends");
                    if (extendsTypes.equals("VkStructureType")) {
                        require.extendsStructures.add(name);
                    }
                }
            }
            extension.require.add(require);
        }
    }
    void parseParams(Element parent, NodeList params, VkFunction fn) throws Exception {
        for (int p = 0; p < params.getLength(); p++) {
            Node param = params.item(p);
            if (param.getParentNode().isSameNode(parent)) {
                VkParam vkParam = parseParam(null, param);
                if (vkParam != null) {
                    fn.params.add(vkParam);
                }
            }
        }
    }

    void parseConstants(NodeList enums) throws Exception {
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
                            data.constants.put(name, data.constants.get(alias));
                        } else {
                            data.constants.put(name, value);
                        }
                    }
                }
            }
        }
    }

    VkParam parseParam(VkType parent, Node paramNode) throws Exception {
        if (paramNode.getNodeType() == Node.ELEMENT_NODE) {
            Element eElement = (Element) paramNode;
            VkParam param = new VkParam();
            String typeName = eElement.getElementsByTagName("type").item(0).getTextContent();
            param.paramType = data.types.get(typeName);
            if (param.paramType == null) {
                throw new Exception("did not find param type " + typeName);
            }
            Hashtable<String, String> attributes = new Hashtable<>();
            NamedNodeMap map = eElement.getAttributes();
            for (int i=0;i<map.getLength();i++) {
                String name = map.item(i).getNodeName();
                String value = map.item(i).getNodeValue();
                attributes.put(name, value);
            }

            param.name = eElement.getElementsByTagName("name").item(0).getTextContent();
            if (attributes.containsKey("altlen")) {
                param.altlen = attributes.get("altlen");
                attributes.remove("altlen");
            }
            if (attributes.containsKey("len")) {
                if (param.altlen != null) {
                    param.len = param.altlen;
                } else {
                    param.len = attributes.get("len");
                }
                attributes.remove("len");
            }
            if (attributes.containsKey("values")) {
                parent.structType = attributes.get("values");
                attributes.remove("values");
            }
            if (attributes.containsKey("optional")) {
                param.optional = attributes.get("optional");
                attributes.remove("optional");
            }
            if (attributes.containsKey("noautovalidity")) {
                if (!attributes.get("noautovalidity").equals("true")) {
                    throw new Exception("VkParser.parseParam noautovalidity unexpected value");
                }
                param.noautovalidity = true;
                attributes.remove("noautovalidity");
            }
            if (attributes.containsKey("featurelink")) {
                param.featurelink = attributes.get("featurelink");
                attributes.remove("featurelink");
            }
            if (attributes.containsKey("limittype")) {
                param.limittype = attributes.get("limittype");
                attributes.remove("limittype");
            }
            if (attributes.containsKey("selection")) {
                param.selection = attributes.get("selection");
                attributes.remove("selection");
            }
            if (attributes.containsKey("externsync")) {
                param.externsync = attributes.get("externsync");
                attributes.remove("externsync");
            }
            if (attributes.containsKey("selector")) {
                param.selector = attributes.get("selector");
                attributes.remove("selector");
            }
            if (attributes.containsKey("objecttype")) {
                param.objecttype = attributes.get("objecttype");
                attributes.remove("objecttype");
            }
            if (attributes.containsKey("api")) {
                param.api = attributes.get("api");
                attributes.remove("api");
                if (param.api.equals("vulkansc")) {
                    return null;
                }
            }
            if (attributes.containsKey("deprecated")) {
                param.deprecated = attributes.get("deprecated");
                attributes.remove("deprecated");
            }
            if (attributes.containsKey("stride")) {
                param.stride = attributes.get("stride");
                attributes.remove("stride");
            }
            if (attributes.containsKey("validstructs")) {
                param.validstructs = attributes.get("validstructs");
                attributes.remove("validstructs");
            }
            if (attributes.size() > 0) {
                throw new Exception("VkParser.parseParam some attributes were not parsed");
            }
            param.full = eElement.getTextContent().replaceAll("\\s{2,}", " ").trim();
            NodeList commentNode = eElement.getElementsByTagName("comment");
            if (commentNode != null && commentNode.item(0) != null) {
                String comment = commentNode.item(0).getTextContent();
                param.full = param.full.replace(comment, "");
            }
            return param;
        }
        return null;
    }
}
