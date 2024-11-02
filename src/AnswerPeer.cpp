#include "Peer.h"
#include "fmt/core.h"

AnswerPeer::AnswerPeer(
    rtc::Configuration config,
    std::string selfID,
    std::string remoteID,
    std::string fileName,
    int threadCount) :
    Peer(config, selfID, remoteID),
    fileReader(fileName), threadCount(threadCount) {
    pc->onLocalDescription([&](rtc::Description sdp) {
        std::cout << "Local Description created" << std::endl;

        r = cpr::Patch(cpr::Url{ baseUrl + selfUserID + ".json" },
            cpr::Body{ json{{"sdp", std::string(sdp)}}.dump() },
            cpr::Header{ {"Content-Type", "application/json"} });
        });

    pc->onLocalCandidate([&](rtc::Candidate cand) {
        candidates.push_back(cand);

        r = cpr::Patch(cpr::Url{ baseUrl + selfUserID + ".json" },
            cpr::Body{ json{{"candidates", candidates}}.dump() },
            cpr::Header{ {"Content-Type", "application/json"} });
        });

    pc->onGatheringStateChange([&](rtc::PeerConnection::GatheringState state) {
        std::cout << "[Gathering State: " << state << "]" << std::endl;
        if (state == rtc::PeerConnection::GatheringState::Complete) {
            r = cpr::Patch(cpr::Url{ baseUrl + remoteUserID + ".json" },
                cpr::Body{ json{{"peer", selfUserID}}.dump() },
                cpr::Header{ {"Content-Type", "application/json"} });
        }
        });
}

void AnswerPeer::waitForDataChannel() {
    pc->onDataChannel([&](std::shared_ptr<rtc::DataChannel> datachannel) {
        std::cout << "DataChannel created: " << datachannel->label() << std::endl;
        communicationChannel = datachannel;
        communicationLock.unlock();
        });

    r = cpr::Get(cpr::Url{ baseUrl + remoteUserID + ".json" });

    assert(r.status_code == 200 && fmt::format("Request failed with status code: %d", r.status_code).c_str());

    json j = json::parse(r.text);
    rtc::Description offerDesc(j["sdp"].get<std::string>());

    pc->setRemoteDescription(offerDesc);

    for (auto& cand : j["candidates"].get<std::vector<std::string>>()) {
        std::cout << "Candidate received: " << cand << std::endl;
        pc->addRemoteCandidate(rtc::Candidate(cand));
    }
}

void AnswerPeer::startCommunication() {
    communicationLock.lock();

    communicationChannel->onOpen([&]() {
        std::cout << "Communication DataChannel opened" << std::endl;
        int fileNameSize = fileReader.name().size();
        // 8 bytes for file size
        uint64_t fileSize = fileReader.size();
        int chunks = fileReader.chunks();
        int infoSize = 4
            + fileNameSize
            + 4
            + 8
            + 10;
        unsigned char* info = new unsigned char[infoSize];
        memcpy(info, &fileNameSize, 4);
        memcpy(info + 4, fileReader.name().data(), fileReader.name().size());
        memcpy(info + 4 + fileNameSize, &chunks, 4);
        memcpy(info + 4 + fileNameSize + 4, &fileSize, 8);
        memcpy(info + 4 + fileNameSize + 4 + 8, fileReader.getID().data(), 10);
        communicationChannel->send((rtc::byte*)info, infoSize);
        });

    communicationChannel->onMessage([&](rtc::message_variant message) {
        bool ack = 0;
        memcpy(&ack, std::get<rtc::binary>(message).data(), 1);
        assert(ack && "File transfer not acknoledged");
        fileTransferLock.unlock();
        });

    communicationChannel->onClosed([&]() {
        std::cout << "Communication DataChannel closed" << std::endl;
        });

    communicationLock.unlock();
}

void AnswerPeer::beginFileTransfer()
{
    fileTransferLock.lock();
    open = true;
    for (int i = 0; i < threadCount; i++) {
        auto dc = pc->createDataChannel(std::to_string(i));
        dataChannels.push_back(dc);
    }
    for (auto& dc : dataChannels) {
        dc->onOpen([this, &dc]() {
            std::cout << "DataChannel " << dc->label() << " opened" << std::endl;
            this->threads.push_back(std::thread(FileTransfer, dc, std::ref(this->fileReader)));
            });
        dc->onClosed([this, &dc]() {
            std::cout << "DataChannel " << dc->label() << " closed" << std::endl;
            this->completionLock.unlock();
            });
    }
    fileTransferLock.unlock();
}

void FileTransfer(std::shared_ptr<rtc::DataChannel> dc, FileReader& fileReader) {
    while (fileReader.hasNext()) {
        dc->send(fileReader.readNextPacket());
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    if (dc->isOpen()) {
        dc->close();
    }
    std::cout << "Closing thread for " << dc->label() << std::endl;
}