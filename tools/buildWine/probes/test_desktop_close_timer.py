#!/usr/bin/env python3
"""Compile Wine's actual desktop timer code with a deterministic timer queue.

Run in WSL/Linux with a configured Wine 11 tree (no Boxedwine runtime needed):
  python3 test_desktop_close_timer.py --wine-source /path/to/wine
For an out-of-tree build add --wine-build /path/to/build. --winstation can
select an original or patched copy of server/winstation.c for an A/B check.
Only the timer queue and unrelated desktop services are stubbed. The real
add/remove desktop-thread and close-timeout functions execute in this test.
"""
import argparse
import json
from pathlib import Path
import shlex
import subprocess
import tempfile


HARNESS = r'''
static struct process manager, application, system_process;
static struct winstation station;
static struct desktop desk;
static struct timeout_user *active;
static timeout_callback callback;
static void *callback_arg;
static unsigned int generations, closed, unlinked;
static int timer_tokens[16];

#define CHECK(c, text) do { if (!(c)) { fprintf(stderr, "FAIL: %s\n", text); exit(1); } } while (0)

struct process *get_top_window_owner(struct desktop *desktop) { return &manager; }
void add_desktop_hook_count(struct desktop *desktop, struct thread *thread, int count) {}
void set_rawinput_process(struct process *process, int enable) {}
void unlink_named_object(struct object *object) { unlinked++; }
void post_desktop_message(struct desktop *desktop, unsigned int message, lparam_t wparam, lparam_t lparam)
{
    CHECK(desktop == &desk && message == WM_CLOSE, "close the unused desktop");
    closed++;
}
struct timeout_user *add_timeout_user(timeout_t when, timeout_callback func, void *arg)
{
    CHECK(!active && when == -TICKS_PER_SEC, "one desktop-close timer with its normal delay");
    CHECK(generations < 16, "bounded timer queue");
    active = (struct timeout_user *)&timer_tokens[generations++];
    callback = func;
    callback_arg = arg;
    return active;
}
void remove_timeout_user(struct timeout_user *timer)
{
    CHECK(active == timer, "cancel the active timer");
    active = NULL;
}
static void attach(struct thread *thread, struct process *process)
{
    memset(thread, 0, sizeof(*thread));
    thread->process = process;
    /* Wine attaches the desktop before incrementing running_threads. */
    add_desktop_thread(&desk, thread);
    process->running_threads++;
}
static void detach(struct thread *thread)
{
    remove_desktop_thread(&desk, thread);
    thread->process->running_threads--;
}
int main(void)
{
    struct thread owner, app, worker, another_worker, service;
    struct timeout_user *pending;
    list_init(&desk.threads);
    desk.winstation = &station;
    system_process.is_system = 1;

    attach(&owner, &manager);
    attach(&app, &application);
    CHECK(!active, "live application has no close timer");
    detach(&app);
    CHECK(active && active == desk.close_timeout, "last application arms close timer");
    pending = active;

    attach(&worker, &manager);
    CHECK(active == pending && desk.close_timeout == pending,
          "desktop manager's new thread must preserve the pending close");
    CHECK(generations == 1, "manager activity must not extend the original deadline");
    puts("PASS manager startup thread preserves the close timer");

    attach(&app, &application);
    CHECK(!active && !desk.close_timeout, "new application cancels the close timer");
    puts("PASS new application cancels the close timer");
    detach(&app);
    CHECK(active && generations == 2, "last application leaving rearms the close timer");
    pending = active;

    attach(&another_worker, &manager);
    attach(&service, &system_process);
    CHECK(active == pending && desk.close_timeout == pending, "manager and service threads preserve the timer");
    detach(&another_worker);
    CHECK(active == pending, "manager thread exit must not reset the deadline");
    CHECK(generations == 2, "no duplicate timers");
    puts("PASS manager and service activity does not postpone shutdown");

    active = NULL;
    callback(callback_arg);
    CHECK(!desk.close_timeout && closed == 1 && unlinked == 1,
          "timer unlinks the desktop and posts exactly one WM_CLOSE");
    puts("PASS unused desktop closes when the timer fires");
    return 0;
}
'''


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--wine-source', type=Path, required=True)
    parser.add_argument('--wine-build', type=Path)
    parser.add_argument('--winstation', type=Path)
    parser.add_argument('--cc', default='gcc -m32')
    args = parser.parse_args()
    source = args.wine_source.resolve()
    build = (args.wine_build or source).resolve()
    implementation = (args.winstation or source / 'server/winstation.c').resolve(strict=True)
    with tempfile.TemporaryDirectory(prefix='wine-desktop-timer-') as directory:
        folder = Path(directory)
        test = folder / 'test.c'
        test.write_text('#include ' + json.dumps(str(implementation)) + '\n' + HARNESS)
        executable = folder / 'test'
        command = shlex.split(args.cc) + [
            '-O1', '-ffunction-sections', '-fdata-sections', '-Wl,--gc-sections',
            '-D__WINESRC__', '-DWINE_UNIX_LIB', '-I' + str(source / 'server'),
            '-I' + str(build / 'include'), '-I' + str(source / 'include'),
            str(test), '-o', str(executable),
        ]
        subprocess.run(command, check=True)
        return subprocess.run([str(executable)]).returncode


if __name__ == '__main__':
    raise SystemExit(main())
