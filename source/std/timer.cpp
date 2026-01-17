#include "timer.h"

#if defined(__3DS__)
    #include <3ds.h>
    
    uint64_t time_us_64_p(void) {
        return osGetTime()* 1000ULL;
    }

    void sleep_us_p(uint64_t us) {
        svcSleepThread(us * 1000ULL);
    }
#else
    #include <chrono>
    #include <thread>

    uint64_t time_us_64_p(void) {
        using clock = std::chrono::steady_clock;
        return (uint64_t)std::chrono::duration_cast<std::chrono::microseconds>(
                   clock::now().time_since_epoch())
            .count();
    }

    void sleep_us_p(uint64_t us) {
        std::this_thread::sleep_for(std::chrono::microseconds(us));
    }
#endif

