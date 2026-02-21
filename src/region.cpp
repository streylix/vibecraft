#include "vibecraft/region.h"

#include <zlib.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>

namespace vibecraft {

// --- Free functions ---

int ChunkToRegionCoord(int chunk_coord) {
    // Floor division for negative coordinates.
    if (chunk_coord >= 0) {
        return chunk_coord / kRegionSize;
    }
    return (chunk_coord - kRegionSize + 1) / kRegionSize;
}

int ChunkToRegionOffset(int chunk_coord) {
    int mod = chunk_coord % kRegionSize;
    if (mod < 0) {
        mod += kRegionSize;
    }
    return mod;
}

// --- RegionFile ---

RegionFile::RegionFile(const std::string& file_path)
    : file_path_(file_path), offset_table_(kRegionChunkCount) {}

bool RegionFile::SaveChunk(const Chunk& chunk) {
    int local_x = ChunkToRegionOffset(chunk.GetChunkX());
    int local_z = ChunkToRegionOffset(chunk.GetChunkZ());
    int slot = SlotIndex(local_x, local_z);

    // Serialize and compress.
    std::vector<uint8_t> raw = SerializeChunk(chunk);
    std::vector<uint8_t> compressed = Compress(raw);
    if (compressed.empty()) {
        return false;
    }

    // Read existing offset table if the file exists.
    ReadOffsetTable();

    // Open file for read+write, or create if it doesn't exist.
    std::fstream file(file_path_, std::ios::in | std::ios::out | std::ios::binary);
    if (!file.is_open()) {
        // File doesn't exist — create it.
        std::ofstream create(file_path_, std::ios::binary);
        if (!create.is_open()) {
            return false;
        }
        // Write empty header.
        std::vector<uint8_t> empty_header(kRegionHeaderSize, 0);
        create.write(reinterpret_cast<const char*>(empty_header.data()),
                     kRegionHeaderSize);
        create.close();

        // Reopen for read+write.
        file.open(file_path_, std::ios::in | std::ios::out | std::ios::binary);
        if (!file.is_open()) {
            return false;
        }
    }

    // Find the end of file to append data.
    file.seekp(0, std::ios::end);
    auto data_offset = static_cast<uint32_t>(file.tellp());

    // If the file is smaller than the header, extend it.
    if (data_offset < kRegionHeaderSize) {
        std::vector<uint8_t> padding(kRegionHeaderSize - data_offset, 0);
        file.write(reinterpret_cast<const char*>(padding.data()),
                   static_cast<std::streamsize>(padding.size()));
        data_offset = kRegionHeaderSize;
    }

    // Write compressed data at end.
    file.seekp(data_offset);
    file.write(reinterpret_cast<const char*>(compressed.data()),
               static_cast<std::streamsize>(compressed.size()));

    // Update offset table.
    offset_table_[slot].offset = data_offset;
    offset_table_[slot].length = static_cast<uint32_t>(compressed.size());

    // Write offset table back to header.
    if (!WriteOffsetTable(file)) {
        return false;
    }

    file.close();
    return true;
}

std::unique_ptr<Chunk> RegionFile::LoadChunk(int cx, int cz) {
    int local_x = ChunkToRegionOffset(cx);
    int local_z = ChunkToRegionOffset(cz);
    int slot = SlotIndex(local_x, local_z);

    // Read offset table.
    if (!ReadOffsetTable()) {
        return nullptr;
    }

    const auto& entry = offset_table_[slot];
    if (entry.offset == 0 || entry.length == 0) {
        return nullptr;  // No data for this chunk.
    }

    // Open file and read compressed data.
    std::ifstream file(file_path_, std::ios::binary);
    if (!file.is_open()) {
        return nullptr;
    }

    file.seekg(entry.offset);
    std::vector<uint8_t> compressed(entry.length);
    file.read(reinterpret_cast<char*>(compressed.data()),
              static_cast<std::streamsize>(entry.length));
    if (!file) {
        return nullptr;
    }
    file.close();

    // Decompress.
    constexpr std::size_t kRawChunkSize =
        static_cast<std::size_t>(kChunkVolume) * 3;
    std::vector<uint8_t> raw = Decompress(compressed, kRawChunkSize);
    if (raw.empty()) {
        return nullptr;
    }

    return DeserializeChunk(raw, cx, cz);
}

bool RegionFile::HasChunk(int cx, int cz) const {
    // Need non-const version for ReadOffsetTable, but we can check the file
    // directly if table not loaded.
    auto* self = const_cast<RegionFile*>(this);
    if (!self->ReadOffsetTable()) {
        return false;
    }

    int local_x = ChunkToRegionOffset(cx);
    int local_z = ChunkToRegionOffset(cz);
    int slot = SlotIndex(local_x, local_z);

    return offset_table_[slot].offset != 0 && offset_table_[slot].length != 0;
}

const std::string& RegionFile::GetFilePath() const {
    return file_path_;
}

std::size_t RegionFile::GetFileSize() const {
    std::ifstream file(file_path_, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return 0;
    }
    return static_cast<std::size_t>(file.tellg());
}

int RegionFile::SlotIndex(int local_x, int local_z) {
    return local_z * kRegionSize + local_x;
}

bool RegionFile::ReadOffsetTable() {
    if (table_loaded_) {
        return true;
    }

    std::ifstream file(file_path_, std::ios::binary);
    if (!file.is_open()) {
        // File doesn't exist — that's fine, table stays zeroed.
        offset_table_.assign(kRegionChunkCount, {0, 0});
        return false;
    }

    // Check file is large enough for the header.
    file.seekg(0, std::ios::end);
    auto file_size = file.tellg();
    if (file_size < kRegionHeaderSize) {
        offset_table_.assign(kRegionChunkCount, {0, 0});
        return false;
    }

    file.seekg(0);
    for (int i = 0; i < kRegionChunkCount; ++i) {
        uint32_t offset = 0;
        uint32_t length = 0;
        file.read(reinterpret_cast<char*>(&offset), sizeof(offset));
        file.read(reinterpret_cast<char*>(&length), sizeof(length));
        offset_table_[i].offset = offset;
        offset_table_[i].length = length;
    }

    table_loaded_ = true;
    return true;
}

bool RegionFile::WriteOffsetTable(std::fstream& file) {
    file.seekp(0);
    for (int i = 0; i < kRegionChunkCount; ++i) {
        file.write(reinterpret_cast<const char*>(&offset_table_[i].offset),
                   sizeof(uint32_t));
        file.write(reinterpret_cast<const char*>(&offset_table_[i].length),
                   sizeof(uint32_t));
    }
    return file.good();
}

std::vector<uint8_t> RegionFile::Compress(const std::vector<uint8_t>& raw) {
    if (raw.empty()) {
        return {};
    }

    // Worst-case compressed size estimate.
    uLongf bound = compressBound(static_cast<uLong>(raw.size()));
    std::vector<uint8_t> compressed(bound);

    uLongf compressed_size = bound;
    int ret = compress2(compressed.data(), &compressed_size,
                        raw.data(), static_cast<uLong>(raw.size()),
                        Z_DEFAULT_COMPRESSION);
    if (ret != Z_OK) {
        return {};
    }

    compressed.resize(compressed_size);
    return compressed;
}

std::vector<uint8_t> RegionFile::Decompress(const std::vector<uint8_t>& compressed,
                                             std::size_t expected_size) {
    if (compressed.empty()) {
        return {};
    }

    std::vector<uint8_t> raw(expected_size);
    uLongf raw_size = static_cast<uLongf>(expected_size);
    int ret = uncompress(raw.data(), &raw_size,
                         compressed.data(),
                         static_cast<uLong>(compressed.size()));
    if (ret != Z_OK || raw_size != expected_size) {
        return {};
    }

    return raw;
}

std::vector<uint8_t> RegionFile::SerializeChunk(const Chunk& chunk) {
    constexpr std::size_t kRawChunkSize =
        static_cast<std::size_t>(kChunkVolume) * 3;
    std::vector<uint8_t> raw(kRawChunkSize);

    std::size_t pos = 0;

    // Block IDs.
    for (int y = 0; y < kChunkSizeY; ++y) {
        for (int z = 0; z < kChunkSizeZ; ++z) {
            for (int x = 0; x < kChunkSizeX; ++x) {
                raw[pos++] = chunk.GetBlock(x, y, z);
            }
        }
    }

    // Light data.
    for (int y = 0; y < kChunkSizeY; ++y) {
        for (int z = 0; z < kChunkSizeZ; ++z) {
            for (int x = 0; x < kChunkSizeX; ++x) {
                raw[pos++] = chunk.GetRawLight(x, y, z);
            }
        }
    }

    // Fluid levels.
    for (int y = 0; y < kChunkSizeY; ++y) {
        for (int z = 0; z < kChunkSizeZ; ++z) {
            for (int x = 0; x < kChunkSizeX; ++x) {
                raw[pos++] = chunk.GetFluidLevel(x, y, z);
            }
        }
    }

    return raw;
}

std::unique_ptr<Chunk> RegionFile::DeserializeChunk(const std::vector<uint8_t>& raw,
                                                     int cx, int cz) {
    constexpr std::size_t kRawChunkSize =
        static_cast<std::size_t>(kChunkVolume) * 3;
    if (raw.size() != kRawChunkSize) {
        return nullptr;
    }

    auto chunk = std::make_unique<Chunk>(cx, cz);
    std::size_t pos = 0;

    // Block IDs.
    for (int y = 0; y < kChunkSizeY; ++y) {
        for (int z = 0; z < kChunkSizeZ; ++z) {
            for (int x = 0; x < kChunkSizeX; ++x) {
                chunk->SetBlock(x, y, z, raw[pos++]);
            }
        }
    }

    // Light data.
    for (int y = 0; y < kChunkSizeY; ++y) {
        for (int z = 0; z < kChunkSizeZ; ++z) {
            for (int x = 0; x < kChunkSizeX; ++x) {
                chunk->SetRawLight(x, y, z, raw[pos++]);
            }
        }
    }

    // Fluid levels.
    for (int y = 0; y < kChunkSizeY; ++y) {
        for (int z = 0; z < kChunkSizeZ; ++z) {
            for (int x = 0; x < kChunkSizeX; ++x) {
                chunk->SetFluidLevel(x, y, z, raw[pos++]);
            }
        }
    }

    // Clear dirty flag since we just loaded from disk.
    chunk->ClearDirty();

    return chunk;
}

}  // namespace vibecraft
