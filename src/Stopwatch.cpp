#include <Stopwatch.hpp>

void Stopwatch::start() {
    start_ = lapStart_ = std::chrono::system_clock::now();
}

double Stopwatch::lap() {
    auto now = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = now - lapStart_;
    lapStart_ = now;
    return diff.count();
}

double Stopwatch::stop() {
    auto now = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = now - start_;
    return diff.count();
}
