#include <cstdint>
#include <string>
#include <fstream>
#include <filesystem>

struct GlobalHeader{
    const char magicBytes[4] = {'Z', 'E', 'M', '1'};
    uint64_t original_size;
    uint32_t checksum_id = 0;
    uint16_t block_size = 4 * 1024 * 1024;
    uint16_t version = 1;
    int codec_id = 0;
};

struct BlockHeader{
    uint32_t uncompressed_size;
    uint32_t compressed_size;
    uint32_t block_seq_num = 0;
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
    void writeBlock(BlockHeader& bheader, std::vector<char>& data);
    ~ContainerWriter();
private:
    std::ofstream out_;
};

class ContainerReader {
public:
    explicit ContainerReader(const std::string& input_path);
private:
    std::ifstream in_;
};

