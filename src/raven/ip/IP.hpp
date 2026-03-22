#pragma once

#include <string>
namespace raven::ip {

enum class IPVersion {
    IPv4,
    IPv6
};

struct IP {
    IPVersion version;
    std::string dotNotation;
};

}
