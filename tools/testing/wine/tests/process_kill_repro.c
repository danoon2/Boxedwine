/* Copyright (C) 2026 The BoxedWine Team. GPL-2.0-or-later. */
#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static unsigned tests, failures;
#define CHECK(condition, message) do { ++tests; if (!(condition)) { ++failures; \
    printf("process_kill_repro.c:%d: Test failed: cycle %u: %s (errno %d)\n", __LINE__, cycle, message, errno); } } while (0)

int main(void)
{
    unsigned cycle;
    setbuf(stdout, NULL);
    for (cycle = 1; cycle <= 4; ++cycle)
    {
        int ready[2], sockets[2], fd, status = 0, unlocked = 0, eof = 0;
        unsigned attempt;
        pid_t child, reaped;
        char message = 0, path[128];
        struct flock lock = { .l_type = F_WRLCK, .l_whence = SEEK_SET, .l_len = 1 };
        int result = pipe(ready);
        CHECK(!result, "create handshake pipe");
        if (result) break;
        result = socketpair(AF_UNIX, SOCK_STREAM, 0, sockets);
        CHECK(!result, "create socket pair");
        if (result) break;
        snprintf(path, sizeof(path), "/tmp/boxedwine-kill-%ld-%u", (long)getpid(), cycle);
        fd = open(path, O_CREAT | O_EXCL | O_RDWR, 0600);
        CHECK(fd >= 0, "create locked file");
        if (fd < 0) break;
        child = fork();
        if (!child)
        {
            close(ready[0]);
            close(sockets[0]);
            message = fcntl(fd, F_SETLK, &lock) ? 'E' : 'R';
            if (write(ready[1], &message, 1) != 1) _exit(2);
            /* Even cycles exercise deferred deletion of the current thread. */
            if (!(cycle & 1))
            {
                if (read(sockets[1], &message, 1) != 1) _exit(3);
                kill(getpid(), SIGKILL);
            }
            for (;;) pause();
        }
        CHECK(child > 0, "fork lock owner");
        if (child < 0) break;
        close(ready[1]);
        close(sockets[1]);
        CHECK(!fcntl(sockets[0], F_SETFL, O_NONBLOCK), "set socket nonblocking");
        CHECK(read(ready[0], &message, 1) == 1 && message == 'R', "child acquired lock");
        CHECK(!fcntl(fd, F_GETLK, &lock) && lock.l_type == F_WRLCK && lock.l_pid == child,
            "lock belongs to child before kill");
        CHECK((cycle & 1) ? !kill(child, SIGKILL) : write(sockets[0], "K", 1) == 1,
            "request external or self SIGKILL");
        /* A zombie must release its resources before the parent reaps it.
         * Give asynchronous host threads a bounded opportunity to exit. */
        for (attempt = 0; attempt < 200 && (!unlocked || !eof); ++attempt)
        {
            lock.l_type = F_WRLCK;
            unlocked = !fcntl(fd, F_GETLK, &lock) && lock.l_type == F_UNLCK;
            eof = recv(sockets[0], &message, 1, 0) == 0;
            if (!unlocked || !eof) usleep(10000);
        }
        CHECK(unlocked, "file lock released before waitpid");
        CHECK(eof, "socket peer closed before waitpid");
        /* Socket EOF can precede publication of the wait status on Linux too. */
        for (attempt = 0; attempt < 200; ++attempt)
        {
            reaped = waitpid(child, &status, WNOHANG);
            if (reaped) break;
            usleep(10000);
        }
        CHECK(reaped == child, "killed child is reapable");
        CHECK(WIFSIGNALED(status) && WTERMSIG(status) == SIGKILL, "waitpid reports SIGKILL");
        close(fd);
        close(sockets[0]);
        close(ready[0]);
        unlink(path);
    }
    printf("0000:process-kill: %u tests executed (0 marked as todo, %u failures), 0 skipped.\n", tests, failures);
    return failures ? 1 : 0;
}
