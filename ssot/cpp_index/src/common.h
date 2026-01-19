#pragma once

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

inline void write_varint(std::ofstream& out, uint64_t value) {
    while (value >= 0x80) {
        uint8_t byte = static_cast<uint8_t>(value) | 0x80;
        out.write(reinterpret_cast<const char*>(&byte), 1);
        value >>= 7;
    }
    uint8_t byte = static_cast<uint8_t>(value);
    out.write(reinterpret_cast<const char*>(&byte), 1);
}

inline bool read_varint(std::ifstream& in, uint64_t& value) {
    value = 0;
    uint64_t shift = 0;
    while (true) {
        uint8_t byte = 0;
        if (!in.read(reinterpret_cast<char*>(&byte), 1)) {
            return false;
        }
        value |= static_cast<uint64_t>(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0) {
            return true;
        }
        shift += 7;
        if (shift > 63) {
            return false;
        }
    }
}

inline bool read_varint_with_len(std::ifstream& in, uint64_t& value, uint32_t& bytes_read) {
    value = 0;
    bytes_read = 0;
    uint64_t shift = 0;
    while (true) {
        uint8_t byte = 0;
        if (!in.read(reinterpret_cast<char*>(&byte), 1)) {
            return false;
        }
        ++bytes_read;
        value |= static_cast<uint64_t>(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0) {
            return true;
        }
        shift += 7;
        if (shift > 63) {
            return false;
        }
    }
}

inline bool read_string_at(std::ifstream& in, uint64_t offset, std::string& out, uint64_t* end_offset = nullptr) {
    in.clear();
    in.seekg(static_cast<std::streamoff>(offset), std::ios::beg);
    if (!in.good()) {
        return false;
    }
    uint64_t len = 0;
    uint32_t len_bytes = 0;
    if (!read_varint_with_len(in, len, len_bytes)) {
        return false;
    }
    out.resize(static_cast<size_t>(len));
    if (!in.read(&out[0], static_cast<std::streamsize>(len))) {
        return false;
    }
    if (end_offset) {
        *end_offset = offset + len_bytes + len;
    }
    return true;
}
