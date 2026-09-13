/* Copyright (C) 2026 The BoxedWine Team. GPL-2.0-or-later. */
#define _GNU_SOURCE
#include <errno.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

static unsigned tests, failures;
static unsigned char child_stack[65536] __attribute__((aligned(16)));
struct shared_state {
    int ready[2], go[2], self_kill;
    volatile unsigned value;
};
#define CHECK(condition, message) do { ++tests; if (!(condition)) { ++failures; \
    printf("process_shared_exit_repro.c:%d: Test failed: cycle %u: %s (errno %d)\n", __LINE__, cycle, message, errno); } } while (0)

static int child_main(void *argument)
{
    struct shared_state *state = argument;
    char byte;
    state->value = 0x13579bdf;
    if (write(state->ready[1], "R", 1) != 1) return 2;
    if (read(state->go[0], &byte, 1) != 1) return 3;
    if (state->self_kill) kill(getpid(), SIGKILL);
    for (;;) pause();
}

int main(void)
{
    unsigned cycle;
    setbuf(stdout, NULL);
    for (cycle = 1; cycle <= 2; ++cycle)
    {
        struct shared_state state = {.self_kill = cycle == 2};
        unsigned attempt;
        pid_t child, reaped;
        char byte = 0;
        int status = 0, result;
        result = pipe(state.ready);
        CHECK(!result, "create readiness pipe");
        if (result) break;
        result = pipe(state.go);
        CHECK(!result, "create trigger pipe");
        if (result) break;
        child = clone(child_main, child_stack + sizeof(child_stack), CLONE_VM | SIGCHLD, &state);
        CHECK(child > 0, "create child sharing the parent's memory");
        if (child < 0) break;
        CHECK(read(state.ready[0], &byte, 1) == 1 && byte == 'R', "child reached handshake");
        CHECK(state.value == 0x13579bdf, "parent observes the child's shared write");
        printf("SHARED_EXIT cycle=%u child=%ld before kill\n", cycle, (long)child);
        CHECK(state.self_kill ? write(state.go[1], "K", 1) == 1 : !kill(child, SIGKILL),
            "request external or self SIGKILL");
        for (attempt = 0; attempt < 200; ++attempt)
        {
            reaped = waitpid(child, &status, WNOHANG);
            if (reaped) break;
            usleep(10000);
        }
        CHECK(reaped == child, "shared-memory child is reapable");
        CHECK(WIFSIGNALED(status) && WTERMSIG(status) == SIGKILL, "waitpid reports SIGKILL");
        CHECK(state.value == 0x13579bdf, "parent's memory survives child exit");
        close(state.ready[0]); close(state.ready[1]);
        close(state.go[0]); close(state.go[1]);
    }
    printf("0000:process-shared-exit: %u tests executed (0 marked as todo, %u failures), 0 skipped.\n", tests, failures);
    return failures ? 1 : 0;
}
