#include <cassert>
#include <chrono>
#include <future>
#include <iostream>
#include <ratio>
#include <string>
#include <vector>

namespace {

using clock_t = std::chrono::high_resolution_clock;
using duration_t = std::chrono::duration<double, std::nano>;

auto
now()
{
    return clock_t::now();
}

void
print(const std::string& prefix, duration_t d)
{
    std::cout << prefix << ": " << std::chrono::duration_cast<std::chrono::milliseconds>(d).count()
              << " ms"
              << "\n";
}

} // namespace

int
main(int argc, char* argv[])
{
    assert(argc == 3);
    const auto max_clients = std::atoi(argv[1]);
    const auto cmd = argv[2];

    const auto total_start = now();
    std::vector<std::future<duration_t>> tasks;
    for (auto i = 0; i < max_clients; ++i) {
        tasks.push_back(std::async(std::launch::async, [&cmd]() {
            const auto start = now();
            std::system(cmd);
            return duration_t(now() - start);
        }));
    }

    auto exec = duration_t();
    for (auto& t : tasks) {
        exec += t.get();
    }

    print("exec time", exec);
    print("total time", now() - total_start);
}
