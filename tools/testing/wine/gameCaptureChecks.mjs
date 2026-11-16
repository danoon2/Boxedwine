// Read-only checks shared by the next discovery driver and its lightweight tests.
export function parseLifecycle(text) {
    const lines = String(text || '').split(/\r?\n/);
    const markers = lines.filter(line => /^BW_GAME_(EXIT|CLEANUP):/.test(line));
    const exits = markers.filter(line => line.startsWith('BW_GAME_EXIT:'));
    const cleanups = markers.filter(line => line.startsWith('BW_GAME_CLEANUP:'));
    if (exits.length > 1 || cleanups.length > 1) throw Error('Duplicate lifecycle markers');
    for (const marker of markers) {
        if (!/^BW_GAME_(EXIT|CLEANUP):(0|[1-9][0-9]*)$/.test(marker)) throw Error('Malformed lifecycle marker');
    }
    const exit = exits.length ? Number(exits[0].split(':')[1]) : null;
    const cleanup = cleanups.length ? Number(cleanups[0].split(':')[1]) : null;
    if (exit !== null && exit !== 0) throw Error('Application exit failed: ' + exit);
    if (cleanup !== null && cleanup !== 0) throw Error('Wine cleanup failed: ' + cleanup);
    if (cleanup !== null && (exit === null || markers.indexOf(exits[0]) > markers.indexOf(cleanups[0])))
        throw Error('Cleanup appeared before application exit');
    return { exit, cleanup, complete: exit === 0 && cleanup === 0 };
}

// Modify only CSS layout. The production canvas sizes, visibility, backgrounds
// and compositing order are retained, including a stale overlay if one exists.
export const nativePixelStyle = `
#dropzone { align-items: flex-start !important; justify-content: flex-start !important; }
div.emscripten_border {
  width: calc(var(--boxedwine-canvas-width) * 1px + 2px) !important;
  height: calc(var(--boxedwine-canvas-height) * 1px + 2px) !important;
  min-width: 0 !important; min-height: 0 !important;
  max-width: none !important; max-height: none !important;
  margin: 0 !important; flex: 0 0 auto !important;
}`;

// This function is serialized by Playwright and executes in the test page.
export function measureCanvasStack() {
    const frame = document.querySelector('.emscripten_border');
    if (!frame) throw Error('Canvas frame is absent');
    const rect = value => ({x: value.x, y: value.y, width: value.width, height: value.height});
    return { devicePixelRatio, viewport: {width: innerWidth, height: innerHeight},
        frame: rect(frame.getBoundingClientRect()),
        canvases: [...frame.querySelectorAll('canvas')].map((canvas, order) => {
            const style = getComputedStyle(canvas);
            // Fullscreen removes sibling canvases from the composited top layer
            // even when their ordinary computed CSS says they are visible.
            let visible = !document.fullscreenElement || document.fullscreenElement.contains(canvas), transformed = false;
            for (let element = canvas; element; element = element.parentElement) {
                const computed = getComputedStyle(element);
                visible &&= computed.display !== 'none' && computed.visibility === 'visible'
                    && Number(computed.opacity) !== 0;
                transformed ||= computed.transform !== 'none';
            }
            return {id: canvas.id, width: canvas.width, height: canvas.height, order,
                visible, transformed, zIndex: style.zIndex === 'auto' ? 0 : Number(style.zIndex),
                opacity: style.opacity, background: style.backgroundColor,
                outline: {style: style.outlineStyle, width: style.outlineWidth,
                    offset: style.outlineOffset, color: style.outlineColor},
                boxShadow: style.boxShadow, borderRadius: style.borderRadius,
                rect: rect(canvas.getBoundingClientRect())};
        }) };
}

export function nativeClip(measured) {
    if (measured.devicePixelRatio !== 1) throw Error('Native capture requires device scale 1');
    const visible = measured.canvases.filter(canvas => canvas.visible && canvas.rect.width > 0 && canvas.rect.height > 0);
    if (!visible.length) throw Error('No visible canvas');
    if (visible.some(canvas => !['canvas', 'boxedwine-webgl-canvas-0'].includes(canvas.id)))
        throw Error('Unknown canvas in the display stack');
    visible.sort((a, b) => a.zIndex - b.zIndex || a.order - b.order);
    const top = visible.at(-1);
    const clip = {...top.rect};
    if (visible.some(canvas => canvas.transformed)) throw Error('Transformed canvas cannot be captured at native scale');
    if (top.width !== clip.width || top.height !== clip.height)
        throw Error('Visible canvas backing size differs from displayed size');
    if (!Object.values(clip).every(Number.isInteger) || clip.width <= 0 || clip.height <= 0)
        throw Error('Native capture requires an integer pixel rectangle');
    if (clip.x < 0 || clip.y < 0 || clip.x + clip.width > measured.viewport.width
            || clip.y + clip.height > measured.viewport.height)
        throw Error('Native canvas does not fit in the viewport');
    const frame = measured.frame;
    if (clip.x < frame.x || clip.y < frame.y || clip.x + clip.width > frame.x + frame.width
            || clip.y + clip.height > frame.y + frame.height)
        throw Error('Canvas extends outside its frame');
    return {clip, topCanvas: top.id, backingSize: {width: top.width, height: top.height}};
}
