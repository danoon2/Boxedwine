
(() => {
  const selection = {"RGBA8": [4, 2], "RGB565": [2], "RGBA4": [4], "RGB10_A2": [4, 2, 1], "R16F": [8, 2], "RG16F": [2], "RGBA16F": [2], "R32F": [], "RG32F": [4], "RGBA32F": [], "DEPTH_COMPONENT16": [4, 2], "DEPTH_COMPONENT24": [4, 2], "DEPTH24_STENCIL8": [4, 2], "DEPTH_COMPONENT32F": [4, 2], "DEPTH32F_STENCIL8": [4, 2]};
  const original = WebGL2RenderingContext.prototype.getInternalformatParameter;
  const seen = new Set();
  WebGL2RenderingContext.prototype.getInternalformatParameter = function(target, format, pname) {
    const result = original.call(this, target, format, pname);
    if (target !== this.RENDERBUFFER || pname !== this.SAMPLES || !result) return result;
    const name = Object.keys(selection).find(name => this[name] === format);
    if (!name) return result;
    const wanted = selection[name];
    if (wanted.some(value => !Array.from(result).includes(value)))
      throw new Error('SAMPLE_FILTER requests unsupported samples for ' + name);
    if (!seen.has(format)) {
      seen.add(format);
      console.log('SAMPLE_FILTER ' + name + ' original=' + Array.from(result) + ' reported=' + wanted);
    }
    return new Int32Array(wanted);
  };
})();
