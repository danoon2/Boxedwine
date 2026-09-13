(() => {
  if (typeof WebGL2RenderingContext === 'undefined') return;
  const denied = /*__DENIED__*/;
  const prototype = WebGL2RenderingContext.prototype;
  const original = prototype.getExtension;
  const observed = new WeakSet();
  prototype.getExtension = function(name) {
    if (String(name).toLowerCase() !== 'oes_sample_variables')
      return original.call(this, name);
    const extension = denied ? null : original.call(this, name);
    if (!observed.has(this)) {
      observed.add(this);
      console.log('MASK_EXTENSION_CONTROL ' + JSON.stringify({
        denied, available: !!extension,
        realm: typeof document === 'undefined' ? 'worker' : 'page'
      }));
    }
    return extension;
  };
})();
