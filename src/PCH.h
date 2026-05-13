#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#    define NOMINMAX
#endif

#include <Windows.h>
#include <ShlObj.h>
#include <KnownFolders.h>

#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>

#include "sfse_common/sfse_version.h"
#include "sfse/PluginAPI.h"

#if defined(SFSE_TEMPLATE_HAVE_SPDLOG) && __has_include(<spdlog/spdlog.h>)
#    include <spdlog/spdlog.h>
#    include <spdlog/sinks/basic_file_sink.h>
#    define SFSE_TEMPLATE_SPDLOG 1
#else
#    define SFSE_TEMPLATE_SPDLOG 0
#endif
