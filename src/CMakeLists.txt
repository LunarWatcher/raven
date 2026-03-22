add_library(raven
  raven/SocketServer.cpp
)

if (WIN32)

elseif(UNIX)
    target_sources(raven PUBLIC
        raven/impl/linux/Connection.cpp
        raven/impl/linux/Socket.cpp
    )
endif()

target_include_directories(raven PUBLIC ./)
# vim:ft=cmake
