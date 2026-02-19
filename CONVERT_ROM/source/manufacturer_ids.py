"""Manufacturer ids used by conversion scripts.

These numeric ids must match `source/std/GW_ROM.h`.
"""

from __future__ import annotations

from typing import Any


MANUFACTURER_NINTENDO: int = 0
MANUFACTURER_TRONICA: int = 1
MANUFACTURER_ELEKTRONIKA: int = 2
MANUFACTURER_TIGER: int = 3
MANUFACTURER_DAVID_AND_JOHN: int = 4


def normalize_manufacturer_id(value: Any, default: int = MANUFACTURER_NINTENDO) -> int:
    """Coerce a manufacturer value into a known numeric id.

    Any non-int/unknown value falls back to ``default``.
    """

    try:
        numeric = int(value)
    except Exception:
        return int(default)

    if numeric not in (MANUFACTURER_NINTENDO, MANUFACTURER_TRONICA, MANUFACTURER_ELEKTRONIKA, MANUFACTURER_DAVID_AND_JOHN, MANUFACTURER_TIGER):
        return int(default)
    
    return numeric
