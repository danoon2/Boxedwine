/*
 *  Copyright (C) 2012-2025  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
package boxedwine.org;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.Vector;

public class StreamGobbler implements Runnable
{
    private final InputStream _inputStream;
    private final String name;
    public boolean scriptFinished = false;
    public Vector<String> lines;

    StreamGobbler(InputStream is, String name)
    {
        _inputStream = is;
        this.name = name;
    }

    public void run()
    {
        InputStreamReader isr = null;
        BufferedReader br = null;
        lines = new Vector<>();
        try
        {
            isr = new InputStreamReader(_inputStream);
            br = new BufferedReader(isr);
            String line;
            while ( (line=br.readLine()) != null) {
                if (line.equals("script: success")) {
                    scriptFinished = true;
                }
                lines.addElement(line);
                if (Main.verbose) {
                    System.out.println("  " + name + ": " +line);
                }
            }
        }
        catch (IOException e)
        {
        }
        finally
        {
            if (isr!=null) try {isr.close();} catch (IOException e) {}
            if (isr!=null) try {br.close();} catch (IOException e) {}
        }
    }
}
