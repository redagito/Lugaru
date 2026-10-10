/*
Copyright (C) 2016-2017 - Lugaru contributors (see AUTHORS file)

This file is part of Lugaru.

Lugaru is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

Lugaru is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Lugaru.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Utils/Log.hpp"

#include "Utils/Folders.hpp"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <memory>

namespace
{

std::shared_ptr<spdlog::logger> logger;

// The default level. Info is what a playtest wants: enough to say what loaded
// and how long it took, without every allocation narrating itself. Debug is
// there for when something needs following.
constexpr spdlog::level::level_enum kDefaultLevel = spdlog::level::info;

} // namespace

void Log::init(const std::string& logFilePath)
{
    // A fresh run starts from an empty file, so what is in it describes one
    // launch rather than an accumulating history that has to be dated by hand.
    // The multithreaded sink, because the key-select thread logs too.
    auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath, true);
    fileSink->set_level(kDefaultLevel);

    auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    consoleSink->set_level(kDefaultLevel);

    logger = std::make_shared<spdlog::logger>(
        "lugaru",
        spdlog::sinks_init_list{ fileSink, consoleSink });
    logger->set_level(kDefaultLevel);
    // Long lines, because asset paths are long and wrapping them makes the
    // file harder to read next to the console copy.
    logger->set_pattern("%^%L%$ [%H:%M:%S.%e] %v");

    spdlog::register_logger(logger);
    spdlog::set_default_logger(logger);
}

void Log::shutdown()
{
    if (logger) {
        logger->flush();
    }
    spdlog::shutdown();
    logger.reset();
}

void Log::setLevel(const std::string& levelName)
{
    if (!logger) {
        return;
    }
    logger->set_level(spdlog::level::from_str(levelName));
    for (auto& sink : logger->sinks()) {
        sink->set_level(spdlog::level::from_str(levelName));
    }
}

void Log::trace(const std::string& message)
{
    if (logger) {
        logger->trace(message);
    }
}

void Log::debug(const std::string& message)
{
    if (logger) {
        logger->debug(message);
    }
}

void Log::info(const std::string& message)
{
    if (logger) {
        logger->info(message);
    }
}

void Log::warn(const std::string& message)
{
    if (logger) {
        logger->warn(message);
    }
}

void Log::error(const std::string& message)
{
    if (logger) {
        logger->error(message);
    }
}
