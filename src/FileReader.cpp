#include "FileReader.h"

struct FilePacket {
    int index;
    int packetSize;
    rtc::binary data;
};

FileReader::FileReader(const std::string& fileName) {
    filePath = base / fileName;
    fileID = nanoid::generate(10);
    std::cout << filePath << std::endl;
    assert(fs::exists(filePath) && fs::is_regular_file(filePath) && "File does not exist or is empty");
    fileHandle.open(filePath, std::ios::binary);
    fileSize = fs::file_size(filePath);
    numChunks = (fileSize + READ_SIZE - 1) / READ_SIZE;
}

FileReader::~FileReader() { fileHandle.close(); }

rtc::binary FileReader::readNextPacket() {
    indexMutex.lock();
    unsigned char* data;
    int bytesRead;
    if (fileSize - fileHandle.tellg() >= READ_SIZE) {
        bytesRead = READ_SIZE;
    }
    else {
        bytesRead = fileSize - fileHandle.tellg();
    }
    data = new unsigned char[HEADER_SIZE + bytesRead];
    assert(fileID.size() == 10 && "File ID must be 10 bytes");
    memcpy(data, fileID.data(), 10);
    memcpy(data + 10, &iterator, 4);
    memcpy(data + 14, &bytesRead, 4);
    fileHandle.read(reinterpret_cast<char*>(HEADER_SIZE + data), bytesRead);
    rtc::binary bytes(HEADER_SIZE + bytesRead);
    bytes.assign(reinterpret_cast<std::byte*>(data), reinterpret_cast<std::byte*>(data) + HEADER_SIZE + bytesRead);
    iterator++;
    delete[] data;
    indexMutex.unlock();
    return bytes;
}

size_t FileReader::size() { return fileSize; }

std::string FileReader::name() { return filePath.filename().string(); }

int FileReader::chunks() { return numChunks; }

bool FileReader::hasNext() { return fileHandle.tellg() < static_cast<std::streamoff>(fileSize); }

std::string FileReader::getID() { return fileID; }