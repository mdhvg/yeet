#include "Yeet.h"
#include "FileReader.h"
#include "FileWriter.h"

/*
Communication channel message format:
    - Answerer (Sender)
        - [4 bytes] = File name length (n) (Little endian)
        - [n bytes] = File name in bytes
        - [4 bytes] = Number of chunks (Little endian)
        - [8 bytes] = File size (Little endian)
        - [10 bytes] = File ID

    - Offerer (Receiver)
        - [1 byte] = True if acknoledged, False if not
*/

class Peer {
public:
    Peer(rtc::Configuration config, std::string selfUserID, std::string remoteUserID);
    Peer(rtc::Configuration config, std::string selfUserID);
    ~Peer();

    virtual void waitForDataChannel() = 0;
    virtual void startCommunication() = 0;
    virtual void beginFileTransfer() = 0;
    virtual bool transferringData() = 0;
public:
    // TODO: Optimize this to reduce pointer dereferencing
    std::vector<std::shared_ptr<rtc::DataChannel>> dataChannels;
    std::mutex completionLock;
protected:
    std::shared_ptr<rtc::DataChannel> communicationChannel;
    std::shared_ptr<rtc::PeerConnection> pc;
    std::vector<rtc::Candidate> candidates;
    std::string selfUserID;
    std::string remoteUserID;
    cpr::Response r;
    std::shared_ptr<std::fstream> fileHandle;
    std::mutex communicationLock;
    std::mutex fileTransferLock;
private:
    rtc::Configuration config;
};

// Receiver peer
class OfferPeer : public Peer {
public:
    OfferPeer(rtc::Configuration config, std::string selfID);

    void waitForDataChannel() override;
    void startCommunication() override;
    void beginFileTransfer() override;
    bool transferringData() override;
private:
    FileWriter fileWriter;
};

// Sender peer
class AnswerPeer : public Peer {
public:
    AnswerPeer(rtc::Configuration config, std::string selfID, std::string remoteID, std::string fileName, int threadCount);

    void waitForDataChannel() override;
    void startCommunication() override;
    void beginFileTransfer() override;
    bool transferringData() override;
private:
    FileReader fileReader;
    int threadCount;
};