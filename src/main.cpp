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

// void messageHandler(std::shared_ptr<rtc::DataChannel> dc, std::mutex& dcOpenLock) {
//     dcOpenLock.lock();
//     if (mode == Mode::OFFERER) {
//         fs::path filePath = fs::path("send") / FILE_NAME;
//         std::ifstream file(filePath, std::ios::binary);
//         std::cout << fs::current_path() << std::endl;
//         if (file.is_open()) {
//             file.seekg(0, std::ios::end);
//             int size = file.tellg();
//             file.seekg(0, std::ios::beg);
//             char* data = new char[size];
//             file.read(data, size);
//             rtc::binary bytes(size);
//             bytes.assign(reinterpret_cast<std::byte*>(data), reinterpret_cast<std::byte*>(data) + size);
//             dc->send(bytes);
//         }
//     }
//     // dc->close();
// }

// void messageHandler(std::shared_ptr<rtc::DataChannel> dc, std::mutex& dcOpenLock) {
//     dcOpenLock.lock();
//     std::string message;
//     std::cout << (dc->isOpen() && message.find("quit") == std::string::npos ? "True" : "False") << std::endl;
//     while (dc->isOpen() && message.find("quit") == std::string::npos) {
//         std::cout << "Enter message: ";
//         std::cin >> message;
//         dc->send(message);
//     }
//     dc->close();
// }

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
    // rtc::InitLogger(rtc::LogLevel::Debug);

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

    // std::vector<std::thread> threads(std::stoi(args["t"]));
    // for (int i = 0; i < std::stoi(args["t"]); i++) {
    //     if (runParams.mode == Mode::OFFERER) {
    //         threads[i] = std::thread(transmitter, dc, std::ref(dcOpenLock), std::ref(fileReader));
    //     }
    //     else {
    //         threads[i] = std::thread(messageHandler, dc, std::ref(dcOpenLock));
    //     }
    // }

    while (peer->open) {
        peer->open = false;
        for (auto& dc : peer->dataChannels) {
            if (dc->isOpen()) {
                peer->open = true;
                break;
            }
        }
    }

    // for (auto& thread : threads) {
    //     thread.join();
    // }

    return 0;
}
