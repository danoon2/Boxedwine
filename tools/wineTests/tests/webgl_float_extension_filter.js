(() => {
  // Test-only: withhold optional extension discovery/enabling. This cannot
  // undo implicitly enabled GPU features, so it is a capability-policy control.
  const disabled = new Set(/*__DISABLED__*/);
  const prototype = WebGL2RenderingContext.prototype;
  const getExtension = prototype.getExtension;
  const getSupportedExtensions = prototype.getSupportedExtensions;
  prototype.getExtension = function(name) {
    return disabled.has(String(name).toLowerCase()) ? null : getExtension.call(this, name);
  };
  prototype.getSupportedExtensions = function() {
    const values = getSupportedExtensions.call(this);
    return values && values.filter(name => !disabled.has(name.toLowerCase()));
  };
  console.log('FLOAT_EXTENSION_FILTER disabled=' + Array.from(disabled));
})();
