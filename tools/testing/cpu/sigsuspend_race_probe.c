#define _GNU_SOURCE
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdio.h>
#include <unistd.h>

static pthread_t receiver;
static atomic_int requested;
static volatile sig_atomic_t received;
static int process_signal;
static void handler(int signal) { (void)signal; ++received; }
static void timeout_handler(int signal) {
    (void)signal;
    const char message[] = "sigsuspend race TIMEOUT\n";
    ssize_t ignored = write(STDOUT_FILENO, message, sizeof(message) - 1);
    (void)ignored;
    _exit(3);
}
static void *sender(void *unused)
{
    (void)unused;
    for (int i = 1; i <= 10000; ++i)
    {
        while (atomic_load_explicit(&requested, memory_order_acquire) < i) sched_yield();
        if (process_signal) kill(getpid(), SIGUSR1);
        else pthread_kill(receiver, SIGUSR1);
    }
    return NULL;
}
int main(int argc, char **argv)
{
    (void)argv;
    process_signal = argc > 1;
    struct sigaction action = {0};
    sigset_t mask, temporary;
    action.sa_handler = handler;
    action.sa_flags = SA_RESTART;
    sigemptyset(&action.sa_mask);
    sigaction(SIGUSR1, &action, NULL);
    action.sa_handler = timeout_handler;
    sigaction(SIGALRM, &action, NULL);
    sigemptyset(&mask); sigaddset(&mask, SIGUSR1);
    sigemptyset(&temporary);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);
    receiver = pthread_self();
    pthread_t thread;
    alarm(30);
    if (pthread_create(&thread, NULL, sender, NULL)) return 2;
    for (int i = 1; i <= 10000; ++i)
    {
        atomic_store_explicit(&requested, i, memory_order_release);
        errno = 0;
        int result = sigsuspend(&temporary);
        if (result != -1 || errno != EINTR || received != i)
        {
            printf("sigsuspend race FAIL iteration=%d result=%d errno=%d received=%d\n", i, result, errno, received);
            return 1;
        }
        if (i % 1000 == 0) { printf("sigsuspend race progress=%d\n", i); fflush(stdout); }
    }
    pthread_join(thread, NULL);
    alarm(0);
    puts("sigsuspend race 10000 deliveries PASS");
    return 0;
}
