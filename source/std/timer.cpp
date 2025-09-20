#include <3ds.h>
#include <citro3d.h>
#include <tex3ds.h>


uint64_t time_us_64_p(void) {
    return svcGetSystemTick() / CPU_TICKS_PER_USEC ;
}

void sleep_us_p(uint64_t us) {
    svcSleepThread(us * 1000ULL);
}

