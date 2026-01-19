#pragma once

#include <cstdint>

// Android-only: updates the currently selected game in the menu.
void yokoi_menu_select_game_by_index(uint8_t idx);

// Android-only: retrieve the last selected game index for a manufacturer.
// Returns true if a valid remembered index exists for this manufacturer.
bool yokoi_menu_try_get_last_index_for_manufacturer(uint8_t manufacturer_id, uint8_t* out_idx);
