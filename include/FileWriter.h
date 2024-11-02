#include "Yeet.h"

/*
Packet format for file transfer
[10 bytes] = File ID
[4 bytes] = Chunk index (Little endian)
[4 bytes] = Data size (Little endian)
[max -> (1024 * 1024) - (10 + 4 + 4) bytes, min -> 0 bytes] = Data
*/

class FileWriter {
public:
    FileWriter();
    ~FileWriter();

    void open(const std::string path, const std::string fileID, int chunks, uint64_t fileSize);
    void write(rtc::binary data);
    bool isComplete();
private:
    fs::path base = basePath / "recv";
    fs::path path;
    std::ofstream fileHandle;
    int numChunks;
    std::string fileID;
    std::mutex indexMutex;
    int iterator = 0;
    uint64_t fileSize;
    std::unordered_map<int, rtc::binary> chunks;
};