"""Generate an original dummy ROM/PNG pack; contains no commercial game data."""
import struct
import sys
from pathlib import Path
import zlib

def png():
    def chunk(kind, data):
        return struct.pack('>I', len(data)) + kind + data + struct.pack('>I', zlib.crc32(kind + data))
    return b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', 8, 8, 8, 6, 0, 0, 0)) + chunk(b'IDAT', zlib.compress(b''.join(b'\x00' + b''.join(bytes((x*32, y*32, 50, 255)) for x in range(8)) for y in range(8)))) + chunk(b'IEND', b'')

out = bytearray(36 + 100 + 16)
def blob(data):
    start = len(out)
    out.extend(data)
    return start, len(data)
def string(value):
    return blob(value.encode())
name, ref, date = string('Synthetic test'), string('AC_01'), string('2026')
rom, melody = blob(bytes(1856)), blob(b'')
path = string('gfx/test.png')
file_name = string('test.png')
segment_data = struct.pack('<BBBiiHHHHBB', 0, 0, 0, 0, 0, 0, 0, 8, 8, 0, 0)
mode = sys.argv[2] if len(sys.argv) > 2 else 'single'
if mode != 'single':
    segment_data += struct.pack('<BBBiiHHHHBB', 0, 0, 0, 0, 0, 0, 0, 8, 8, 1, 1)
segments_off, _ = blob(segment_data)
info_off, _ = blob(struct.pack('<8H', 8, 8, 1, 2 if mode == 'horizontal' else 1 if mode == 'mask' else 0, 80, 60, 80, 60))
empty = blob(b'')
bg_info, _ = blob(struct.pack('<13H', 8, 8, 0, 0, 8, 8, 0, 0, 8, 8, 0, 1 if mode == 'mask' else 0, 0))
# Single-screen backgrounds use a shorter metadata layout.
if mode == 'single':
    bg_info, _ = blob(struct.pack('<9H', 8, 8, 0, 0, 8, 8, 0, 0, 0))
console_info, _ = blob(struct.pack('<6H', 8, 8, 0, 0, 8, 8))
entry = (*name, *ref, *date, *rom, *melody, *path, segments_off, len(segment_data)//21, info_off, 8, *path, bg_info, 9 if mode == "single" else 13, *path, console_info, 6, 0)
struct.pack_into('<25I', out, 36, *entry)
image = blob(png())
struct.pack_into('<4I', out, 136, *file_name, *image)
struct.pack_into('<9I', out, 0, 0x31504B59, 3, 2, 3, 1, 1, 36, 136, 152)
Path(sys.argv[1]).write_bytes(out)
