#pragma once

#if defined(__linux__)
#include "config_linux.h"
#elif defined(__APPLE__)
#include "config_macos.h"
#else
#error Unsupported platform
#endif
