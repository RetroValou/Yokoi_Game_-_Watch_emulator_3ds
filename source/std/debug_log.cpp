#include "debug_log.h"

#include <cstdio>
#include <cstring>
#include <ctime>

#if defined(__3DS__)
#include <3ds.h>
#endif

#if !ENABLE_LOGGING

namespace debug_log {

void init() {}
void vwrite(const char* /*fmt*/, va_list /*args*/) {}
void write(const char* /*fmt*/, ...) {}
void flush() {}
void close() {}

} // namespace debug_log

#else

namespace debug_log {
namespace {

static FILE* g_file = nullptr;
static const char* kLogPath = "sdmc:/3ds/yokoi_log.txt";

static void write_banner() {
    if (!g_file) return;
#if defined(__3DS__)
    const u64 ms = osGetTime();
    std::fprintf(g_file, "\n===== Yokoi start (ms=%llu) =====\n", (unsigned long long)ms);
#else
    const std::time_t t = std::time(nullptr);
    std::fprintf(g_file, "\n===== Yokoi start (t=%lld) =====\n", (long long)t);
#endif
    std::fflush(g_file);
}

} // namespace

void init() {
#if defined(__3DS__)
    // Best-effort: ensure FS services are up before using sdmc:/ paths.
    // Do NOT call romfsInit() here; it's unrelated to SD logging and can be unsafe
    // to invoke early in startup.
    static bool fs_ok = false;
    if (!fs_ok) {
        Result rc = fsInit();
        if (R_SUCCEEDED(rc)) {
            fs_ok = true;
        }
    }
#endif

    if (g_file) return;
    g_file = std::fopen(kLogPath, "ab");
    if (!g_file) return;
    write_banner();
}

void vwrite(const char* fmt, va_list args) {
    if (!fmt) return;
    if (!g_file) init();
    if (!g_file) return;

    std::vfprintf(g_file, fmt, args);
    std::fputc('\n', g_file);
    std::fflush(g_file);
}

void write(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vwrite(fmt, args);
    va_end(args);
}

void flush() {
    if (g_file) std::fflush(g_file);
}

void close() {
    if (!g_file) return;
    std::fflush(g_file);
    std::fclose(g_file);
    g_file = nullptr;
}

} // namespace debug_log

#endif
