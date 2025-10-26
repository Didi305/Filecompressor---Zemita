
#include <cstdint>
#include <string>
#include <fstream>
#include <filesystem>
#include "buffered_writer.hpp"
#include "buffered_reader.hpp"
#include <vector>

#pragma pack(push, 1)    
struct GlobalHeader{
    char magicBytes[4] = {'Z', 'E', 'M', '1'};
    char original_extension[8];
    uint32_t original_size;
    uint32_t checksum_id = 0;
    uint32_t block_size = 2 * 1024;
    uint16_t version = 1;
    int codec_id = 1;
};
#pragma pack(pop)

struct BlockHeader{
    uint32_t block_seq_num = 0;
    uint32_t uncompressed_size;
    uint32_t compressed_size;
    bool operator<(const BlockHeader& other) const {
        return block_seq_num < other.block_seq_num;
    }
};

inline void write_u32_le(std::ostream& out, uint32_t value){
    unsigned char bytes[4];
    bytes[0] = value & 0xFF;
    bytes[1] = (value >> 8) & 0xFF;
    bytes[2] = (value >> 16) & 0xFF;
    bytes[3] = (value >> 24) & 0xFF;
    out.write(reinterpret_cast<char*>(bytes), 4);
}

class ContainerWriter {
public:
    explicit ContainerWriter(std::string& filePath, const GlobalHeader& gHeader);
    void writeBlock(BlockHeader& bheader, char* data);
    void finalize();
    ~ContainerWriter();
private:
    std::ofstream out_;
    BufferedWriter writer_;
};

class ContainerReader {
public:
    explicit ContainerReader(const std::string& input_path);
    std::map<BlockHeader, char*> readAllBlocks();
    ~ContainerReader();
private:
    std::ifstream in_;
    GlobalHeader gHeader_{};
    uint32_t numberOfBlocks;
    std::vector<BlockHeader> blocks;
    BufferedReader reader_;
};

