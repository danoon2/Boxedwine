// Test-only --pre-js replacement for --emrun's browser hooks. Emscripten 5.0.7
// runs addOnExit callbacks before FS.quit() flushes stdout. Module.onExit runs
// after that flush, so the final summary can be acknowledged before ^exit^.
if (globalThis.window && globalThis.document) {
  Module['arguments'] = window.location.search.slice(1).trim().split('&').map(decodeURI);
  if (!Module['arguments'][0]) Module['arguments'] = [];

  // Match emrun's opt-in for its local server; ordinary browser/Node runs keep
  // their normal output and lifetime. Workers forward Module.print to the page.
  if (document.URL.includes('localhost') || document.URL.includes(':6931/')) {
    let sequence = 1;
    let pending = Promise.resolve();
    let exiting = false;
    let reportedFailure = false;
    const previousPrint = Module['print'] || console.log.bind(console);
    const previousError = Module['printErr'] || console.error.bind(console);
    const previousExit = Module['onExit'];

    const post = (body) => fetch('stdio.html', {method: 'POST', body});
    const enqueue = (body) => {
      pending = pending.then(async () => {
        const response = await post(body);
        if (!response.ok) throw new Error('HTTP ' + response.status);
        // Consume the acknowledgment before sending the next message.
        await response.text();
      });
      // Keep the queue rejected so a lost log can never yield a successful exit.
      pending.catch((error) => {
        if (!reportedFailure) {
          reportedFailure = true;
          previousError('ERROR: Test output delivery failed: ' + error);
        }
      });
    };

    Module['print'] = (...args) => {
      enqueue('^out^' + sequence++ + '^' + encodeURIComponent(args.join(' ')));
      previousPrint(...args);
    };
    Module['printErr'] = (...args) => {
      enqueue('^err^' + sequence++ + '^' + encodeURIComponent(args.join(' ')));
      previousError(...args);
    };
    Module['onExit'] = (status) => {
      if (exiting) return;
      exiting = true;
      previousExit?.(status);
      const notifyExit = (code) => {
        // emrun closes this request without a response during server shutdown.
        // Let --kill-exit own browser cleanup; window.close() can race that POST.
        post('^exit^' + code).catch(() => {});
      };
      pending.then(() => notifyExit(status), () => notifyExit(status || 1));
    };
    enqueue('^pageload^');
  }
}
