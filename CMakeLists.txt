cmake_minimum_required(VERSION 3.5)
project(yeet VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps")
# set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

# Add debug flags
if(CMAKE_BUILD_TYPE MATCHES Debug)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")
endif()

file(GLOB_RECURSE LIB_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
add_executable(${CMAKE_PROJECT_NAME} ${LIB_SOURCES})

if(CMAKE_BUILD_TYPE MATCHES Debug)
  target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC DEBUG=1)
endif()

target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC ROOT_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")

# Multi-threading
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Threads::Threads)

# Libdatachannel
set(LIBDATACHANNEL_DIR "${LIB_DIR}/libdatachannel")

add_subdirectory("${LIBDATACHANNEL_DIR}")
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE LibDataChannel::LibDataChannelStatic)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE "${LIBDATACHANNEL_DIR}/include")

# nlohmann json
set(JSON_DIR "${LIBDATACHANNEL_DIR}/deps/json")

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE "${JSON_DIR}/include")

# Nanoid
set(NANOID_DIR "${LIB_DIR}/nanoid")

add_subdirectory("${NANOID_DIR}")
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE nanoid)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE "${NANOID_DIR}/inc")

# FMT
set(FMT_DIR "${LIB_DIR}/fmt")

add_subdirectory("${FMT_DIR}")

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE fmt::fmt)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE "${FMT_DIR}/include")

# CPR
set(CPR_DIR "${LIB_DIR}/cpr")
set(BUILD_SHARED_LIBS OFF)
add_subdirectory("${CPR_DIR}")
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE cpr::cpr)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE "${CPR_DIR}/include")

# NLOHMANN JSON set(JSON_DIR "${LIB_DIR}/json")
#
# add_subdirectory("${JSON_DIR}")
#
# target_link_libraries(${CMAKE_PROJECT_NAME} nlohmann_json::nlohmann_json)
# target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
# "${JSON_DIR}/include")