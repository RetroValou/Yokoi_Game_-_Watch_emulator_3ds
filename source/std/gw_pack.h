#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include "GW_ROM.h"

namespace gw_pack {

// Loads a Yokoi ROM pack file from disk.
// Returns true on success. If false, error_out (if provided) gets a short message.
bool load(const std::string& path, std::string* error_out = nullptr);

// Releases all pack data (games + file table).
void unload();

bool is_loaded();

size_t game_count();
const GW_rom* game_at(size_t index);

// Retrieves a named blob stored in the pack (e.g. "background_Ball.png").
// Returns true if found.
// Note: data points into an internal scratch buffer and is only valid until the
// next gw_pack::get_file_bytes call (or gw_pack::unload).
bool get_file_bytes(const std::string& name, const uint8_t*& data, size_t& size);

} // namespace gw_pack
