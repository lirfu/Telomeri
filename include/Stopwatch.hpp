#ifndef STOPWATCH_HPP
#define STOPWATCH_HPP

#include <chrono>

class Stopwatch {
private:
    std::chrono::time_point<std::chrono::system_clock> start_;
    std::chrono::time_point<std::chrono::system_clock> lapStart_;
public:
    /** Start the timer. Resets internals to the current time point. */
    void start();

    /** Return lap time and start new lap. Doesn't affect total time.
     * @return Lap time. */
    double lap();

    /** Stop the timer and return total time.
     * @return Total elapsed time.*/
    double stop();
};

#endif
