import { createReadStream } from "node:fs";
import { stat } from "node:fs/promises";
import { createServer } from "node:http";
import { dirname, extname, join, normalize, resolve, sep } from "node:path";
import { fileURLToPath } from "node:url";

const scriptDir = dirname(fileURLToPath(import.meta.url));
let root = resolve(scriptDir);
let port = 8000;
let host = "127.0.0.1";
const positional = [];

for (let i = 2; i < process.argv.length; i += 1) {
    const argument = process.argv[i];
    if (argument === "--root") {
        root = resolve(process.argv[++i]);
    } else if (argument === "--port") {
        port = Number(process.argv[++i]);
    } else if (argument === "--host") {
        host = process.argv[++i];
    } else {
        positional.push(argument);
    }
}

if (positional.length > 0) {
    if (/^\d+$/.test(positional[0])) {
        port = Number(positional[0]);
    } else {
        root = resolve(positional[0]);
    }
}
if (positional.length > 1) {
    if (/^\d+$/.test(positional[1])) {
        port = Number(positional[1]);
    } else {
        host = positional[1];
    }
}
if (positional.length > 2) {
    host = positional[2];
}

const mimeTypes = new Map([
    [".css", "text/css; charset=utf-8"],
    [".html", "text/html; charset=utf-8"],
    [".js", "text/javascript; charset=utf-8"],
    [".mjs", "text/javascript; charset=utf-8"],
    [".wasm", "application/wasm"],
    [".zip", "application/zip"],
]);

function resolveRequestPath(url) {
    const { pathname } = new URL(url, "http://localhost");
    const relative = normalize(decodeURIComponent(pathname)).replace(/^(\.\.[/\\])+/, "");
    const absolute = join(root, relative);
    if (absolute !== root && !absolute.startsWith(root + sep)) {
        return null;
    }
    return absolute;
}

async function resolveFile(filePath, info) {
    if (!info.isDirectory()) {
        return { path: filePath, info };
    }

    for (const indexName of ["index.html", "boxedwine.html"]) {
        const indexPath = join(filePath, indexName);
        try {
            return { path: indexPath, info: await stat(indexPath) };
        } catch (error) {
            if (error.code !== "ENOENT") {
                throw error;
            }
        }
    }
    throw Object.assign(new Error("No directory index found"), { code: "ENOENT" });
}

const server = createServer(async (request, response) => {
    const filePath = resolveRequestPath(request.url || "/");
    if (!filePath) {
        response.writeHead(403);
        response.end("Forbidden");
        return;
    }

    let info;
    try {
        info = await stat(filePath);
    } catch (error) {
        response.writeHead(404);
        response.end("Not found");
        return;
    }

    let finalFile;
    try {
        finalFile = await resolveFile(filePath, info);
    } catch (error) {
        response.writeHead(404);
        response.end("Not found");
        return;
    }
    response.writeHead(200, {
        "Content-Length": finalFile.info.size,
        "Content-Type": mimeTypes.get(extname(finalFile.path)) || "application/octet-stream",
        "Cross-Origin-Embedder-Policy": "require-corp",
        "Cross-Origin-Opener-Policy": "same-origin",
        "Cross-Origin-Resource-Policy": "same-origin",
    });
    createReadStream(finalFile.path).pipe(response);
});

server.listen(port, host, () => {
    console.log(`Serving ${root} at http://${host}:${port}/`);
});
