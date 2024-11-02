#include "FileWriter.h"

FileWriter::FileWriter() {}

FileWriter::~FileWriter() { fileHandle.close(); }

void FileWriter::open(const std::string path, const std::string fileID, int chunks, uint64_t fileSize)
{
    this->fileID = fileID;
    this->fileSize = fileSize;
    numChunks = chunks;
    fs::path filePath = base / path;
    fileHandle.open(filePath, std::ios::binary);
}

void FileWriter::write(rtc::binary data) {
    indexMutex.lock();
    int index, size;
    memcpy(&index, data.data() + 10, 4);
    memcpy(&size, data.data() + 14, 4);
    std::cout << "Index: " << index << " Size: " << size << std::endl;
    if (index == iterator) {
        fileHandle.write(reinterpret_cast<const char*>(data.data() + 18), size);
        iterator++;
    }
    else {
        chunks[index] = rtc::binary(data.data() + 18, data.data() + 18 + size);
    }
    if (chunks.find(iterator) != chunks.end()) {
        fileHandle.write(reinterpret_cast<const char*>(chunks[iterator].data()), chunks[iterator].size());
        chunks.erase(iterator);
        iterator++;
    }
    // if (chunks.size() == numChunks) {
    //     fileHandle.seekp(0, std::ios::beg);
    //     for (int i = 0; i < numChunks; i++) {
    //         fileHandle.write(reinterpret_cast<const char*>(chunks[i].data()), chunks[i].size());
    //     }
    // }
    indexMutex.unlock();
}

bool FileWriter::isComplete() { return fileHandle.tellp() == fileSize; }