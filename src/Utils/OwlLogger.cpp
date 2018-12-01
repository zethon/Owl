// Owl - www.owlclient.com
// Copyright (c) 2012-2018, Adalid Claure <aclaure@gmail.com>

#pragma once

#include "OwlLogger.h"

namespace owl
{

static const char* GLOBAL_LOGGER = "Owl";

SpdLogPtr initializeLogger(const std::string& name)
{
    if (!spdlog::get(GLOBAL_LOGGER))
    {
        spdlog::stdout_color_mt(owl::GLOBAL_LOGGER);

#ifdef RELEASE
        spdlog::set_level(spdlog::level::off);
#else
        spdlog::set_level(spdlog::level::trace);
#endif
    }

    SpdLogPtr logger = spdlog::get(name);
    if (!logger)
    {
        logger = spdlog::get(GLOBAL_LOGGER)->clone(name);
        spdlog::register_logger(logger);
    }

    return logger;
}

} // namespace