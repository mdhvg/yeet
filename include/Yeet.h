#ifndef YEET_H
#define YEET_H
#include "cpr/cpr.h"
#include "nlohmann/json.hpp"
#include "rtc/rtc.hpp"
#include <assert.h>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>
#include <memory>
#include <nanoid/nanoid.h>
#include <thread>
#include <unordered_map>

#define ModeString(mode) (mode == Mode::OFFERER ? "OFFERER" : "ANSWERER")
#define HEADER_SIZE (10 + 4 + 4)
#define READ_SIZE ((65535) - HEADER_SIZE)
#define LISTEN_ATTEMPTS 50
#define WAIT_TIME 20

using json = nlohmann::json;
namespace fs = std::filesystem;

inline std::string baseUrl = "";
inline fs::path basePath = "testshare";

#endif // YEET_H
