// Owl - www.owlclient.com
// Copyright (c) 2012-2019, Adalid Claure <aclaure@gmail.com>

#include <spdlog/sinks/stdout_color_sinks.h>
#include "OwlLogger.h"

namespace owl
{

static const char* GLOBAL_LOGGER = "Owl";

[[maybe_unused]] SpdLogPtr rootLogger()
{
    SpdLogPtr root = spdlog::get(GLOBAL_LOGGER);

    if (!root)
    {
        root = spdlog::stdout_color_mt(owl::GLOBAL_LOGGER);
        spdlog::register_logger(root);

#ifdef RELEASE
        spdlog::set_level(spdlog::level::off);
#else
        spdlog::set_level(spdlog::level::trace);
#endif
    }

    return root;
}

SpdLogPtr initializeLogger(const std::string& name)
{
    owl::rootLogger();

    SpdLogPtr logger = spdlog::get(name);
    if (!logger)
    {
        logger = spdlog::get(GLOBAL_LOGGER)->clone(name);
        spdlog::register_logger(logger);
    }

    return logger;
}

} // namespace
