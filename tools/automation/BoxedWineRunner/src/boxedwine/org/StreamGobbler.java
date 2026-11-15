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

import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayDeque;
import java.util.Vector;

public class StreamGobbler implements Runnable
{
    private final InputStream _inputStream;
    private final String name;
    public volatile boolean scriptFinished = false;
    public volatile Throwable failure;
    // Bound both object count and text storage (at most 1 MiB of UTF-16 text).
    static final int MAX_LINES = 4096;
    static final int MAX_CHARS = 512 * 1024;
    static final int MAX_LINE_CHARS = 8192;
    private final ArrayDeque<String> lines = new ArrayDeque<>();
    private int retainedChars;
    private long droppedLines;
    private long truncatedLines;

    StreamGobbler(InputStream is, String name)
    {
        _inputStream = is;
        this.name = name;
    }

    private synchronized void addLine(String line, boolean truncated) {
        if (!truncated && line.equals("script: success")) {
            scriptFinished = true;
        }
        if (truncated) {
            truncatedLines++;
            line += " [line truncated]";
        }
        while (lines.size() >= MAX_LINES || retainedChars + line.length() + 1 > MAX_CHARS) {
            retainedChars -= lines.removeFirst().length() + 1;
            droppedLines++;
        }
        lines.addLast(line);
        retainedChars += line.length() + 1;
        if (Main.verbose) {
            System.out.println("  " + name + ": " + line);
        }
    }

    public synchronized Vector<String> getLines() {
        Vector<String> result = new Vector<>();
        if (droppedLines != 0 || truncatedLines != 0) {
            result.add("Output limited: " + droppedLines + " earlier lines discarded, " + truncatedLines + " oversized lines truncated");
        }
        result.addAll(lines);
        return result;
    }

    public void run() {
        try (InputStreamReader reader = new InputStreamReader(_inputStream)) {
            char[] buffer = new char[8192];
            StringBuilder line = new StringBuilder();
            boolean truncated = false;
            boolean skipLF = false;
            int count;
            // readLine() can itself exhaust the heap on a single enormous line.
            while ((count = reader.read(buffer)) != -1) {
                for (int i = 0; i < count; i++) {
                    char ch = buffer[i];
                    if (skipLF && ch == '\n') {
                        skipLF = false;
                        continue;
                    }
                    skipLF = ch == '\r';
                    if (ch == '\n' || ch == '\r') {
                        addLine(line.toString(), truncated);
                        line.setLength(0);
                        truncated = false;
                    } else if (line.length() < MAX_LINE_CHARS) {
                        line.append(ch);
                    } else {
                        truncated = true;
                    }
                }
            }
            if (line.length() != 0 || truncated) {
                addLine(line.toString(), truncated);
            }
        } catch (IOException | RuntimeException | Error e) {
            failure = e;
        }
    }
}
