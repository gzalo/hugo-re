import os
import struct

import pygame

# Cache decoded surfaces per file path to avoid re-reading the same file
_surface_cache = {}
_oos_cache = {}


def _get_int(data, offset):
    return struct.unpack_from('<i', data, offset)[0]


def _get_short(data, offset):
    return struct.unpack_from('<h', data, offset)[0]


def _palette_from_data(data, offset):
    """Read 256 RGB colors (3 bytes each) → list of (r, g, b, a) tuples."""
    colors = []
    for _ in range(256):
        colors.append((data[offset], data[offset + 1], data[offset + 2], 255))
        offset += 3
    return colors


def _palette_from_alpha_data(data, offset):
    """Read 256 colors from 4-byte little-endian values (alpha forced to 255)."""
    # Java: getInt(data, offset) | 0xFF000000 stored as ARGB packed int
    # ARGB: A=FF, R=data[offset+2], G=data[offset+1], B=data[offset]
    colors = []
    for _ in range(256):
        colors.append((data[offset + 2], data[offset + 1], data[offset], 255))
        offset += 4
    return colors


def _make_surface(pixels, width, height):
    surf = pygame.image.frombuffer(bytes(pixels), (width, height), 'RGBA')
    return surf.convert_alpha()


def _cgf_parse_line(pixels, data, idx, length, x, y, palette, width):
    """Parse one CGF scanline. Modifies pixels bytearray in place.

    The Java implementation uses a for(i=0; i<len; i++) loop with an extra i++
    right after reading the opcode. This while loop replicates that behaviour:
    the final `i += 1` at the bottom mirrors the for-loop's own increment.
    """
    i = 0
    while i < length:
        v = data[idx + i]
        i += 1  # skip past opcode (inner i++ from Java)

        if v == 0:
            off = data[idx + i]
            if off == 0:
                return
            x += off
        elif v == 1:  # UNUSED in original game
            count = data[idx + i]
            for _ in range(count):
                i += 1
                pal_idx = data[idx + i]
                pos = x + width * y
                pixels[pos * 4:pos * 4 + 4] = palette[pal_idx]
                x += 1
                i += 1  # extra i++ present in case v=1
        elif v == 2:  # UNUSED in original game
            count = data[idx + i]
            i += 1
            value = data[idx + i]
            i += 1
            # value_alpha = data[idx + i]  # read to advance i, not used
            for _ in range(count):
                pos = x + width * y
                pixels[pos * 4:pos * 4 + 4] = palette[value]
                x += 1
        elif v == 3:
            count = data[idx + i]
            for _ in range(count):
                i += 1
                pal_idx = data[idx + i]
                pos = x + width * y
                pixels[pos * 4:pos * 4 + 4] = palette[pal_idx]
                x += 1
        elif v == 4:
            count = data[idx + i]
            i += 1
            value = data[idx + i]
            for _ in range(count):
                pos = x + width * y
                pixels[pos * 4:pos * 4 + 4] = palette[value]
                x += 1
        else:
            print(f"Unknown byte in CGF: {v} @ {idx + i}")

        i += 1  # outer for-loop increment


def _decode_cgf(file_content, file_path):
    """Decode a CGF sprite file → list of pygame.Surface (one per frame)."""
    parent_dir = os.path.dirname(file_path)
    palette = None

    # Try to find a sibling .til file to use as the palette source
    try:
        for fname in os.listdir(parent_dir):
            if fname.lower().endswith('.til'):
                with open(os.path.join(parent_dir, fname), 'rb') as f:
                    til_data = f.read()
                palette = _palette_from_data(til_data, 0x20)
                break
    except OSError:
        pass

    if palette is None:
        if len(file_content) < 0x400:
            print(f"Cannot find feasible palette for {file_path}")
            return []
        palette = _palette_from_alpha_data(file_content, len(file_content) - 0x400)

    total_frames = _get_int(file_content, 8)
    surfaces = []

    for frame_idx in range(total_frames):
        meta_offset = 7 * 4 + frame_idx * 6 * 4
        width = _get_int(file_content, meta_offset + 8)
        height = _get_int(file_content, meta_offset + 12)
        payload_offset = _get_int(file_content, meta_offset + 20)

        if width < 0 or height < 0:
            print(f"Invalid frame size: {width}x{height}")
            return []

        if width == 0 or height == 0:
            surfaces.append(pygame.Surface((1, 1), pygame.SRCALPHA))
            continue

        pixels = bytearray(width * height * 4)  # RGBA, zero = fully transparent
        start_offset = 7 * 4 + total_frames * 6 * 4 + payload_offset

        for y in range(height):
            line_len = _get_int(file_content, start_offset)
            _cgf_parse_line(pixels, file_content, start_offset + 4, line_len - 4, 0, y, palette, width)
            start_offset += line_len

        surfaces.append(_make_surface(pixels, width, height))

    return surfaces


def _decode_til(file_content, file_path):
    """Decode a TIL tileset file → list of pygame.Surface (one per frame)."""
    palette = _palette_from_data(file_content, 0x20)

    total_width = _get_short(file_content, 8)
    total_height = _get_short(file_content, 10)
    total_width_tiles = file_content[0x15]
    total_height_tiles = file_content[0x17]
    tile_width = total_width // total_width_tiles
    tile_height = total_height // total_height_tiles
    total_frames = _get_short(file_content, 6) + 2
    delta = total_width_tiles * total_height_tiles * 2
    tile_data_offset = total_frames * delta + 0x320

    surfaces = []
    for frame_idx in range(total_frames):
        frame_offset = 0x320 + frame_idx * delta
        pixels = bytearray(total_width * total_height * 4)
        valid = True

        for l in range(total_width_tiles * total_height_tiles):
            tile_id = _get_short(file_content, frame_offset)
            frame_offset += 2
            x_off = (l % total_width_tiles) * tile_width
            y_off = (l // total_width_tiles) * tile_height
            curr_offset = tile_data_offset + tile_id * tile_height * tile_width

            for y in range(tile_height):
                for x in range(tile_width):
                    if curr_offset >= len(file_content):
                        valid = False
                        break
                    color_idx = file_content[curr_offset]
                    curr_offset += 1
                    pos = (x + x_off) + (y + y_off) * total_width
                    pixels[pos * 4:pos * 4 + 4] = palette[color_idx]
                if not valid:
                    break
            if not valid:
                break

        if not valid:
            break

        surfaces.append(_make_surface(pixels, total_width, total_height))

    return surfaces


def _decode_lzp(file_content, file_path):
    """Decode an LZP compressed image file → list of pygame.Surface (one per frame)."""
    if len(file_content) == 0:
        return []

    palette = _palette_from_data(file_content, 0x20)
    total_frames = _get_int(file_content, 0)
    width = _get_int(file_content, 4)
    height = _get_int(file_content, 8)
    N = 4095

    surfaces = []
    for frame_idx in range(total_frames):
        offset_to_offsets = len(file_content) - total_frames * 4 + frame_idx * 4
        offset_local = _get_int(file_content, offset_to_offsets)
        compressed_len = _get_int(file_content, offset_local)

        pixels = bytearray(width * height * 4)  # RGBA, zero = fully transparent
        data_ptr = offset_local + 4
        data_end = data_ptr + compressed_len

        window = [0] * 4096
        dst_pos = 0
        window_index = 4078
        bit_idx = 0
        end = False
        flags = 0

        while not end:
            if bit_idx == 0:
                if data_ptr >= data_end:
                    break
                flags = file_content[data_ptr]
                data_ptr += 1

            if (flags & 1) != 0:
                if data_ptr >= data_end:
                    break
                c = file_content[data_ptr]
                data_ptr += 1
                if dst_pos > width * height:
                    break
                pos = dst_pos % width + (dst_pos // width) * width
                pixels[pos * 4:pos * 4 + 4] = palette[c]
                dst_pos += 1
                window[window_index] = c
                window_index = (window_index + 1) & N
            else:
                if data_ptr >= data_end:
                    break
                offset = file_content[data_ptr]
                data_ptr += 1
                if data_ptr >= data_end:
                    break
                length_byte = file_content[data_ptr]
                data_ptr += 1
                offset |= (length_byte & 0xF0) << 4
                run_len = (length_byte & 0xF) + 3
                for k in range(run_len):
                    c = window[(offset + k) & N]
                    if dst_pos > width * height:
                        end = True
                        break
                    pos = dst_pos % width + (dst_pos // width) * width
                    pixels[pos * 4:pos * 4 + 4] = palette[c]
                    dst_pos += 1
                    window[window_index] = c
                    window_index = (window_index + 1) & N

            flags >>= 1
            bit_idx = (bit_idx + 1) & 7

        surfaces.append(_make_surface(pixels, width, height))

    return surfaces


def decode_oos(file_path):
    """Decode an OOS animation sync file → list of frame indices."""
    if file_path in _oos_cache:
        return _oos_cache[file_path]

    with open(file_path, 'rb') as f:
        data = f.read()

    offset_to_length = _get_int(data, 0x14)
    offset_to_data = _get_int(data, 0x18)
    length = _get_int(data, offset_to_length)
    result = [data[offset_to_data + i] for i in range(length)]

    _oos_cache[file_path] = result
    return result


def decode_surfaces(file_path):
    """Decode a CGF, TIL, or LZP file → list of pygame.Surface.

    Results are cached by file path so repeated calls for the same file
    (e.g. loading different frame ranges from CASELIVE.TIL) are cheap.
    """
    if file_path in _surface_cache:
        return _surface_cache[file_path]

    ext = os.path.splitext(file_path)[1].upper()

    with open(file_path, 'rb') as f:
        file_content = f.read()

    if ext == '.CGF':
        surfaces = _decode_cgf(file_content, file_path)
    elif ext == '.TIL':
        surfaces = _decode_til(file_content, file_path)
    elif ext == '.LZP':
        surfaces = _decode_lzp(file_content, file_path)
    else:
        print(f"Unsupported format for decode_surfaces: {ext}")
        surfaces = []

    _surface_cache[file_path] = surfaces
    return surfaces
