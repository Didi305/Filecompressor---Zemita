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

void write_u32_le(std::ostream& out, uint32_t value){
    
}

void write_header(GlobalHeader global, std::string path){
    global.original_size = std::filesystem::file_size(path);
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
}
