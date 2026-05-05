add_library(raven
    raven/SocketServer.cpp
    raven/ConnectionPool.cpp

    raven/conn/Connection.cpp

    raven/config/SSLConfig.cpp
    raven/ssl/OpenSSLImplHelper.cpp
)

if (WIN32)
elseif(UNIX)
    target_sources(raven PUBLIC
        raven/impl/linux/LinuxConnection.cpp
        raven/impl/linux/LinuxConnectionPool.cpp
        raven/impl/linux/LinuxSocket.cpp
    )
endif()

if(RAVEN_DEBUG_LOGGING)
    target_compile_definitions(raven PUBLIC -DDEBUG_LOGGING=true)
endif()

find_package(OpenSSL REQUIRED)
target_link_libraries(raven PUBLIC openssl::openssl)
target_include_directories(raven PUBLIC ./)
# vim:ft=cmake
