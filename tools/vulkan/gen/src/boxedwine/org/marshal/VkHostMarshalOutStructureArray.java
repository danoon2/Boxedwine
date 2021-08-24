package boxedwine.org.marshal;

import boxedwine.org.VkFunction;
import boxedwine.org.VkParam;

/**
 * Created by James on 8/22/2021.
 */
public class VkHostMarshalOutStructureArray extends VkHostMarshal {
    public void before(VkFunction fn, StringBuilder out, VkParam param) throws Exception {
        out.append("    ");
        out.append(param.paramType.name);
        out.append("* ");
        out.append(param.name);
        out.append(" = NULL;\n");
        out.append("    if (");
        out.append(param.paramArg);
        out.append(") {\n        ");
        out.append(param.name);
        out.append(" = new ");
        out.append(param.paramType.name);
        out.append("[");
        param.countString = "";
        if (!param.countInStructure && param.countParam.isPointer) {
            param.countString += "*";
        }
        param.countString += param.countParam.name;
        out.append(param.countString);
        out.append("];\n    }\n");
    }

    public void after(VkFunction fn, StringBuilder out, VkParam param) throws Exception {
        out.append("    for (U32 i=0;i<");
        out.append(param.countString);
        out.append(";i++) {\n        Marshal");
        out.append(param.paramType.name);
        out.append("::write(");
        out.append(param.paramArg);
        out.append(" + i * ");
        out.append(param.paramType.getSize());
        out.append(", &");
        out.append(param.name);
        out.append("[i]);\n    }\n");
        param.paramType.needMarshalOut = true;
    }
}