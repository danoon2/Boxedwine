/* Browser-runner control: a passing summary must not hide a later error exit. */
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    const char *group;
    int failures = 0, status = 0;

    if (argc != 2) return 64;
    group = argv[1];
    if (!strcmp(group, "late-failure")) status = 7;
    else if (!strcmp(group, "known-failure")) failures = status = 1;
    else if (!strcmp(group, "wrong-failure-status")) { failures = 1; status = 3; }
    else if (strcmp(group, "pass")) return 64;

    if (failures)
        puts("graphics_exit_status_probe.c:100: Test failed: deliberate harness control");
    printf("0020:%s: 1 tests executed (0 marked as todo, %d failures), 0 skipped.\n",
            group, failures);
    fflush(stdout);
    return status;
}
