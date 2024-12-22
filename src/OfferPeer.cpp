#include "Peer.h"
#include "fmt/core.h"

OfferPeer::OfferPeer(rtc::Configuration config, std::string selfID) :
    Peer(config, selfID) {
    pc->onLocalDescription([&](rtc::Description sdp) {
        std::cout << "Local Description created" << std::endl;

        r = cpr::Patch(cpr::Url{ baseUrl + selfUserID + ".json" },
            cpr::Body{ json{ {"sdp", std::string(sdp)}, {"peer", ""} }.dump() },
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
        });

    pc->onIceStateChange([&](rtc::PeerConnection::IceState state) {
        std::cout << "[ICE State: " << state << "]" << std::endl;
        });

}

void OfferPeer::waitForDataChannel() {
    communicationChannel = pc->createDataChannel("communication");

    int attempts = 0;
    while (attempts < LISTEN_ATTEMPTS && remoteUserID.size() == 0) {
        r = cpr::Get(cpr::Url{ baseUrl + selfUserID + ".json" });

        assert(r.status_code == 200 && fmt::format("Request failed with status code: %d", r.status_code).c_str());

        json j = json::parse(r.text);
        remoteUserID = j["peer"].get<std::string>();

        std::this_thread::sleep_for(std::chrono::seconds(5));
        attempts++;
    }

    assert(remoteUserID.size() != 0 && "No peer connected");

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

void OfferPeer::startCommunication()
{
    communicationChannel->onOpen([&]() {
        std::cout << "DataChannel opened: " << communicationChannel->label() << std::endl;
        communicationLock.unlock();
        });

    communicationChannel->onMessage([&](rtc::message_variant message) {
        communicationLock.lock();
        rtc::binary data = std::get<rtc::binary>(message);
        int fileNameSize, chunks;
        uint64_t fileSize;
        std::string fileID(10, 0);
        memcpy(&fileNameSize, data.data(), 4);
        std::string fileName(fileNameSize, 0);
        memcpy(fileName.data(), data.data() + 4, fileNameSize);
        memcpy(&chunks, data.data() + 4 + fileNameSize, 4);
        memcpy(&fileSize, data.data() + 4 + fileNameSize + 4, 8);
        memcpy(fileID.data(), data.data() + 4 + fileNameSize + 4 + 8, 10);

        fileWriter.open(fileName, fileID, chunks, fileSize);

        int ack[] = { 1 };

        communicationChannel->send((rtc::byte*)ack, 1);
        fileTransferLock.unlock();
        });


    communicationChannel->onClosed([&]() {
        std::cout << "Communication DataChannel closed" << std::endl;
        communicationLock.unlock();
        });
}

void OfferPeer::beginFileTransfer()
{
    fileTransferLock.lock();
    pc->onDataChannel([&](std::shared_ptr<rtc::DataChannel> datachannel) {
        std::cout << "DataChannel created: " << datachannel->label() << std::endl;
        dataChannels.push_back(datachannel);
        datachannel->onMessage([this, datachannel](rtc::message_variant message) {
            fileWriter.write(std::get<rtc::binary>(message));
            if (fileWriter.isComplete()
                &&
                datachannel->isOpen()) {
                datachannel->close();
            }
            });
        datachannel->onClosed([datachannel]() {
            std::cout << "DataChannel closed: " << datachannel->label() << std::endl;
            });
        });
    fileTransferLock.unlock();
}

bool OfferPeer::transferringData()
{
    return !fileWriter.isComplete();
}