add_executable(
    basic-http-server src/BasicHttpServer.cpp
)
target_link_libraries(basic-http-server PRIVATE raven)
