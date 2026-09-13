import assert from 'node:assert/strict';
import fs from 'node:fs';
import vm from 'node:vm';
const script = fs.readFileSync(new URL('webgl_mask_extension_control.js', import.meta.url), 'utf8');
let checks = 0;
for (const denied of [false, true]) {
  for (const worker of [false, true]) {
    const calls = [], logs = [], value = {};
    class Context {getExtension(name) {calls.push(name);return value;}}
    const context = vm.createContext({WebGL2RenderingContext: Context, console: {log: v => logs.push(v)}, ...(worker ? {} : {document: {}})});
    vm.runInContext(script.replace('/*__DENIED__*/', String(denied)), context);
    const a = new Context(), b = new Context();
    assert.equal(a.getExtension('OES_sample_variables'), denied ? null : value); ++checks;
    assert.equal(a.getExtension('OES_sample_variables'), denied ? null : value); ++checks;
    assert.equal(b.getExtension('oes_sample_variables'), denied ? null : value); ++checks;
    assert.equal(a.getExtension('EXT_color_buffer_float'), value); ++checks;
    assert.deepEqual(calls, denied ? ['EXT_color_buffer_float'] : ['OES_sample_variables','OES_sample_variables','oes_sample_variables','EXT_color_buffer_float']); ++checks;
    assert.equal(logs.length, 2); ++checks;
    for (const log of logs) {
      assert.deepEqual(JSON.parse(log.slice('MASK_EXTENSION_CONTROL '.length)), {denied,available:!denied,realm:worker?'worker':'page'}); ++checks;
    }
  }
}
console.log(JSON.stringify({passed:true,checks,scope:'Per-context extension control, page/worker, real delegation and denial; no GPU execution.'}));
