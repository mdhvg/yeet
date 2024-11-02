#include "Peer.h"
#include "fmt/core.h"

Peer::Peer(rtc::Configuration config, std::string selfUserID, std::string remoteUserID) :
    selfUserID(selfUserID),
    remoteUserID(remoteUserID),
    config(config) {
    pc = std::make_shared<rtc::PeerConnection>(config);
    communicationLock.lock();
    fileTransferLock.lock();
    completionLock.lock();
    std::cout << "Peer created" << std::endl;
}

Peer::Peer(rtc::Configuration config, std::string selfUserID) :
    selfUserID(selfUserID),
    config(config) {
    pc = std::make_shared<rtc::PeerConnection>(config);
    communicationLock.lock();
    fileTransferLock.lock();
    completionLock.lock();
    std::cout << "Peer created" << std::endl;
}

Peer::~Peer() {
    pc->close();
}