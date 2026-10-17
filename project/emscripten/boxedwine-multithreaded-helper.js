addToLibrary({
  // Message ID for Emscripten's 'call Module[handler] on the main thread' pthread
  // command. Emscripten 6.0.0 changed these IDs from strings to integers, so a
  // hand-built {cmd: 'callHandler'} message is dropped as an unknown command.
  // Only --js-library files are preprocessed, so EM_JS and --pre-js code reads
  // the resolved value from here (declare it with EM_JS_DEPS).
  // String library values are emitted as JavaScript expressions. Preserve the
  // quotes around the pre-6.0 command through both preprocessing and emission.
  $bwCallHandlerCmd: {{{ JSON.stringify(typeof CMD_CALL_HANDLER != 'undefined' ? CMD_CALL_HANDLER : "'callHandler'") }}},

  // SDL2 WebAudio stores its state on Module.SDL2 in the browser main thread.
  // With PROXY_TO_PTHREAD, SDL audio can call plain EM_ASM_INT from the pthread;
  // if that happens, Module.SDL2 is missing and audio startup fails. Proxy only
  // SDL audio snippets back to the browser main thread.
  emscripten_asm_const_int__deps: ['$runEmAsmFunction', '$runMainThreadEmAsm'],
  emscripten_asm_const_int: (code, sigPtr, argbuf) => {
    if (ENVIRONMENT_IS_PTHREAD) {
      var asmConst = ASM_CONSTS[code];
      if (asmConst) {
        var source = Function.prototype.toString.call(asmConst);
        if (source.indexOf("SDL2.audio") !== -1 || source.indexOf("audioContext") !== -1) {
          return runMainThreadEmAsm(code, sigPtr, argbuf, 1);
        }
      }
    }
    return runEmAsmFunction(code, sigPtr, argbuf);
  },
});
