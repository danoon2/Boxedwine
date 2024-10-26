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
