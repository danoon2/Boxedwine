package boxedwine.org.marshal;

import boxedwine.org.data.VkFunction;
import boxedwine.org.data.VkParam;

/**
 * Created by James on 8/22/2021.
 */
public class VkHostMarshalInStructureArray  extends VkHostMarshal {
    public void before(VkFunction fn, StringBuilder out, VkParam param) throws Exception {
        out.append("    ");
        out.append(param.paramType.name);
        if (param.isDoublePointer) {
            out.append("*");
        }
        out.append("* ");
        out.append(param.name);
        out.append(" = NULL;\n");
        out.append("    if (");
        out.append(param.paramArg);
        out.append(") {\n        ");
        out.append(param.name);
        out.append(" = new ");
        out.append(param.paramType.name);
        if (param.isDoublePointer) {
            out.append("*");
        }
        out.append("[");
        param.countString = "";
        if (!param.countInStructure && param.countParam.isPointer) {
            param.countString += "*";
        }
        param.countString += param.countParam.name;
        out.append(param.countString);
        out.append("];\n");

        out.append("        for (U32 i=0;i<");
        out.append(param.countString);
        out.append(";i++) {\n");
        if (param.isDoublePointer) {
            out.append("            ");
            out.append(param.name);
            out.append("[i] = new ");
            out.append(param.paramType.name);
            if (param.secondArrayLen != null) {
                out.append("[");
                out.append(param.secondArrayLen);
                out.append("];\n");
            } else {
                out.append("();\n");
            }
        }
        if (param.isDoublePointer && param.secondArrayLen != null) {
            out.append("            U32 address = cpu->memory->readd(");
            out.append(param.paramArg);
            out.append(" + i * 4);\n");

            out.append("            for (U32 j=0;j<");
            out.append(param.secondArrayLen);
            out.append(";j++) {\n");
            out.append("                Marshal");
            out.append(param.paramType.name);
            out.append("::read(pBoxedInfo, cpu->memory, address + j * 4, &");
            out.append(param.name);
            out.append("[i][j]);\n            }\n        }\n");
        } else {
            out.append("            Marshal");
            out.append(param.paramType.name);
            out.append("::read(pBoxedInfo, cpu->memory, ");

            if (param.isDoublePointer) {
                out.append("cpu->memory->readd(");
                out.append(param.paramArg);
                out.append(" + i * 4), ");
                out.append(param.name);
                out.append("[i]);\n        }\n");
            } else {
                out.append(param.paramArg);
                out.append(" + i * ");
                if (param.stride != null) {
                    out.append(param.stride);
                } else {
                    if (param.paramType.sizeof == 0) {
                        throw new Exception();
                    }
                    out.append(param.paramType.sizeof);
                }
                out.append(", &");
                out.append(param.name);
                out.append("[i]);\n        }\n");
                if (param.stride != null) {
                    out.append("        ");
                    out.append(param.stride);
                    out.append(" = 4;\n");
                }
            }
        }
        out.append("    }\n");
        param.paramType.setNeedMarshalIn(true);
    }

    public void after(VkFunction fn, StringBuilder out, VkParam param) throws Exception {
        out.append("    if (");
        out.append(param.name);
        out.append(") {\n");
        if (param.isDoublePointer) {
            out.append("        for (U32 i=0;i<");
            out.append(param.countString);
            out.append(";i++) {\n");
            out.append("            delete");
            if (param.secondArrayLen != null) {
                out.append("[]");
            }
            out.append(" ");
            out.append(param.name);
            out.append("[i];\n");
            out.append("        }\n");
        }
        out.append("        delete[] ");
        out.append(param.name);
        out.append(";\n    }\n");
    }
}