"""Generate an original dummy ROM/PNG pack; contains no commercial game data."""
import struct
import sys
from pathlib import Path
import zlib

def png():
    def chunk(kind, data):
        return struct.pack('>I', len(data)) + kind + data + struct.pack('>I', zlib.crc32(kind + data))
    return b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', 8, 8, 8, 6, 0, 0, 0)) + chunk(b'IDAT', zlib.compress(b''.join(b'\x00' + b''.join(bytes((x*32, y*32, 50, 255)) for x in range(8)) for y in range(8)))) + chunk(b'IEND', b'')

mode = sys.argv[2] if len(sys.argv) > 2 else 'single'
games = [('Synthetic test', 'AC_01', 0)]
if mode == 'clock':
    games = [('Clock startup', 'JR_55', 0)]
if mode.startswith('multi'):
    games = [('Nintendo one', 'AC_01', 0), ('Tronica one', 'TM_01', 1),
             ('Elektronika one', 'EL_01', 2), ('Nintendo two', 'JR_55', 0),
             ('Tronica two', 'TM_02', 1), ('Elektronika two', 'EL_02', 2)]
    if mode == 'multi-reordered':
        games.reverse()
single = mode in ('single', 'clock') or mode.startswith('multi')
file_offset = 36 + 100 * len(games)
out = bytearray(file_offset + 16)
def blob(data):
    start = len(out)
    out.extend(data)
    return start, len(data)
def string(value):
    return blob(value.encode())
date = string('2026')
rom_data = bytearray(4096 if mode == 'clock' else 1856)
if mode == 'clock':
    # Clear a clock digit, then latch it elsewhere before going to sleep.
    program = (0x5f, 0x43, 0x20, 0x10, 0x18, 0x5f, 0x4c, 0x10, 0x5d)
    word = 0
    for opcode in program:
        rom_data[3 * 16 * 64 + 7 * 64 + word] = opcode
        word = (word >> 1) | (int((word & 1) == ((word >> 1) & 1)) << 5)
rom, melody = blob(rom_data), blob(b'')
path = string('gfx/test.png')
file_name = string('test.png')
segment_data = struct.pack('<BBBiiHHHHBB', 0, 0, 0, 0, 0, 0, 0, 8, 8, 0, 0)
if not single:
    segment_data += struct.pack('<BBBiiHHHHBB', 0, 0, 0, 0, 0, 0, 0, 8, 8, 1, 1)
segments_off, _ = blob(segment_data)
info_off, _ = blob(struct.pack('<8H', 8, 8, 1, 2 if mode == 'horizontal' else 1 if mode == 'mask' else 0, 80, 60, 80, 60))
empty = blob(b'')
bg_info, _ = blob(struct.pack('<13H', 8, 8, 0, 0, 8, 8, 0, 0, 8, 8, 0, 1 if mode == 'mask' else 0, 0))
# Single-screen backgrounds use a shorter metadata layout.
if single:
    bg_info, _ = blob(struct.pack('<9H', 8, 8, 0, 0, 8, 8, 0, 0, 0))
console_info, _ = blob(struct.pack('<6H', 8, 8, 0, 0, 8, 8))
for index, (name, ref, manufacturer) in enumerate(games):
    entry = (*string(name), *string(ref), *date, *rom, *melody, *path, segments_off,
             len(segment_data)//21, info_off, 8, *path, bg_info, 9 if single else 13,
             *path, console_info, 6, manufacturer)
    struct.pack_into('<25I', out, 36 + index * 100, *entry)
image = blob(png())
struct.pack_into('<4I', out, file_offset, *file_name, *image)
struct.pack_into('<9I', out, 0, 0x31504B59, 3, 2, 3, len(games), 1, 36, file_offset, file_offset + 16)
Path(sys.argv[1]).write_bytes(out)
