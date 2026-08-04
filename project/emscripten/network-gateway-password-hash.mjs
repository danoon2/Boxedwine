import { randomBytes, scryptSync } from "node:crypto";

const scryptOptions = {
    N: 16384,
    r: 8,
    p: 1,
};

function usage() {
    console.log(`Usage:
  node network-apps/gateway-password-hash.mjs <username>
  node network-apps/gateway-password-hash.mjs <username> --password <password>

Prints a BOXEDWINE_GATEWAY_AUTH value for network-gateway.mjs dashboard Basic Auth.`);
}

function parseArgs(args) {
    let username = "";
    let password = "";
    for (let i = 0; i < args.length; i += 1) {
        const arg = args[i];
        if (arg === "--help" || arg === "-h") {
            usage();
            process.exit(0);
        } else if (arg === "--password") {
            password = args[++i] || "";
        } else if (!username) {
            username = arg;
        } else {
            throw new Error(`Unknown argument: ${arg}`);
        }
    }
    if (!username) {
        throw new Error("Username is required");
    }
    if (username.includes(":")) {
        throw new Error("Username cannot contain ':'");
    }
    return { username, password };
}

function readHidden(prompt) {
    if (!process.stdin.isTTY) {
        throw new Error("Interactive password entry requires a TTY. Use --password for scripted usage.");
    }
    return new Promise((resolve, reject) => {
        let value = "";
        const stdin = process.stdin;
        const onData = (chunk) => {
            const text = chunk.toString("utf8");
            for (const char of text) {
                if (char === "\u0003") {
                    cleanup();
                    process.stdout.write("\n");
                    reject(new Error("Cancelled"));
                    return;
                }
                if (char === "\r" || char === "\n") {
                    cleanup();
                    process.stdout.write("\n");
                    resolve(value);
                    return;
                }
                if (char === "\b" || char === "\u007f") {
                    value = value.slice(0, -1);
                    continue;
                }
                value += char;
            }
        };
        const cleanup = () => {
            stdin.off("data", onData);
            stdin.setRawMode(false);
            stdin.pause();
        };
        process.stdout.write(prompt);
        stdin.setRawMode(true);
        stdin.resume();
        stdin.on("data", onData);
    });
}

function hashPassword(username, password) {
    const salt = randomBytes(16);
    const hash = scryptSync(password, salt, 32, scryptOptions);
    return `${username}:scrypt:v1:${salt.toString("base64")}:${hash.toString("base64")}`;
}

try {
    const options = parseArgs(process.argv.slice(2));
    let password = options.password;
    if (!password) {
        password = await readHidden("Password: ");
        const confirm = await readHidden("Confirm password: ");
        if (password !== confirm) {
            throw new Error("Passwords do not match");
        }
    }
    if (!password) {
        throw new Error("Password cannot be empty");
    }

    const auth = hashPassword(options.username, password);
    console.log("");
    console.log("Add this before starting network-gateway.mjs:");
    console.log("");
    console.log(`export BOXEDWINE_GATEWAY_AUTH='${auth}'`);
    console.log("");
    console.log("Or pass it directly:");
    console.log("");
    console.log(`node network-gateway.mjs --auth '${auth}' --port 8001`);
} catch (error) {
    console.error(error && error.message ? error.message : error);
    usage();
    process.exit(1);
}
