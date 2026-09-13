// Standalone test of the production counter bank. No emulator or GPU is used.
#include <array>
#include <atomic>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <set>
#include <string>
#include <thread>
#include <GL/gl.h>
#include <GL/glext.h>

// Including the counter bank must preserve the caller's dispatch macros.
#define GL_FUNCTION(...) 11
#define GL_FUNCTION_FMT(...) 12
#define GL_FUNCTION_CUSTOM(...) 13
#define GL_EXT_FUNCTION(...) 14
#include "../../source/opengl/glCounters.h"
static_assert(GL_FUNCTION() == 11 && GL_FUNCTION_FMT() == 12
    && GL_FUNCTION_CUSTOM() == 13 && GL_EXT_FUNCTION() == 14);
#undef GL_FUNCTION
#undef GL_FUNCTION_FMT
#undef GL_FUNCTION_CUSTOM
#undef GL_EXT_FUNCTION

static void require(bool value, const char* message) {
    if (!value) {
        std::fprintf(stderr, "FAIL: %s\n", message);
        std::exit(1);
    }
}

int main() {
    GLCallCounters bank;
    GLCallCounters other;
    std::set<std::string> names;
    for (unsigned i = 0; i < GLCallCounters::count; ++i) {
        require(bank.get(i) == 0, "new bank is not zero initialized");
        const char* name = GLCallCounters::name(i);
        if (name) require(names.insert(name).second, "distinct call indices share a diagnostic name");
    }
    require(names.size() > 3000, "GL name table is incomplete");
    const auto checkName = [](unsigned index, const char* expected) {
        const char* actual = GLCallCounters::name(index);
        require(actual && std::strcmp(actual, expected) == 0, expected);
    };
    checkName(ClearIndex, "glClearIndex");
    checkName(Finish, "glFinish");
    checkName(Flush, "glFlush");
    checkName(DrawArrays, "glDrawArrays");
    checkName(CompileShader, "glCompileShader");
    checkName(GetTextureSubImage, "glGetTextureSubImage");
    checkName(kXSwapBuffers, "glXSwapBuffers");
    checkName(kEglMakeCurrent, "eglMakeCurrent");
    checkName(kEglWaitClient, "eglWaitClient");
    checkName(kGlProcAddressAvailable, "glProcAddressAvailable");
    checkName(GLCallCounters::contextChanges, "hostContextChanges");
    checkName(GLCallCounters::mainThreadDispatches, "mainThreadDispatches");
    for (unsigned invalid : {GLCallCounters::count, UINT_MAX}) {
        bank.record(invalid);
        require(bank.get(invalid) == 0 && !GLCallCounters::name(invalid), "invalid index is not safely ignored");
    }
    bank.record(ClearIndex);
    bank.record(GLCallCounters::mainThreadDispatches);
    require(bank.get(ClearIndex) == 1 && bank.get(GLCallCounters::mainThreadDispatches) == 1,
        "first or last counter index is not counted");

    constexpr unsigned iterations = 50000;
    const std::array<unsigned, 4> separate{Finish, Flush, kEglWaitGL, kXMakeCurrent};
    std::atomic<unsigned> ready{0};
    std::atomic<bool> start{false};
    std::array<std::thread, 4> workers;
    for (unsigned i = 0; i < workers.size(); ++i) {
        workers[i] = std::thread([&, i]() {
            ready.fetch_add(1);
            while (!start.load()) std::this_thread::yield();
            for (unsigned j = 0; j < iterations; ++j) {
                bank.record(DrawArrays);
                bank.record(separate[i]);
            }
        });
    }
    while (ready.load() != workers.size()) std::this_thread::yield();
    start.store(true);
    for (auto& worker : workers) worker.join();
    require(bank.get(DrawArrays) == workers.size() * iterations, "concurrent increments were lost");
    for (unsigned index : separate) require(bank.get(index) == iterations, "per-call counters were mixed");
    require(bank.get(ClearIndex) == 1 && bank.get(GLCallCounters::mainThreadDispatches) == 1,
        "concurrent updates changed unrelated counters");
    for (unsigned i = 0; i < GLCallCounters::count; ++i) require(other.get(i) == 0, "banks share mutable storage");
    std::printf("PASS: %zu unique names, bounds, macro preservation, bank isolation, 400000 concurrent increments\n", names.size());
    return 0;
}
