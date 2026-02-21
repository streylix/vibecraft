#ifndef VIBECRAFT_REGION_H
#define VIBECRAFT_REGION_H

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

#include "vibecraft/chunk.h"

namespace vibecraft {

/// Number of chunks along one axis of a region.
constexpr int kRegionSize = 32;

/// Total number of chunk slots in a region (32 x 32).
constexpr int kRegionChunkCount = kRegionSize * kRegionSize;

/// Size of the region file header in bytes.
/// The header contains an offset table: for each of the 1024 chunk slots,
/// an 8-byte entry (4 bytes offset, 4 bytes length) = 8192 bytes total.
constexpr int kRegionHeaderSize = kRegionChunkCount * 8;

/// Convert a chunk coordinate to a region coordinate.
/// Uses floor division: rx = floor(cx / 32).
int ChunkToRegionCoord(int chunk_coord);

/// Convert a chunk coordinate to a local offset within the region [0, 31].
int ChunkToRegionOffset(int chunk_coord);

/// Handles reading and writing chunk data to a single region file.
///
/// Region file format:
///   - Header: 8192 bytes (1024 entries of 8 bytes each)
///     - Each entry: uint32_t offset (from start of file), uint32_t compressed_size
///     - offset == 0 means the slot is empty (no chunk stored)
///   - Data: compressed chunk blobs appended after header
///
/// Each chunk blob is compressed with zlib deflate. The raw chunk data
/// layout within the blob is:
///   - 65536 bytes: block IDs (uint8_t[kChunkVolume])
///   - 65536 bytes: light data (uint8_t[kChunkVolume])
///   - 65536 bytes: fluid levels (uint8_t[kChunkVolume])
///   Total raw: 196608 bytes per chunk.
class RegionFile {
public:
    /// Construct a RegionFile for the given file path.
    /// Does not open or create the file until Save/Load is called.
    explicit RegionFile(const std::string& file_path);

    /// Save a chunk to this region file.
    /// The chunk's position within the region is determined by its chunk coords.
    /// Data is compressed with zlib before writing.
    /// Returns true on success.
    bool SaveChunk(const Chunk& chunk);

    /// Load a chunk from this region file.
    /// Returns nullptr if no data is stored for the given local offset,
    /// or if the file does not exist or is corrupt.
    /// The returned chunk has its chunk coordinates set to (cx, cz).
    std::unique_ptr<Chunk> LoadChunk(int cx, int cz);

    /// Check if a chunk exists in this region file.
    bool HasChunk(int cx, int cz) const;

    /// Get the file path of this region file.
    const std::string& GetFilePath() const;

    /// Get the file size in bytes, or 0 if the file does not exist.
    std::size_t GetFileSize() const;

private:
    /// Offset table entry.
    struct OffsetEntry {
        uint32_t offset = 0;   ///< Byte offset from start of file (0 = empty).
        uint32_t length = 0;   ///< Compressed data length in bytes.
    };

    /// Compute the slot index for a chunk coordinate pair within this region.
    static int SlotIndex(int local_x, int local_z);

    /// Read the offset table from the file.
    /// Returns false if the file doesn't exist or is too small.
    bool ReadOffsetTable();

    /// Write the offset table to the file.
    bool WriteOffsetTable(std::fstream& file);

    /// Compress raw data using zlib deflate.
    /// Returns the compressed data, or an empty vector on failure.
    static std::vector<uint8_t> Compress(const std::vector<uint8_t>& raw);

    /// Decompress zlib data.
    /// expected_size is the expected uncompressed size.
    /// Returns the decompressed data, or an empty vector on failure.
    static std::vector<uint8_t> Decompress(const std::vector<uint8_t>& compressed,
                                            std::size_t expected_size);

    /// Serialize a chunk to raw bytes (blocks + light + fluid).
    static std::vector<uint8_t> SerializeChunk(const Chunk& chunk);

    /// Deserialize raw bytes into a new chunk at the given coordinates.
    static std::unique_ptr<Chunk> DeserializeChunk(const std::vector<uint8_t>& raw,
                                                    int cx, int cz);

    std::string file_path_;
    std::vector<OffsetEntry> offset_table_;
    bool table_loaded_ = false;
};

}  // namespace vibecraft

#endif  // VIBECRAFT_REGION_H
