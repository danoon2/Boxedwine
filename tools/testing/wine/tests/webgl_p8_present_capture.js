/* Observe displayed default-framebuffer colors without changing draw state. */
(() => {
  const proto = globalThis.WebGL2RenderingContext && WebGL2RenderingContext.prototype;
  if (!proto) return;
  const seen = new WeakMap();
  const observed = new WeakMap();
  const expected = [];
  for (let life=0; life<2; ++life) for (let phase=0; phase<4; ++phase) {
    expected.push({life, phase, colors:[23,129].flatMap(i => [i^(0x55+phase+life*7),255-i,((i*3+phase)&255)^128,255])});
  }
  function capture(gl) {
    if (gl.drawingBufferWidth < 481 || gl.drawingBufferHeight < 241 || gl.getParameter(gl.DRAW_FRAMEBUFFER_BINDING)) return;
    const fb = gl.getParameter(gl.READ_FRAMEBUFFER_BINDING), read = gl.getParameter(gl.READ_BUFFER);
    const pack = gl.getParameter(gl.PIXEL_PACK_BUFFER_BINDING);
    const settings = [gl.PACK_ALIGNMENT,gl.PACK_ROW_LENGTH,gl.PACK_SKIP_PIXELS,gl.PACK_SKIP_ROWS];
    const saved = settings.map(v => gl.getParameter(v));
    gl.bindFramebuffer(gl.READ_FRAMEBUFFER, null);
    const defaultRead = gl.getParameter(gl.READ_BUFFER);
    const bytes = new Uint8Array(8);
    try {
      gl.readBuffer(gl.BACK);
      gl.bindBuffer(gl.PIXEL_PACK_BUFFER, null);
      settings.forEach((v,i) => gl.pixelStorei(v,i === 0 ? 1 : 0));
      gl.readPixels(160,240,1,1,gl.RGBA,gl.UNSIGNED_BYTE,bytes,0);
      gl.readPixels(480,240,1,1,gl.RGBA,gl.UNSIGNED_BYTE,bytes,4);
    } finally {
      settings.forEach((v,i) => gl.pixelStorei(v,saved[i]));
      gl.bindBuffer(gl.PIXEL_PACK_BUFFER, pack);
      gl.readBuffer(defaultRead);
      gl.bindFramebuffer(gl.READ_FRAMEBUFFER,fb);
      if (fb) gl.readBuffer(read);
    }
    let raw=observed.get(gl); if (!raw) observed.set(gl,raw=new Set());
    const signature=gl.drawingBufferWidth+':'+gl.drawingBufferHeight+':'+Array.from(bytes).join(',');
    if (raw.size<64 && !raw.has(signature)) {
      raw.add(signature);
      console.log('P8_DISPLAY_RAW '+JSON.stringify({width:gl.drawingBufferWidth,height:gl.drawingBufferHeight,colors:Array.from(bytes)}));
    }
    const match=expected.find(row => row.colors.every((v,i)=>v===bytes[i]));
    if (!match) return;
    let keys=seen.get(gl); if (!keys) seen.set(gl, keys=new Set());
    const key=match.life+':'+match.phase;
    if (keys.has(key)) return;
    keys.add(key);
    console.log('P8_DISPLAY_FRAME '+JSON.stringify({...match,width:gl.drawingBufferWidth,height:gl.drawingBufferHeight}));
  }
  for (const name of ['flush','blitFramebuffer','drawArrays','drawElements']) {
    const original=proto[name];
    proto[name]=function(...args) { const result=original.apply(this,args); capture(this); return result; };
  }
})();
