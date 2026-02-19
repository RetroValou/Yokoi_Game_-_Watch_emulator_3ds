#include "gw_pack.h"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "segment.h"

#include "debug_log.h"
#define GWPACK_LOG(...) YOKOI_LOG(__VA_ARGS__)

namespace gw_pack {
namespace {

constexpr uint32_t kPackMagic = 0x31504B59; // 'YKP1' little-endian

// Pack format versions:
// - v1: original header (no content version)
// - v2: adds a per-pack "content_version" used to detect outdated packs
// - v3: extends game entries with explicit manufacturer id
constexpr uint32_t kPackVersionV1 = 1;
constexpr uint32_t kPackVersionV2 = 2;
constexpr uint32_t kPackVersionV3 = 3;

// Pack "platform" ids (written by CONVERT_ROM/convert_3ds.py).
// These are used to prevent loading the wrong pack for a given frontend.
constexpr uint32_t kPlatform3ds = 1;
constexpr uint32_t kPlatformRgds = 2; // RGDS/Android runtime PNG pathing

#ifndef YOKOI_ROMPACK_REQUIRED_CONTENT_VERSION
#define YOKOI_ROMPACK_REQUIRED_CONTENT_VERSION 1
#endif

constexpr uint32_t kRequiredContentVersion = (uint32_t)YOKOI_ROMPACK_REQUIRED_CONTENT_VERSION;

struct PackHeaderV1 {
    uint32_t magic;
    uint32_t version;
    uint32_t platform;
    uint32_t game_count;
    uint32_t file_count;
    uint32_t games_offset;
    uint32_t files_offset;
    uint32_t data_offset;
};

struct PackHeaderV2 {
    uint32_t magic;
    uint32_t version;
    uint32_t platform;
    uint32_t content_version;
    uint32_t game_count;
    uint32_t file_count;
    uint32_t games_offset;
    uint32_t files_offset;
    uint32_t data_offset;
};

struct GameEntryV1 {
    uint32_t name_off, name_len;
    uint32_t ref_off, ref_len;
    uint32_t date_off, date_len;

    uint32_t rom_off, rom_size;
    uint32_t melody_off, melody_size;

    uint32_t path_segment_off, path_segment_len;
    uint32_t segments_off, segments_count;

    uint32_t segment_info_off, segment_info_count;     // uint16_t count

    uint32_t path_background_off, path_background_len;
    uint32_t background_info_off, background_info_count; // uint16_t count

    uint32_t path_console_off, path_console_len;
    uint32_t console_info_off, console_info_count; // uint16_t count
};

// v3: same as v1 plus a trailing uint32 manufacturer id.
struct GameEntryV2 {
    uint32_t name_off, name_len;
    uint32_t ref_off, ref_len;
    uint32_t date_off, date_len;

    uint32_t rom_off, rom_size;
    uint32_t melody_off, melody_size;

    uint32_t path_segment_off, path_segment_len;
    uint32_t segments_off, segments_count;

    uint32_t segment_info_off, segment_info_count;     // uint16_t count

    uint32_t path_background_off, path_background_len;
    uint32_t background_info_off, background_info_count; // uint16_t count

    uint32_t path_console_off, path_console_len;
    uint32_t console_info_off, console_info_count; // uint16_t count

    uint32_t manufacturer;
};

struct FileEntryV1 {
    uint32_t name_off, name_len;
    uint32_t data_off, data_size;
};

#pragma pack(push, 1)
struct SegmentDiskV1 {
    uint8_t id0;
    uint8_t id1;
    uint8_t id2;

    int32_t pos_scr_x;
    int32_t pos_scr_y;

    uint16_t pos_tex_x;
    uint16_t pos_tex_y;
    uint16_t size_tex_x;
    uint16_t size_tex_y;

    uint8_t color_index;
    uint8_t screen;
};
#pragma pack(pop)

struct FileSlice {
    uint32_t off = 0;
    uint32_t size = 0;
};

struct GameStorage {
    std::vector<uint8_t> rom;
    std::vector<uint8_t> melody;
    std::vector<Segment> segments;
    std::vector<uint16_t> segment_info;
    std::vector<uint16_t> background_info;
    std::vector<uint16_t> console_info;
};

struct GameRecord {
    GameStorage storage;
    std::unique_ptr<GW_rom> gw;
};

static std::vector<GameRecord> g_games;
static std::unordered_map<std::string, FileSlice> g_files;
static bool g_loaded = false;

#if defined(__ANDROID__)
// Android: load the entire pack into memory for stable pointers and fewer IO calls.
static std::vector<uint8_t> g_pack_blob;
static size_t g_pack_size = 0;
#else
// 3DS/others: stream from disk to keep memory usage low.
static FILE* g_pack_file = nullptr;
static size_t g_pack_size = 0;
static std::vector<uint8_t> g_file_scratch;
#endif

static bool bounds_ok(size_t off, size_t len, size_t total) {
    if (off > total) return false;
    if (len > total) return false;
    if (off + len > total) return false;
    return true;
}

static constexpr uint32_t expected_platform_id() {
#if defined(__3DS__)
    return kPlatform3ds;
#elif defined(__ANDROID__)
    return kPlatformRgds;
#else
    // Unknown/desktop builds: allow any.
    return 0;
#endif
}

static const char* platform_name(uint32_t id) {
    switch (id) {
        case kPlatform3ds: return "3ds";
        case kPlatformRgds: return "rgds";
        default: return "unknown";
    }
}

#if defined(__ANDROID__)
static bool file_read_at(uint32_t off, void* dst, size_t len, size_t total, std::string* error_out, const char* what) {
    if (!bounds_ok(off, len, total)) {
        if (error_out) *error_out = std::string(what ? what : "read") + " out of range";
        return false;
    }
    if (len == 0) return true;
    if (g_pack_blob.empty() || !dst) {
        if (error_out) *error_out = "pack blob not loaded";
        return false;
    }
    std::memcpy(dst, g_pack_blob.data() + off, len);
    return true;
}
#else
static bool file_read_at(uint32_t off, void* dst, size_t len, size_t total, std::string* error_out, const char* what) {
    if (!bounds_ok(off, len, total)) {
        if (error_out) *error_out = std::string(what ? what : "read") + " out of range";
        return false;
    }
    if (!g_pack_file) {
        if (error_out) *error_out = "pack file not open";
        return false;
    }
    if (std::fseek(g_pack_file, (long)off, SEEK_SET) != 0) {
        if (error_out) *error_out = std::string("fseek failed for ") + (what ? what : "read");
        return false;
    }
    const size_t got = std::fread(dst, 1, len, g_pack_file);
    if (got != len) {
        if (error_out) *error_out = std::string("fread failed for ") + (what ? what : "read");
        return false;
    }
    return true;
}
#endif

static std::string file_read_string(uint32_t off, uint32_t len, size_t total) {
    if (len == 0) return std::string();
    if (!bounds_ok(off, len, total)) return std::string();
    std::string out;
    out.resize((size_t)len);
    if (!file_read_at(off, out.data(), (size_t)len, total, nullptr, "string")) return std::string();
    return out;
}

} // namespace

bool load(const std::string& path, std::string* error_out) {
    unload();

    GWPACK_LOG("gw_pack: load '%s'", path.c_str());

#if defined(__ANDROID__)
    FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) {
        const int e = errno;
        if (error_out) {
            *error_out = "open failed: " + path;
            if (e != 0) {
                *error_out += " (";
                *error_out += std::strerror(e);
                *error_out += ")";
            }
        }
        return false;
    }

    std::fseek(f, 0, SEEK_END);
    long fsize = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);
    if (fsize <= 0) {
        std::fclose(f);
        if (error_out) *error_out = "empty file: " + path;
        return false;
    }

    g_pack_size = (size_t)fsize;
    const size_t total = g_pack_size;
    g_pack_blob.resize(total);
    const size_t got = std::fread(g_pack_blob.data(), 1, total, f);
    std::fclose(f);
    if (got != total) {
        unload();
        if (error_out) *error_out = "fread failed for pack";
        return false;
    }
#else
    g_pack_file = std::fopen(path.c_str(), "rb");
    if (!g_pack_file) {
        const int e = errno;
        if (error_out) {
            *error_out = "open failed: " + path;
            if (e != 0) {
                *error_out += " (";
                *error_out += std::strerror(e);
                *error_out += ")";
            }
        }
        return false;
    }

    std::fseek(g_pack_file, 0, SEEK_END);
    long fsize = std::ftell(g_pack_file);
    std::fseek(g_pack_file, 0, SEEK_SET);
    if (fsize <= 0) {
        std::fclose(g_pack_file);
        g_pack_file = nullptr;
        if (error_out) *error_out = "empty file: " + path;
        return false;
    }

    g_pack_size = (size_t)fsize;
    const size_t total = g_pack_size;
#endif
    GWPACK_LOG("gw_pack: file size %u bytes", (unsigned)total);

    if (total < 8) {
        unload();
        if (error_out) *error_out = "too small";
        return false;
    }

    uint32_t magic = 0;
    uint32_t version = 0;
    if (!file_read_at(0, &magic, sizeof(uint32_t), total, error_out, "magic")) {
        unload();
        return false;
    }
    if (!file_read_at(sizeof(uint32_t), &version, sizeof(uint32_t), total, error_out, "version")) {
        unload();
        return false;
    }

    if (magic != kPackMagic) {
        unload();
        if (error_out) *error_out = "bad magic";
        GWPACK_LOG("gw_pack: bad magic");
        return false;
    }

    uint32_t platform = 0;
    uint32_t content_version = 0;
    uint32_t game_count = 0;
    uint32_t file_count = 0;
    uint32_t games_offset = 0;
    uint32_t files_offset = 0;
    uint32_t data_offset = 0;

    if (version == kPackVersionV1) {
        unload();
        if (error_out) *error_out = "unsupported pack format v1 (regenerate/re-import a v2 pack)";
        GWPACK_LOG("gw_pack: v1 rejected");
        return false;
    } else if (version == kPackVersionV2 || version == kPackVersionV3) {
        PackHeaderV2 hdr{};
        if (!file_read_at(0, &hdr, sizeof(PackHeaderV2), total, error_out, "header")) {
            unload();
            return false;
        }
        platform = hdr.platform;
        content_version = hdr.content_version;
        game_count = hdr.game_count;
        file_count = hdr.file_count;
        games_offset = hdr.games_offset;
        files_offset = hdr.files_offset;
        data_offset = hdr.data_offset;
        GWPACK_LOG(
            "gw_pack: v%u platform=%u content=%u games=%u files=%u",
            (unsigned)version,
            (unsigned)platform,
            (unsigned)content_version,
            (unsigned)game_count,
            (unsigned)file_count);
    } else {
        unload();
        if (error_out) *error_out = "unsupported pack format version";
        GWPACK_LOG("gw_pack: unsupported version %u", (unsigned)version);
        return false;
    }

    // Validate platform id for known frontends (reject wrong pack for device).
    {
        const uint32_t want = expected_platform_id();
        if (want != 0 && platform != want) {
            unload();
            if (error_out) {
                *error_out = "wrong pack platform (have " + std::to_string(platform) + ":" + platform_name(platform) +
                             ", need " + std::to_string(want) + ":" + platform_name(want) + ")";
            }
            GWPACK_LOG(
                "gw_pack: wrong platform have=%u(%s) need=%u(%s)",
                (unsigned)platform,
                platform_name(platform),
                (unsigned)want,
                platform_name(want));
            return false;
        }
    }

    (void)data_offset;

    if (content_version < kRequiredContentVersion) {
        unload();
        if (error_out) {
            *error_out = "rom pack too old (have " + std::to_string(content_version) + ", need " +
                         std::to_string(kRequiredContentVersion) + ")";
        }
        GWPACK_LOG(
            "gw_pack: content too old have=%u need=%u",
            (unsigned)content_version,
            (unsigned)kRequiredContentVersion);
        return false;
    }

    const size_t game_entry_size = (version >= kPackVersionV3) ? sizeof(GameEntryV2) : sizeof(GameEntryV1);
    const size_t games_bytes = (size_t)game_count * game_entry_size;
    const size_t files_bytes = (size_t)file_count * sizeof(FileEntryV1);

    if (!bounds_ok(games_offset, games_bytes, total) || !bounds_ok(files_offset, files_bytes, total)) {
        unload();
        if (error_out) *error_out = "bad offsets";
        GWPACK_LOG("gw_pack: bad offsets games_off=%u files_off=%u", (unsigned)games_offset, (unsigned)files_offset);
        return false;
    }

    // Build file table (store offsets/sizes; file bytes are streamed on demand).
    g_files.clear();
    if (file_count > 0) {
        for (uint32_t i = 0; i < file_count; i++) {
            FileEntryV1 fe{};
            if (!file_read_at(
                    files_offset + (size_t)i * sizeof(FileEntryV1),
                    &fe,
                    sizeof(FileEntryV1),
                    total,
                    error_out,
                    "file entry")) {
                unload();
                return false;
            }

            const std::string name = file_read_string(fe.name_off, fe.name_len, total);
            if (name.empty()) {
                unload();
                if (error_out) *error_out = "file name read failed";
                GWPACK_LOG("gw_pack: file name read failed i=%u", (unsigned)i);
                return false;
            }
            if (!bounds_ok(fe.data_off, fe.data_size, total)) {
                unload();
                if (error_out) *error_out = "file data out of range";
                GWPACK_LOG("gw_pack: file data out of range '%s'", name.c_str());
                return false;
            }
            g_files.emplace(name, FileSlice{fe.data_off, fe.data_size});
        }
    }

    // Build games.
    g_games.clear();
    g_games.reserve(game_count);

    for (uint32_t i = 0; i < game_count; i++) {
        GameRecord rec;

        GameEntryV1 ge{};
        uint32_t manufacturer_id = GW_rom::MANUFACTURER_NINTENDO;
        if (version >= kPackVersionV3) {
            GameEntryV2 ge2{};
            if (!file_read_at(
                    games_offset + (size_t)i * game_entry_size,
                    &ge2,
                    sizeof(GameEntryV2),
                    total,
                    error_out,
                    "game entry")) {
                unload();
                return false;
            }
            std::memcpy(&ge, &ge2, sizeof(GameEntryV1));
            manufacturer_id = ge2.manufacturer;
        } else {
            if (!file_read_at(
                    games_offset + (size_t)i * game_entry_size,
                    &ge,
                    sizeof(GameEntryV1),
                    total,
                    error_out,
                    "game entry")) {
                unload();
                return false;
            }
        }

        const std::string name = file_read_string(ge.name_off, ge.name_len, total);
        const std::string ref = file_read_string(ge.ref_off, ge.ref_len, total);
        const std::string date = file_read_string(ge.date_off, ge.date_len, total);
        const std::string path_segment = file_read_string(ge.path_segment_off, ge.path_segment_len, total);
        const std::string path_background = file_read_string(ge.path_background_off, ge.path_background_len, total);
        const std::string path_console = file_read_string(ge.path_console_off, ge.path_console_len, total);

        if (name.empty()) {
            unload();
            if (error_out) *error_out = "game name missing";
            GWPACK_LOG("gw_pack: game name missing i=%u", (unsigned)i);
            return false;
        }

        // ROM bytes.
        if (!bounds_ok(ge.rom_off, ge.rom_size, total)) {
            unload();
            if (error_out) *error_out = "rom out of range";
            GWPACK_LOG("gw_pack: rom out of range i=%u", (unsigned)i);
            return false;
        }
        rec.storage.rom.resize(ge.rom_size);
        if (ge.rom_size > 0 && !file_read_at(ge.rom_off, rec.storage.rom.data(), ge.rom_size, total, error_out, "rom")) {
            unload();
            return false;
        }

        // Melody bytes.
        if (ge.melody_size > 0) {
            if (!bounds_ok(ge.melody_off, ge.melody_size, total)) {
                unload();
                if (error_out) *error_out = "melody out of range";
                GWPACK_LOG("gw_pack: melody out of range i=%u", (unsigned)i);
                return false;
            }
            rec.storage.melody.resize(ge.melody_size);
            if (!file_read_at(
                    ge.melody_off,
                    rec.storage.melody.data(),
                    ge.melody_size,
                    total,
                    error_out,
                    "melody")) {
                unload();
                return false;
            }
        }

        // Segments.
        const size_t seg_bytes = (size_t)ge.segments_count * sizeof(SegmentDiskV1);
        if (!bounds_ok(ge.segments_off, seg_bytes, total)) {
            unload();
            if (error_out) *error_out = "segments out of range";
            GWPACK_LOG("gw_pack: segments out of range i=%u", (unsigned)i);
            return false;
        }
        if (ge.segments_count > 0) {
            rec.storage.segments.clear();
            rec.storage.segments.resize(ge.segments_count);
            for (uint32_t si = 0; si < ge.segments_count; si++) {
                SegmentDiskV1 sd{};
                if (!file_read_at(
                        ge.segments_off + (size_t)si * sizeof(SegmentDiskV1),
                        &sd,
                        sizeof(SegmentDiskV1),
                        total,
                        error_out,
                        "segment")) {
                    unload();
                    return false;
                }

                Segment s{};
                s.id[0] = sd.id0;
                s.id[1] = sd.id1;
                s.id[2] = sd.id2;
                s.pos_scr[0] = (int)sd.pos_scr_x;
                s.pos_scr[1] = (int)sd.pos_scr_y;
                s.pos_tex[0] = sd.pos_tex_x;
                s.pos_tex[1] = sd.pos_tex_y;
                s.size_tex[0] = sd.size_tex_x;
                s.size_tex[1] = sd.size_tex_y;
                s.color_index = sd.color_index;
                s.screen = sd.screen;
                s.state = false;
                s.buffer_state = false;
                s.index_vertex = 0;
                rec.storage.segments[si] = s;
            }
        }

        // segment_info / background_info / console_info.
        auto read_u16_array = [&](uint32_t off, uint32_t count, std::vector<uint16_t>& dst, const char* what) -> bool {
            dst.clear();
            if (count == 0) return true;
            const size_t bytes = (size_t)count * sizeof(uint16_t);
            if (!bounds_ok(off, bytes, total)) {
                if (error_out) *error_out = std::string(what) + " out of range";
                return false;
            }
            dst.resize(count);
            return file_read_at(off, dst.data(), bytes, total, error_out, what);
        };

        if (!read_u16_array(ge.segment_info_off, ge.segment_info_count, rec.storage.segment_info, "segment_info")) {
            unload();
            return false;
        }
        if (!read_u16_array(ge.background_info_off, ge.background_info_count, rec.storage.background_info, "background_info")) {
            unload();
            return false;
        }
        if (!read_u16_array(ge.console_info_off, ge.console_info_count, rec.storage.console_info, "console_info")) {
            unload();
            return false;
        }

        // Create GW_rom (points into GameStorage vectors).
        const uint8_t* rom_ptr = rec.storage.rom.empty() ? nullptr : rec.storage.rom.data();
        const size_t rom_size = rec.storage.rom.size();
        const uint8_t* melody_ptr = rec.storage.melody.empty() ? nullptr : rec.storage.melody.data();
        const size_t melody_size = rec.storage.melody.size();

        const Segment* seg_ptr = rec.storage.segments.empty() ? nullptr : rec.storage.segments.data();
        const size_t seg_count = rec.storage.segments.size();

        const uint16_t* seg_info_ptr = rec.storage.segment_info.empty() ? nullptr : rec.storage.segment_info.data();
        const uint16_t* bg_info_ptr = rec.storage.background_info.empty() ? nullptr : rec.storage.background_info.data();
        const uint16_t* cs_info_ptr = rec.storage.console_info.empty() ? nullptr : rec.storage.console_info.data();

        rec.gw = std::unique_ptr<GW_rom>(new GW_rom(
            name,
            ref,
            date,
            rom_ptr,
            rom_size,
            melody_ptr,
            melody_size,
            path_segment,
            seg_ptr,
            seg_count,
            seg_info_ptr,
            path_background,
            bg_info_ptr,
            path_console,
            cs_info_ptr,
            (uint8_t)manufacturer_id));

        g_games.push_back(std::move(rec));
    }

    g_loaded = true;
    return true;
}

void unload() {
    g_loaded = false;
    g_files.clear();
    g_games.clear();
    g_pack_size = 0;
#if defined(__ANDROID__)
    g_pack_blob.clear();
#else
    g_file_scratch.clear();
    if (g_pack_file) {
        std::fclose(g_pack_file);
        g_pack_file = nullptr;
    }
#endif
}

bool is_loaded() {
    return g_loaded;
}

size_t game_count() {
    return g_loaded ? g_games.size() : 0;
}

const GW_rom* game_at(size_t index) {
    if (!g_loaded) return nullptr;
    if (index >= g_games.size()) return nullptr;
    return g_games[index].gw.get();
}

bool get_file_bytes(const std::string& name, const uint8_t*& data, size_t& size) {
    data = nullptr;
    size = 0;
    if (!g_loaded) return false;
    auto it = g_files.find(name);
    if (it == g_files.end()) return false;

#if defined(__ANDROID__)
    if (it->second.size == 0) return false;
    if (g_pack_blob.empty()) return false;
    if (!bounds_ok(it->second.off, it->second.size, g_pack_blob.size())) return false;
    data = g_pack_blob.data() + it->second.off;
    size = (size_t)it->second.size;
    return true;
#else
    if (!g_pack_file) return false;
    if (it->second.size == 0) return false;
    g_file_scratch.resize((size_t)it->second.size);
    if (!file_read_at(it->second.off, g_file_scratch.data(), (size_t)it->second.size, g_pack_size, nullptr, "file bytes")) {
        return false;
    }
    data = g_file_scratch.data();
    size = g_file_scratch.size();
    return true;
#endif
}

} // namespace gw_pack
