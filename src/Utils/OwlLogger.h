// Owl - www.owlclient.com
// Copyright (c) 2012-2018, Adalid Claure <aclaure@gmail.com>

#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace owl
{

using SpdLogPtr = std::shared_ptr<spdlog::logger>;

[[maybe_unused]] SpdLogPtr rootLogger();
SpdLogPtr initializeLogger(const std::string& name);

} // namespace