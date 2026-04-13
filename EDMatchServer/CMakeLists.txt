cmake_minimum_required(VERSION 3.20)
project(EDMatchServer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# vcpkg toolchain
set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "Vcpkg toolchain file")

set(SOURCES
    main.cpp
    Core/IOCPServer.cpp
    Core/Session.cpp
    Core/PacketHandler.cpp
    DB/RedisClient.cpp
    DB/MySQLPool.cpp
    Auth/LoginHandler.cpp
    Auth/TokenManager.cpp
    Match/MatchQueue.cpp
    Match/MatchMaker.cpp
)

set(HEADERS
    Common/Platform.h
    Common/Protocol.h
    Common/Packet.h
    Common/Logger.h
    Core/IOCPServer.h
    Core/Session.h
    Core/PacketHandler.h
    DB/RedisClient.h
    DB/MySQLPool.h
    Auth/LoginHandler.h
    Auth/TokenManager.h
    Match/MatchQueue.h
    Match/MatchMaker.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Winsock
target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)

# hiredis
find_package(hiredis CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE hiredis::hiredis)

# nlohmann/json
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)

# libmysql
find_package(unofficial-libmysql REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE unofficial::libmysql::libmysql)

# OpenSSL (SHA256 for password hashing)
find_package(OpenSSL REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE OpenSSL::SSL OpenSSL::Crypto)
