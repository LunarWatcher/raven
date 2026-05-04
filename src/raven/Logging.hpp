#pragma once

#include <cstdio>

namespace raven {

#ifdef DEBUG_LOGGING
#define RavenVaradic(...) , ##__VA_ARGS__
#define RavenLog(pattern, ...) printf(pattern RavenVaradic(__VA_ARGS__))
#else
#define RavenLog(pattern, ...)
#endif

}
