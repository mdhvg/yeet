#include "Peer.h"

enum Mode {
    OFFERER,
    ANSWERER
};

struct RunParams {
    bool interactive = true;
    Mode mode = Mode::OFFERER;
};

inline RunParams runParams;
inline Mode mode;

void addValueArgument(std::unordered_map<std::string, std::string>& args, char* key, char* value) {
    args[key] = value;
}

void addFlagArgument(std::unordered_map<std::string, std::string>& args, char* key) {
    args[key] = "1";
}

void parseArguments(std::unordered_map<std::string, std::string>& args, int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
            continue;
        }
        switch (argv[i][1])
        {
        case 'i':
        case 't':
        case 'r':
        case 'u': {
            assert(!(i + 1 == argc || argv[i + 1][0] == '-') && "Expected value for argument: ");
            addValueArgument(args, argv[i] + 1, argv[i + 1]);
            break;
        }
        default:
            break;
        }
    }
}

int main(int argc, char** argv) {
    rtc::InitLogger(rtc::LogLevel::Fatal);

    std::unordered_map<std::string, std::string> args;
    args["t"] = "4";
    parseArguments(args, argc, argv);
    assert(args.find("u") != args.end() && "Username is required");

    if (args.find("i") != args.end()) {
        runParams.interactive = false;
    }

    if (args.find("r") != args.end()) {
        runParams.mode = Mode::ANSWERER;
    }

    for (auto& arg : args) {
        std::cout << arg.first << ": " << arg.second << std::endl;
    }

    rtc::Configuration config;
    config.iceServers.push_back(rtc::IceServer("stun:stun.l.google.com:19302"));
    config.iceServers.push_back(rtc::IceServer("stun:stun1.l.google.com:19302"));
    config.iceServers.push_back(rtc::IceServer("stun:stun2.l.google.com:19302"));
    config.iceServers.push_back(rtc::IceServer("stun:stun3.l.google.com:19302"));
    config.iceServers.push_back(rtc::IceServer("stun:stun4.l.google.com:19302"));

    config.enableIceTcp = true;

    std::shared_ptr<Peer> peer;

    if (runParams.mode == Mode::OFFERER) {
        peer = std::make_shared<OfferPeer>(config, args["u"]);
    }
    else {
        peer = std::make_shared<AnswerPeer>(config, args["u"], args["r"], args["i"], std::stoi(args["t"]));
    }

    peer->waitForDataChannel();
    peer->startCommunication();
    peer->beginFileTransfer();

    std::this_thread::sleep_for(std::chrono::seconds(6));
    while (peer->transferringData()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << "File transfer complete" << std::endl;
    return 0;
}
