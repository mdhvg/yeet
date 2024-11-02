#include "Yeet.h"

/*
Packet format for file transfer
[10 bytes] = File ID
[4 bytes] = Chunk index (Little endian)
[4 bytes] = Data size (Little endian)
[max -> (1024 * 1024) - (10 + 4 + 4) bytes, min -> 0 bytes] = Data
*/

class FileReader {
public:
    FileReader(const std::string& fileName);
    ~FileReader();

    rtc::binary readNextPacket();
    size_t size();
    std::string name();
    int chunks();
    bool hasNext();
    std::string getID();
private:
    std::mutex indexMutex;
    fs::path base = basePath / "send";
    fs::path filePath;
    std::ifstream fileHandle;
    size_t fileSize;
    std::string fileID;
    int iterator = 0;
    int numChunks = 0;
};