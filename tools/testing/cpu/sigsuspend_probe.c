#define _GNU_SOURCE
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <unistd.h>

static volatile sig_atomic_t received;
static void handler(int signal) { (void)signal; ++received; }
static volatile sig_atomic_t nested_passed;
static void nested_handler(int signal)
{
    sigset_t temporary, actual;
    (void)signal;
    sigemptyset(&temporary);
    if (kill(getpid(), SIGUSR2)) return;
    errno = 0;
    int result = sigsuspend(&temporary);
    int error = errno;
    if (sigprocmask(SIG_SETMASK, NULL, &actual)) return;
    nested_passed = result == -1 && error == EINTR && received == 5 &&
        sigismember(&actual, SIGUSR1) && sigismember(&actual, SIGUSR2) &&
        sigismember(&actual, SIGRTMAX);
}

int main(void)
{
    struct sigaction action;
    sigset_t original, temporary, actual;
    unsigned int failures = 0;
    memset(&action, 0, sizeof(action));
    action.sa_handler = handler;
    sigemptyset(&action.sa_mask);
    if (sigaction(SIGALRM, &action, NULL)) return 2;
    sigemptyset(&temporary);
    for (unsigned int test = 0; test < 3; ++test)
    {
        action.sa_flags = test ? SA_RESTART : 0;
        if (sigaction(SIGALRM, &action, NULL)) return 2;
        sigemptyset(&original);
        if (test) sigaddset(&original, SIGUSR1);
        if (test == 2) sigaddset(&original, SIGRTMAX);
        if (sigprocmask(SIG_SETMASK, &original, NULL)) return 2;
        struct itimerval timer = {{0, 0}, {0, 50000}};
        if (setitimer(ITIMER_REAL, &timer, NULL)) return 2;
        errno = 0;
        int result = sigsuspend(&temporary);
        int error = errno;
        if (sigprocmask(SIG_SETMASK, NULL, &actual)) return 2;
        int matches = 1;
        for (int signal = 1; signal <= SIGRTMAX; ++signal)
            if (sigismember(&original, signal) != sigismember(&actual, signal)) matches = 0;
        int passed = result == -1 && error == EINTR && received == (sig_atomic_t)test + 1 && matches;
        printf("sigsuspend case=%u return=%d errno=%d handled=%d restored=%d %s\n",
               test, result, error, received, matches, passed ? "PASS" : "FAIL");
        fflush(stdout);
        failures += !passed;
    }
    /* Delivery must also work when the signal is already pending on entry. */
    if (sigaction(SIGUSR1, &action, NULL)) return 2;
    sigemptyset(&original);
    sigaddset(&original, SIGUSR1);
    if (sigprocmask(SIG_SETMASK, &original, NULL)) return 2;
    if (kill(getpid(), SIGUSR1)) return 2;
    errno = 0;
    int result = sigsuspend(&temporary);
    int error = errno;
    if (sigprocmask(SIG_SETMASK, NULL, &actual)) return 2;
    int passed = result == -1 && error == EINTR && received == 4 && sigismember(&actual, SIGUSR1);
    printf("sigsuspend pending return=%d errno=%d handled=%d restored=%d %s\n",
           result, error, received, sigismember(&actual, SIGUSR1), passed ? "PASS" : "FAIL");
    failures += !passed;
    errno = 0;
    result = syscall(SYS_rt_sigsuspend, &temporary, 6);
    error = errno;
    if (sigprocmask(SIG_SETMASK, NULL, &actual)) return 2;
    passed = result == -1 && error == EINVAL && sigismember(&actual, SIGUSR1);
    printf("sigsuspend invalid-size return=%d errno=%d mask-unchanged=%d %s\n",
           result, error, sigismember(&actual, SIGUSR1), passed ? "PASS" : "FAIL");
    failures += !passed;
    action.sa_handler = handler;
    if (sigaction(SIGUSR2, &action, NULL)) return 2;
    action.sa_handler = nested_handler;
    sigaddset(&action.sa_mask, SIGUSR2);
    if (sigaction(SIGUSR1, &action, NULL)) return 2;
    sigaddset(&original, SIGRTMAX);
    sigaddset(&temporary, SIGRTMAX);
    if (sigprocmask(SIG_SETMASK, &original, NULL)) return 2;
    if (kill(getpid(), SIGUSR1)) return 2;
    errno = 0;
    result = sigsuspend(&temporary);
    error = errno;
    if (sigprocmask(SIG_SETMASK, NULL, &actual)) return 2;
    passed = result == -1 && error == EINTR && nested_passed &&
        sigismember(&actual, SIGUSR1) && sigismember(&actual, SIGRTMAX) &&
        !sigismember(&actual, SIGUSR2);
    printf("sigsuspend nested return=%d errno=%d inner=%d %s\n",
           result, error, nested_passed, passed ? "PASS" : "FAIL");
    failures += !passed;
    return failures ? 1 : 0;
}
