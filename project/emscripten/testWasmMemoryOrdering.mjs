import assert from 'node:assert/strict';
import {readFileSync} from 'node:fs';
import vm from 'node:vm';

const source = readFileSync(new URL('./boxedwine-shell.js', import.meta.url), 'utf8');
function extract(name) {
    const start = source.indexOf('function ' + name + '(');
    assert.ok(start >= 0, 'missing shell function: ' + name);
    const indent = source.slice(source.lastIndexOf('\n', start) + 1, start);
    const end = source.indexOf('\n' + indent + '}', start);
    assert.ok(end >= 0, 'missing function end: ' + name);
    return source.slice(start, end + indent.length + 2);
}
function setup(query = '', override = true) {
    const context = vm.createContext({
        Config: {
            urlParams: '', rootZipFile: 'root.zip', extraZipFiles: [],
            extraPayload: [], appZipFile: 'game.zip', appPayload: [],
            appDirPrefix: '/home/username/files', d_drive: '/d_drive',
            frameSkip: '0', isSoundEnabled: true, audioFreq: 44100, bpp: 32,
            cpu: '', envProp: '', WorkingDir: '', appSubfolder: '',
            PlayScript: '', Program: 'game.exe', ProgramArgs: []
        },
        ROOT: '/root', DEFAULT_AUDIO_FREQ: 44100, DEFAULT_BPP: 32,
        window: {location: {href: 'http://localhost/boxedwine.html?' + query}},
        allowParameterOverride: () => override,
        console: {log: () => {}}
    });
    vm.runInContext(['getParameter', 'getSafeMemory', 'getEmulatorParams'].map(extract).join('\n'), context);
    context.Config.safeMemory = context.getSafeMemory();
    return context;
}
for (const [query, expected] of [
    ['', false], ['safe-memory=true', true], ['safe-memory=false', false],
    ['safe-memory=1', false], ['safe-memory', false],
    ['args=-safe-memory', false], ['unsafe-memory=true', false]
]) {
    const context = setup(query);
    assert.equal(context.Config.safeMemory, expected, query);
    const args = Array.from(context.getEmulatorParams());
    assert.equal(args.includes('-safe-memory'), expected, query);
    if (expected) assert.ok(args.indexOf('-safe-memory') < args.indexOf('/bin/wine'));
}
assert.equal(setup('safe-memory=true', false).Config.safeMemory, false);
const replacement = setup();
replacement.Config.urlParams = 'safe-memory=true';
assert.equal(replacement.getSafeMemory(), true, 'configured URL parameters must work');
const guest = setup();
guest.Config.ProgramArgs = ['-safe-memory'];
assert.equal(guest.Config.safeMemory, false);
assert.deepEqual(Array.from(guest.getEmulatorParams()).slice(-3), ['/bin/wine', 'game.exe', '-safe-memory']);
const linux = setup('safe-memory=true');
linux.Config.runLinux = true;
linux.Config.Program = '/bin/test';
const linuxArgs = Array.from(linux.getEmulatorParams());
assert.ok(linuxArgs.indexOf('-safe-memory') < linuxArgs.indexOf('/bin/test'));
console.log('PASS: safe-memory defaults, URL opt-in, override policy, and emulator/guest argument separation');
