

#include <cstdint>
#include <string>
#include <fstream>
#include <filesystem>
#pragma pack(push, 1)    
struct GlobalHeader{
    char magicBytes[4] = {'Z', 'E', 'M', '1'};
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
    char* data;
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
    explicit ContainerWriter(const std::string& output_path);
    void writeGlobalHeader(GlobalHeader& gheader);
    void writeBlock(BlockHeader& bheader);
    ~ContainerWriter();
private:
    std::ofstream out_;
};

class ContainerReader {
public:
    explicit ContainerReader(const std::string& input_path);
    GlobalHeader readGlobalHeader();
    BlockHeader* readAllBlocks(std::vector<BlockHeader>& blocks, uint32_t numberOfBlocks);
    ~ContainerReader();
private:
    std::ifstream in_;
};

