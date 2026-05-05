#pragma once

#include <array>
#include <functional>
#include <cstdint>

namespace raven {

static constexpr size_t WindowSize = 16'384;
using Buffer = std::array<char, WindowSize>;

using WriteCallback = std::function<size_t(
    Buffer& out, size_t nextStartPosition
)>;

struct WriteBuffer {
    WriteCallback func;
    size_t lastIndex = 0;
};


}
