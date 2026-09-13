// Usage: node runCanvasFullscreenTest.mjs CHROME PLAYWRIGHT_MODULE NEW_OUTPUT
// A compositor regression for the production shell/CSS, with an old-target
// negative control. No Wine or emulator runtime is launched.
import assert from 'node:assert/strict';
import {createServer} from 'node:http';
import {createRequire} from 'node:module';
import {createHash} from 'node:crypto';
import {mkdir, readFile, writeFile} from 'node:fs/promises';
import {openSync, closeSync, existsSync} from 'node:fs';
import {spawn, spawnSync} from 'node:child_process';
import {resolve, join} from 'node:path';
import {setTimeout as delay} from 'node:timers/promises';

const [chrome, playwrightModule, directory] = process.argv.slice(2);
if (!chrome || !playwrightModule || !directory) throw Error('Expected CHROME PLAYWRIGHT_MODULE NEW_OUTPUT');
const output = resolve(directory);
await mkdir(output, {recursive:false});
const {chromium} = createRequire(import.meta.url)(playwrightModule);
const shell = await readFile(new URL('../../project/emscripten/boxedwine-shell.js', import.meta.url));
const css = await readFile(new URL('../../project/emscripten/boxedwine.css', import.meta.url));
const template = await readFile(new URL('../../project/emscripten/shell.html', import.meta.url));
const canvasMarkup = template.toString().match(/<canvas\b[^>]*><\/canvas>/g);
assert.equal(canvasMarkup?.length, 2);
const source = await readFile(new URL(import.meta.url));
const start = shell.indexOf('function setupCanvasFullscreen(');
const end = shell.indexOf('function toggleSound(', start);
assert(start >= 0 && end > start);
const helper = shell.subarray(start,end).toString();
const sha256 = bytes => createHash('sha256').update(bytes).digest('hex');
const report = {scope:'Fullscreen layer/input regression, not Wine/game acceptance',
    startedAt:new Date().toISOString(), inputs:{shell:sha256(shell),css:sha256(css),template:sha256(template),driver:sha256(source)},
    states:[], errors:[], passed:false};
for (const [name,bytes] of [['boxedwine-shell.js',shell],['boxedwine.css',css],['shell.html',template],['driver.mjs',source]])
    await writeFile(join(output,name),bytes,{flag:'wx'});
const html = `<!doctype html><html><head><link rel="stylesheet" href="/boxedwine.css"></head>
<body><div id="app"><div id="controls"><button id="enter">Fullscreen</button></div>
<div id="dropzone"><div class="emscripten_border">
${canvasMarkup.join('\n')}
</div></div></div><script>
${helper}
const input = document.getElementById('canvas'), gdi = document.getElementById('boxedwine-webgl-canvas-0');
input.width=gdi.width=800; input.height=gdi.height=600; input.tabIndex=0;
input.getContext('2d').fillStyle='#123cbe'; input.getContext('2d').fillRect(0,0,800,600);
gdi.getContext('2d').fillStyle='#24b45c'; gdi.getContext('2d').fillRect(0,0,800,600);
window.events=[];
input.addEventListener('keydown',event=>window.events.push({key:event.key}));
input.addEventListener('mousemove',event=>window.events.push({dx:event.movementX,dy:event.movementY}));
if (location.search === '?fixed') setupCanvasFullscreen(input);
document.getElementById('enter').onclick = () => {
  if (location.search === '?fixed') {
    // Emscripten setCanvasElementSize round-trips target.id through a selector.
    // With the old DOM order, this cleared GDI instead of resizing SDL's canvas.
    const resizeTarget = document.querySelector(input.id);
    window.resizeTargetId = resizeTarget.id;
    resizeTarget.width = 800; resizeTarget.height = 600;
    input.getContext('2d').fillStyle='#123cbe'; input.getContext('2d').fillRect(0,0,800,600);
  }
  // Match the inline canvas size/letterbox styles set by SDL/Emscripten.
  input.style.width='800px'; input.style.height='600px'; input.style.padding='30px 40px';
  input.requestFullscreen().catch(error=>window.events.push({error:String(error)}));
};
</script></body></html>`;
await writeFile(join(output,'fixture.html'),html,{flag:'wx'});
const server=createServer((request,response)=>{
    response.writeHead(200,{'Content-Type':request.url==='/boxedwine.css'?'text/css':'text/html','Cache-Control':'no-store'});
    response.end(request.url==='/boxedwine.css'?css:html);
});
let browser, child, fd;
try {
    await new Promise(done=>server.listen(0,'127.0.0.1',done));
    const profile=join(output,'chrome-profile'); await mkdir(profile);
    fd=openSync(join(output,'chrome.log'),'wx');
    const args=['--headless=new','--no-first-run','--no-default-browser-check','--disable-background-networking',
        '--enable-logging=stderr','--force-device-scale-factor=1','--window-size=1000,800',
        '--remote-debugging-port=0','--user-data-dir='+profile,'about:blank'];
    report.command=[chrome,...args];
    child=spawn(chrome,args,{windowsHide:true,stdio:['ignore',fd,fd]});
    const endpoint=join(profile,'DevToolsActivePort'), deadline=Date.now()+30000;
    while(!existsSync(endpoint)) {
        if(child.exitCode!==null || Date.now()>deadline) throw Error('Chrome startup failed');
        await delay(100);
    }
    const port=Number((await readFile(endpoint,'utf8')).split('\n')[0]);
    browser=await chromium.connectOverCDP('http://127.0.0.1:'+port);
    report.browserVersion=browser.version();
    const page=browser.contexts()[0].pages()[0];
    page.on('pageerror',error=>report.errors.push(String(error)));
    async function capture(name, expected) {
        const state=await page.evaluate(()=>{
            const input=document.getElementById('canvas'),gdi=document.getElementById('boxedwine-webgl-canvas-0');
            return {fullscreen:document.fullscreenElement?.id||null, focused:document.hasFocus(),
                pointerLock:document.pointerLockElement?.id||null, input:input.getBoundingClientRect().toJSON(),
                gdi:gdi.getBoundingClientRect().toJSON(),events:window.events, dpr:devicePixelRatio,
                resizeTargetId:window.resizeTargetId||null};
        });
        const png=await page.screenshot();
        const artifact=name+'.png'; await writeFile(join(output,artifact),png,{flag:'wx'});
        const row={name,expected,artifact,sha256:sha256(png),state};
        report.states.push(row);
        return row;
    }
    for(const fixed of [false,true]) {
        const label=fixed?'fixed':'original';
        await page.goto('http://127.0.0.1:'+server.address().port+'/?'+label);
        await capture(label+'-windowed-gdi','gdi');
        await page.locator('#enter').click();
        await page.waitForFunction(()=>document.fullscreenElement!==null);
        const entered=await capture(label+'-fullscreen-gdi',fixed?'gdi':'input');
        assert.equal(entered.state.fullscreen,fixed?'dropzone':'canvas');
        if(fixed) {
            assert.equal(entered.state.resizeTargetId,'canvas');
            assert.deepEqual(entered.state.input,entered.state.gdi);
            await page.evaluate(()=>document.getElementById('boxedwine-webgl-canvas-0').style.visibility='hidden');
            await capture('fixed-fullscreen-gl-layer','input');
            await page.evaluate(()=>document.getElementById('boxedwine-webgl-canvas-0').style.visibility='visible');
            await capture('fixed-fullscreen-gdi-return','gdi');
            await page.bringToFront();
            await page.locator('#canvas').focus(); await page.keyboard.press('ArrowUp');
            assert(await page.evaluate(()=>document.hasFocus()), 'Pointer lock requires an active document');
            await page.evaluate(()=>document.getElementById('canvas').onclick=()=>document.getElementById('canvas').requestPointerLock());
            await page.locator('#canvas').click();
            await page.waitForFunction(()=>document.pointerLockElement?.id==='canvas');
            await page.mouse.move(400,300); await page.mouse.move(413,309);
            const input=await capture('fixed-fullscreen-input','gdi');
            assert(input.state.events.some(event=>event.key==='ArrowUp'));
            assert(input.state.events.some(event=>event.dx!==undefined&&(event.dx||event.dy)));
            await page.evaluate(()=>document.exitPointerLock());
        }
        await page.evaluate(()=>document.exitFullscreen());
        await page.waitForFunction(()=>document.fullscreenElement===null);
        await capture(label+'-exited','gdi');
    }
    assert.deepEqual(report.errors,[]);
    report.passed=true;
} catch(error) {report.errors.push(String(error)); process.exitCode=1;}
finally {
    if(browser) await browser.close();
    if(child && child.exitCode===null) spawnSync('taskkill',['/PID',String(child.pid),'/T','/F'],{windowsHide:true,stdio:'ignore'});
    if(fd!==undefined) closeSync(fd);
    server.closeAllConnections(); await new Promise(done=>server.close(done));
    report.finishedAt=new Date().toISOString();
    await writeFile(join(output,'report.json'),JSON.stringify(report,null,2)+'\n',{flag:'wx'});
    console.log(JSON.stringify({passed:report.passed,states:report.states.length,errors:report.errors,output}));
}
