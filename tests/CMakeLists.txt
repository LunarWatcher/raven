add_executable(tests
    src/http/HttpTests.cpp
)

find_package(cpr REQUIRED)

include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.13.0
)

FetchContent_MakeAvailable(Catch2)

target_link_libraries(tests
    Catch2::Catch2WithMain
    cpr::cpr
    raven
)

# vim:ft=cmake
