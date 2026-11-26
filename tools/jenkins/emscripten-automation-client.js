// Injected only by the local/CI automation server, after the normal browser shell.
(() => {
    const config = AUTOMATION_CONFIG;
    const output = [];
    const browserErrors = [];
    let finished = false;
    for (const name of ['log', 'warn', 'error']) {
        const original = console[name].bind(console);
        console[name] = (...args) => {
            const line = args.map(String).join(' ');
            output.push(line);
            original(...args);
        };
    }
    const post = async (suffix, body) => {
        const response = await fetch('/automation/' + config.token + '/' + suffix, {method: 'POST', body});
        if (!response.ok) throw new Error('Automation HTTP ' + response.status + ': ' + suffix);
        return response;
    };
    // Stream progress as well as the final result: a hung/crashed browser must
    // leave useful output even when Module.onExit is never reached.
    let sent = 0;
    let pending = Promise.resolve();
    const flush = () => {
        const end = output.length;
        if (end > sent) {
            const body = JSON.stringify(output.slice(sent, end));
            pending = pending.then(() => post('log', body));
            sent = end;
        }
        return pending;
    };
    const progress = setInterval(() => flush().catch(console.error), 5000);
    const originalArchiveFailure = failArchiveSetup;
    failArchiveSetup = error => {
        originalArchiveFailure(error);
        if (finished) return;
        finished = true;
        clearInterval(progress);
        const detail = String(error && error.stack || error);
        browserErrors.push(detail);
        // Setup failed before the guest started: there is no installed state
        // to flush and onExit will never run. Keep the error and retry fresh.
        flush().then(() => post('result', JSON.stringify({
            reason: 'archive_setup_failed', browserErrors
        }))).catch(failure => console.error('Automation result delivery failed: ' + failure));
    };
    const originalLoadFile = loadFile;
    loadFile = (pathPrefix, filename, callback) => {
        const pin = config.archives[filename];
        if (!pin) return originalLoadFile(pathPrefix, filename, callback);
        const download = async () => {
            for (let attempt = 1; attempt <= 3; attempt++) {
                try {
                    const response = await fetch(pathPrefix + filename, {method: 'GET', cache: 'no-store'});
                    if (response.status !== 200) throw new Error('HTTP ' + response.status);
                    const bytes = await response.arrayBuffer();
                    if (bytes.byteLength !== pin.size) throw new Error('size mismatch');
                    const hash = await crypto.subtle.digest('SHA-256', bytes);
                    const actual = Array.from(new Uint8Array(hash), value => value.toString(16).padStart(2, '0')).join('');
                    if (actual !== pin.sha256) throw new Error('SHA-256 mismatch');
                    console.log('Automation archive verified: ' + filename + ' SHA-256 ' + actual);
                    return bytes;
                } catch (error) {
                    console.error('Automation archive transfer ' + filename + ' attempt ' + attempt + '/3: ' + error);
                    if (attempt === 3) throw new Error('Unable to load ' + filename + ' after 3 transfers: ' + error);
                    // A failed socket write can have consumed an unknown prefix,
                    // even with send(). Restart the response, never its suffix.
                    await new Promise(resolve => setTimeout(resolve, 250 * attempt));
                }
            }
        };
        return download().then(bytes => callback(new Uint8Array(bytes))).catch(failArchiveSetup);
    };
    const uploads = [];
    const originalInitialized = Module.onRuntimeInitialized;
    Module.onRuntimeInitialized = function () {
        if (originalInitialized) originalInitialized();
        // FS.quit runs immediately after onExit and closes IDBFS connections.
        // Keep them open until our asynchronous final flush is acknowledged;
        // this isolated browser is then terminated by the runner.
        IDBFS.quit = () => {};
        // Retain the recorder's actual framebuffer/crop/diff, including MT runs.
        emrun_file_dump = (name, bytes) => {
            uploads.push(post('file/' + encodeURIComponent(name), bytes));
        };
    };
    getEmulatorParams = () => [
        '-root', ROOT, '-zip', Config.extraZipFiles[0], '-zip', Config.rootZipFile,
        '-play', config.script, '-w', config.cwd, ...config.options,
        // Match the browser shell's GDI preset. Its default WebGL profile
        // restricts display modes and otherwise prevents paletted apps (AoE)
        // from starting, even when the registry already selects GDI.
        '-env', 'WINE_D3D_CONFIG=webgl=0,webgl_glsl_es=0',
        '-nosound', '-novideo', '/bin/wine', config.program, ...config.args
    ];
    Module.onExit = code => {
        if (finished) return;
        finished = true;
        clearInterval(progress);
        console.log('Automation guest exited: ' + code + '; flushing filesystem');
        // Preserve the installed files and registry before starting the next
        // phase with the same profile. A fresh profile is used for every retry.
        FS.syncfs(false, async error => {
            try {
                await Promise.all(uploads);
                await flush();
                const response = await post('result', JSON.stringify({code, syncError: error ? String(error) : null, browserErrors}));
                const next = await response.json();
                if (next.url) location.replace(next.url);
            } catch (failure) {
                console.error('Automation result delivery failed: ' + failure);
            }
        });
    };
    addEventListener('error', event => {
        const detail = String(event.error && event.error.stack || event.message);
        browserErrors.push(detail);
        console.error('Browser error: ' + detail);
    });
    addEventListener('unhandledrejection', event => {
        const detail = String(event.reason && event.reason.stack || event.reason);
        browserErrors.push(detail);
        console.error('Browser rejection: ' + detail);
    });
})();
