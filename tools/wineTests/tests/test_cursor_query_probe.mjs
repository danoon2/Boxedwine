import assert from 'node:assert/strict';
import {checkCursorTrace, checkLockedCursor, checkWarpTrace} from '../runCursorQueryProbe.mjs';

const row = (kind, {ok = '1,1,1', client = '470,427', screen = '554,517', buttons = '0,0', capture = 0} = {}) =>
    `INPUT_${kind} generation=3 mode=GL ok=${ok} error=0 message=470,428 screen=${screen} client=${client} origin=84,90 buttons=${buttons} capture=${capture}\n`;
const ready = 'INPUT_READY mode=GL\n';
const down = row('DOWN', {buttons: '1,1', capture: 1});
const held = row('POLL', {buttons: '1,1', capture: 1});
const up = row('UP');
const released = row('POLL');
const good = ready + down + held + up + released;
assert.equal(checkCursorTrace(good).maxMessageDelta, 1);
assert.throws(() => checkCursorTrace(good.replaceAll('ok=1,1,1', 'ok=0,1,1')), /query returned false/);
assert.throws(() => checkCursorTrace(ready + down + held + up), /Missing delayed/);
assert.throws(() => checkCursorTrace(ready + down + held + up + row('POLL', {client: '588,535', screen: '672,625'})), /diverges/);
assert.throws(() => checkCursorTrace(ready + down + held + up + row('POLL', {screen: '470,427'})), /Screen\/client/);
assert.throws(() => checkCursorTrace(ready + down + row('POLL', {capture: 1}) + up + released), /Held button/);
assert.throws(() => checkCursorTrace(ready + down + held + up + row('POLL', {buttons: '1,1'})), /Released button/);
assert.throws(() => checkCursorTrace(good + 'INPUT_ERROR present=8876086a\n'), /API error/);
assert.throws(() => checkCursorTrace(good.replace('INPUT_DOWN generation=3', 'INPUT_DOWN generation=unknown')), /Malformed/);
console.log('PASS cursor-query positive and eight rejection controls');

const locked = row('POLL') + row('POLL');
assert(checkLockedCursor(locked, [470, 428], false).passed);
assert.throws(() => checkLockedCursor(locked, [320, 240], false), /relative motion/);
assert.throws(() => checkLockedCursor(locked, [470, 428], true), /button state/);
assert.throws(() => checkLockedCursor(row('POLL'), [470, 428], false), /two delayed/);
assert.throws(() => checkLockedCursor(locked.replaceAll('message=470,428', 'message=320,240'), [470, 428], false), /event\/query/);
assert.throws(() => checkLockedCursor(locked.replaceAll('ok=1,1,1', 'ok=0,1,1'), [470, 428], false), /query failed/);
console.log('PASS pointer-lock expected-motion, delayed-query and button rejection controls');

const warp = 'INPUT_WARP client=320,240 screen=404,330\n'
    + 'INPUT_WARP_QUERY elapsed=1000 ok=1,1 client=320,240\n'
    + 'INPUT_WARP_QUERY elapsed=1500 ok=1,1 client=320,240\n';
assert(checkWarpTrace(warp).passed);
assert.throws(() => checkWarpTrace(warp.replaceAll('client=320,240\n', 'client=72,72\n')), /Stale cursor/);
assert.throws(() => checkWarpTrace(warp.replace('elapsed=1000', 'elapsed=500')), /Invalid delayed/);
assert.throws(() => checkWarpTrace(warp.replaceAll('ok=1,1', 'ok=0,1')), /Invalid delayed/);
assert.throws(() => checkWarpTrace(warp.split('\n').slice(0, 2).join('\n')), /Missing delayed/);
console.log('PASS delayed warp query and stale/early/failed/missing rejection controls');
