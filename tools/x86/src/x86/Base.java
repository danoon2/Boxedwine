package x86;

import java.io.FileOutputStream;
import java.io.IOException;

abstract public class Base {
    abstract public void generate(FileOutputStream fos_init);
    static public String header = "/*\r\n" +
            " *  Copyright (C) 2016  The BoxedWine Team\n" +
            " *\r\n" +
            " *  This program is free software; you can redistribute it and/or modify\n" +
            " *  it under the terms of the GNU General Public License as published by\n" +
            " *  the Free Software Foundation; either version 2 of the License, or\n" +
            " *  (at your option) any later version.\n" +
            " *\r\n" +
            " *  This program is distributed in the hope that it will be useful,\n" +
            " *  but WITHOUT ANY WARRANTY; without even the implied warranty of\n" +
            " *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n" +
            " *  GNU General Public License for more details.\n" +
            " *\r\n" +
            " *  You should have received a copy of the GNU General Public License\n" +
            " *  along with this program; if not, write to the Free Software\n" +
            " *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.\n" +
            " */\r\n\r\n";

    public void out(FileOutputStream fos, String line) throws IOException {
        line=line+"\r\n";
        fos.write(line.getBytes());
    }
}
