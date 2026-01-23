#include <cstdint>
#include <fstream>
#include <map>
#include <string>
#include <vector>
#include <zemita/utils.hpp>

#include "io/buffered_reader.hpp"
#include "io/buffered_writer.hpp"

const int MAGIC_BYTES_SIZE = 4;
const int EXTENSION_BYTES_SIZE = 5;

#pragma pack(push, 1)

struct GlobalHeader
{
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    char magicBytes[MAGIC_BYTES_SIZE] = {'Z', 'E', 'M', '1'};
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    char original_extension[EXTENSION_BYTES_SIZE];
    uint32_t original_size;
    uint32_t block_size = BLOCK_SIZE;  // 64KB
    uint8_t version = 1;
    uint8_t codec_id = 1;
};
#pragma pack(pop)

struct BlockHeader
{
    uint16_t block_seq_num = 0;
    uint32_t uncompressed_size;
    uint32_t compressed_size;
    auto operator<(const BlockHeader& other) const { return block_seq_num < other.block_seq_num; }
};

/*inline void write_u32_le(std::ostream& out, uint32_t value)
{
    unsigned char bytes[4];
    bytes[0] = value & 0xFF;
    bytes[1] = (value >> 8) & 0xFF;
    bytes[2] = (value >> 16) & 0xFF;
    bytes[3] = (value >> 24) & 0xFF;
    out.write(reinterpret_cast<char*>(bytes), 4);
}*/

class ContainerWriter
{
   public:
    explicit ContainerWriter(std::string& filePath, const GlobalHeader& gHeader);
    void writeBlock(BlockHeader& bheader, const std::vector<Match>& matches);
    void write(const char* toBeWrittenData, size_t datasize);
    void finalize();
    ~ContainerWriter();
    auto getWriter() { return &writer_; };

   private:
    std::ofstream out_;
    BufferedWriter writer_;
};

class ContainerReader
{
   public:
    explicit ContainerReader(const std::string& input_path);
    GlobalHeader readGlobalHeader(const std::string& path);
    std::map<BlockHeader, std::vector<Match>> readAllBlocks();
    ~ContainerReader();

   private:
    std::ifstream in_;
    GlobalHeader gHeader_{};
    uint32_t numberOfBlocks;
    std::vector<BlockHeader> blocks;
    BufferedReader reader_;
};
