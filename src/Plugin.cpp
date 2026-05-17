#include "PCH.h"

namespace
{
    constexpr const char* kPluginName    = "sfse-template-plugin";
    constexpr const char* kPluginAuthor  = "your name";
    constexpr std::uint32_t kPluginVersion = 1;

    PluginHandle g_pluginHandle = static_cast<PluginHandle>(kPluginHandle_Invalid);

    // Resolves <My Documents>\My Games\Starfield\SFSE\Logs\<plugin>.log, the
    // standard location SFSE itself writes to. Returns empty path on failure.
    std::filesystem::path ResolveLogPath()
    {
        PWSTR documents = nullptr;
        if (FAILED(SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_DEFAULT, nullptr, &documents)) || !documents) {
            return {};
        }
        std::filesystem::path path = documents;
        CoTaskMemFree(documents);

        path /= L"My Games";
        path /= L"Starfield";
        path /= L"SFSE";
        path /= L"Logs";

        std::error_code ec;
        std::filesystem::create_directories(path, ec);

        path /= std::string(kPluginName) + ".log";
        return path;
    }

#if SFSE_TEMPLATE_SPDLOG
    void InitLogging(const std::filesystem::path& logPath)
    {
        try {
            auto logger = spdlog::basic_logger_mt(kPluginName, logPath.string(), /*truncate*/ true);
            logger->set_pattern("[%H:%M:%S.%e] [%l] %v");
            logger->flush_on(spdlog::level::info);
            spdlog::set_default_logger(std::move(logger));
            spdlog::set_level(spdlog::level::info);
        } catch (...) {
            // Logging is best-effort; never let it kill the plugin.
        }
    }

    template <typename... Args>
    void LogInfo(spdlog::format_string_t<Args...> fmt, Args&&... args)
    {
        spdlog::info(fmt, std::forward<Args>(args)...);
    }
#else
    std::mutex g_logMutex;
    std::ofstream g_logFile;

    void InitLogging(const std::filesystem::path& logPath)
    {
        std::scoped_lock lock(g_logMutex);
        g_logFile.open(logPath, std::ios::out | std::ios::trunc);
    }

    void LogLine(std::string_view line)
    {
        std::scoped_lock lock(g_logMutex);
        if (g_logFile.is_open()) {
            g_logFile << line << '\n';
            g_logFile.flush();
        }
    }

    template <typename... Args>
    void LogInfo(std::string_view fmt, Args&&... args)
    {
        // Minimal fallback: caller supplies a pre-formatted message via the
        // `{}`-less overload below, or formats themselves. Keep it simple.
        LogLine(fmt);
    }

    void LogInfo(std::string&& msg) { LogLine(msg); }
#endif

    const char* DescribeMessage(std::uint32_t type)
    {
        switch (type) {
        case SFSEMessagingInterface::kMessage_PostLoad:         return "PostLoad";
        case SFSEMessagingInterface::kMessage_PostPostLoad:     return "PostPostLoad";
        case SFSEMessagingInterface::kMessage_PostDataLoad:     return "PostDataLoad";
        case SFSEMessagingInterface::kMessage_PostPostDataLoad: return "PostPostDataLoad";
        case SFSEMessagingInterface::kMessage_PreSaveGame:      return "PreSaveGame";
        case SFSEMessagingInterface::kMessage_PostSaveGame:     return "PostSaveGame";
        case SFSEMessagingInterface::kMessage_PreLoadGame:      return "PreLoadGame";
        case SFSEMessagingInterface::kMessage_PostLoadGame:     return "PostLoadGame";
        default:                                                return "Unknown";
        }
    }

    void OnSFSEMessage(SFSEMessagingInterface::Message* msg)
    {
        if (!msg) return;
#if SFSE_TEMPLATE_SPDLOG
        spdlog::info("SFSE message: type={} ({}) sender={} dataLen={}",
                     msg->type, DescribeMessage(msg->type),
                     msg->sender ? msg->sender : "(null)", msg->dataLen);
#else
        char buf[256];
        std::snprintf(buf, sizeof(buf),
                      "SFSE message: type=%u (%s) sender=%s dataLen=%u",
                      msg->type, DescribeMessage(msg->type),
                      msg->sender ? msg->sender : "(null)", msg->dataLen);
        LogInfo(std::string{buf});
#endif
    }
}

extern "C" {

__declspec(dllexport) SFSEPluginVersionData SFSEPlugin_Version =
{
    SFSEPluginVersionData::kVersion,

    kPluginVersion,
    "sfse-template-plugin",
    "your name",

    0,  // not address independent
    SFSEPluginVersionData::kStructureIndependence_1_14_70_Layout,
    { RUNTIME_VERSION_1_16_242, 0 },

    0,  // any SFSE version
    0, 0,
};

__declspec(dllexport) bool SFSEPlugin_Load(const SFSEInterface* sfse)
{
    const auto logPath = ResolveLogPath();
    if (!logPath.empty()) {
        InitLogging(logPath);
    }

    g_pluginHandle = sfse->GetPluginHandle();

#if SFSE_TEMPLATE_SPDLOG
    spdlog::info("{} v{} loaded", kPluginName, kPluginVersion);
    spdlog::info("SFSE version: 0x{:08X}  runtime version: 0x{:08X}  pluginHandle: {}",
                 sfse->sfseVersion, sfse->runtimeVersion, g_pluginHandle);
#else
    char buf[256];
    std::snprintf(buf, sizeof(buf), "%s v%u loaded", kPluginName, kPluginVersion);
    LogInfo(std::string{buf});
    std::snprintf(buf, sizeof(buf),
                  "SFSE version: 0x%08X  runtime version: 0x%08X  pluginHandle: %u",
                  sfse->sfseVersion, sfse->runtimeVersion, g_pluginHandle);
    LogInfo(std::string{buf});
#endif

    auto* messaging = static_cast<SFSEMessagingInterface*>(
        sfse->QueryInterface(kInterface_Messaging));
    if (messaging && messaging->interfaceVersion >= SFSEMessagingInterface::kInterfaceVersion) {
        if (!messaging->RegisterListener(g_pluginHandle, "SFSE", OnSFSEMessage)) {
#if SFSE_TEMPLATE_SPDLOG
            spdlog::warn("Failed to register SFSE messaging listener");
#else
            LogInfo(std::string{"Failed to register SFSE messaging listener"});
#endif
        }
    } else {
#if SFSE_TEMPLATE_SPDLOG
        spdlog::warn("SFSE messaging interface unavailable");
#else
        LogInfo(std::string{"SFSE messaging interface unavailable"});
#endif
    }

    return true;
}

}  // extern "C"
