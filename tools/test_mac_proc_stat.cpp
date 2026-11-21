/* Copyright (C) 2026 The BoxedWine Team. GPL-2.0-or-later. */
#include "boxedwine.h"
#include <array>
#include <sstream>
#include <ctime>

// The small native harness links the production platform/file/string code.
// An unexpected production panic must fail the test, without starting SDL.
void kpanic(const char* message) {
    fprintf(stderr, "%s\n", message);
    abort();
}

int main(int argc, char** argv) {
    assert(argc == 2);
    BReadFile missing((std::string(argv[1]) + "/missing").c_str());
    assert(!missing.isOpen());
    assert(missing.readAll().isEmpty());
    BReadFile unopened;
    assert(unopened.readAll().isEmpty());

    std::string filename = std::string(argv[1]) + "/input";
    std::string content("first\nsecond\0last", 17);
    std::ofstream(filename, std::ios::binary).write(content.data(), content.size());
    BReadFile file(filename.c_str());
    assert(file.isOpen());
    BString data = file.readAll();
    assert(data.length() == content.size());
    assert(!memcmp(data.c_str(), content.data(), content.size()));
    assert(file.readAll().isEmpty());
    file.close();
    assert(file.readAll().isEmpty());

    for (int repeat = 0; repeat < 10; ++repeat) {
        std::istringstream input(Platform::procStat().c_str());
        std::array<U64, 10> aggregate{}, sum{};
        std::string line;
        unsigned int cpus = 0;
        bool gotAggregate = false, gotBootTime = false;
        while (std::getline(input, line)) {
            std::istringstream fields(line);
            std::string name;
            fields >> name;
            if (name.compare(0, 3, "cpu") == 0) {
                std::array<U64, 10> ticks{};
                for (auto& tick : ticks) {
                    assert(fields >> tick);
                }
                if (name == "cpu") {
                    assert(!gotAggregate && cpus == 0);
                    aggregate = ticks;
                    gotAggregate = true;
                } else {
                    assert(name == "cpu" + std::to_string(cpus++));
                    for (size_t i = 0; i < sum.size(); ++i) {
                        sum[i] += ticks[i];
                    }
                }
            } else if (name == "btime") {
                U64 bootTime = 0;
                assert(fields >> bootTime);
                assert(bootTime > 0 && bootTime <= (U64)time(nullptr));
                gotBootTime = true;
            }
        }
        assert(gotAggregate && gotBootTime && cpus > 0);
        assert(aggregate == sum);
        assert(sum[0] + sum[1] + sum[2] + sum[3] > 0);
    }
    puts("PASS: missing-file reads and macOS /proc/stat snapshots");
}
