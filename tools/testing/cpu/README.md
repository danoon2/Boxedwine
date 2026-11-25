# CPU exception comparisons

`exception_probe.c` runs small, self-contained 32-bit instruction sequences.
On Windows it records VEH exception codes, addresses and saved context. On
Linux it records `siginfo_t`, trap/error numbers, EIP, EFLAGS and CR2. The
handlers resume after each sequence, so no application binaries are involved.

Build under WSL/Linux:

```sh
gcc -m32 -O0 exception_probe.c -o exception_probe
i686-w64-mingw32-gcc -O0 exception_probe.c -o exception_probe.exe
```

Run the ELF directly on Linux and in Boxedwine, and the PE directly on Windows
and under Wine in Boxedwine. An optional first argument selects a case; `*`
selects all cases. An optional second argument redirects output to a writable
file, useful for avoiding Wine console formatting. Use a fresh isolated root
for emulator runs. Prefer the dynamically linked ELF with the current Wine
filesystem; the local Ubuntu static-glibc executable fails during startup.

```sh
python3 compare_exception_probes.py hardware.log boxedwine.log
```

The comparison checks the signal/exception code, count, Linux errno, trap/error
numbers, instruction and exception addresses, RF/TF, overflow flags for INT 4
and INTO, and CR2. Relocated code addresses and unspecified incoming arithmetic
flags are excluded. Cases include zero and overflow division at 8/16/32 bits,
signed division, register/absolute/indirect/SIB memory operands, AAM, BOUND,
invalid instructions, software interrupts, faults while stepping and page
faults (unmapped, no-access and read-only pages). Linux
also tests non-present data and stack selectors through `modify_ldt`.

Validation on 2026-11-15: 65 Linux cases matched hardware, and 63 Windows cases
matched hardware with `wine11_integer_exceptions.patch`. All 890 entries in
the native fast test suite passed; the 51 exception/debug/signal entries also passed
with linear memory disabled and with safe memory enabled. These checks exercise
the x64 host interpreter and JIT; they do not certify every x86 exception or
every host architecture.

Filesystem v39 qualification also matched all 63 Windows cases using each
finished full/web ZIP, and matched 63 Windows and 65 Linux cases in Chrome
with the rebuilt single-threaded Wasm JIT and the web ZIP.

The Wine patch is included in filesystem v39. Initial testing used an
isolated `ntdll.so` overlay over filesystem v38; the published v38 ZIPs have
not been replaced. Its decoder has a separate test using the actual Wine
source, covering 16-bit addressing/default operand sizes, duplicate prefixes,
high byte registers, segment overrides, displacements and unreadable operands:

```sh
python3 ../../buildWine/probes/test_integer_exceptions.py --wine-source /path/to/wine
```

## Signal suspension

The Linux ELF probes exercise the kernel without involving Wine or a demo:

```sh
gcc -m32 -O0 sigsuspend_probe.c -o sigsuspend_probe
gcc -m32 -O2 -pthread sigsuspend_race_probe.c -o sigsuspend_race_probe
./sigsuspend_probe
./sigsuspend_race_probe
./sigsuspend_race_probe process
```

Run the same binaries from `/files/` in a fresh Boxedwine root with the full
Wine filesystem providing glibc. Check their printed PASS records as well as
normal completion; the legacy command-line recorder build returns 1 after a
normal launch without a playback script.

`sigsuspend_probe` checks EINTR with and without SA_RESTART, full 64-bit mask
restoration, already-pending signals, invalid mask size, and a nested suspension
inside a signal handler. `sigsuspend_race_probe` requires 10,000 successful
handshakes with a sender thread. Its optional `process` argument uses process
signals instead of thread signals. A 30-second watchdog reports TIMEOUT if a
signal is lost between changing the mask and registering the wait.
