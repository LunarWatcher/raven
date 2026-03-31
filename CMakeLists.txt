cmake_minimum_required(VERSION 3.27)

if (ENABLE_CONAN)
    include(FetchContent)
    FetchContent_Declare(conan
        GIT_REPOSITORY https://github.com/conan-io/cmake-conan
        GIT_TAG develop2
    )
    FetchContent_MakeAvailable(conan)
    set(CMAKE_PROJECT_TOP_LEVEL_INCLUDES ${conan_SOURCE_DIR}/conan_provider.cmake)
endif()

project(raven)

set (CMAKE_CXX_STANDARD 23)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    message (STATUS "raven: Running standalone")
    set(RAVEN_STANDALONE ON)

    if (NOT WIN32)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
    endif()

    option(RAVEN_ENABLE_DEMOS "Whether or not to build demos" ON)
    option(RAVEN_BUILD_TESTS "Whether or not to build tests" ON)
    option(RAVEN_DEBUG_LOGGING "Whether or not to enable debug logging. This should never be turned on unless the goal is debugging Raven" ON)
else()
    message (STATUS "raven: Running as a submodule")
    set(RAVEN_STANDALONE OFF)

    option(RAVEN_ENABLE_DEMOS "Whether or not to build demos" OFF)
    option(RAVEN_BUILD_TESTS "Whether or not to build tests" OFF)
    option(RAVEN_DEBUG_LOGGING "Whether or not to enable debug logging. This should never be turned on unless the goal is debugging Raven" OFF)
endif()

# Unused, but reserved for future use
option(RAVEN_DOCS "Whether or not to build doxygen docs" OFF)
option(RAVEN_SANITISE "Whether or not to -fsanitise=undefined. No effect on Windows" OFF)

message(STATUS "Raven build configuration:")
message(STATUS "\t- Demos: ${RAVEN_ENABLE_DEMOS}")
message(STATUS "\t- Tests: ${RAVEN_BUILD_TESTS}")
message(STATUS "\t- Docs: ${RAVEN_DOCS}")
message(STATUS "\t- Sanitising: ${RAVEN_SANITISE}")
message(STATUS "\t- Debug logging: ${RAVEN_DEBUG_LOGGING}")

add_subdirectory(src)

if (RAVEN_BUILD_TESTS)
    add_subdirectory(tests EXCLUDE_FROM_ALL)

    add_custom_target(test
        COMMAND tests
        DEPENDS tests
        COMMENT "Test raven"
    )
endif()
if (RAVEN_ENABLE_DEMOS)
    add_subdirectory(demos)
endif()
# vim:ft=cmake
