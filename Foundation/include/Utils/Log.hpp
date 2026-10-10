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

#ifndef _LOG_HPP_
#define _LOG_HPP_

// The one logging facade for the whole program.
//
// There was no logging before, only ~78 ad-hoc calls to std::cerr, std::cout,
// printf and perror scattered across 16 files. Three things were wrong with
// that: nothing persisted (and on Windows a double-clicked exe has no console
// at all, so almost none of it was ever seen), there were no levels so there
// was no way to turn the noise down, and a test could not capture any of it.
//
// This is a thin wrapper over spdlog. Two sinks are installed at startup:
//
//   - a file, truncated at every launch, next to the user data so it survives
//     a launcher with no console. This is the one to read after a crash.
//   - the console, so a terminal still shows everything as it happens.
//
// The level is a compile-time default that a command-line switch or an
// environment variable can lower, because diagnostics are for finding bugs,
// not for shipping debt.

#include <chrono>
#include <string>

namespace Log
{

// Writes the file truncation reason, so a fresh log records why the last one
// went away. Call once, before anything else logs.
void init(const std::string& logFilePath);

// Flushes and closes the sinks. Safe to call more than once.
void shutdown();

// The active level. Everything at or above it is emitted.
void setLevel(const std::string& levelName);

// The five calls, one per severity. The point of the exercise: the call site
// names the severity, not the formatting.
void trace(const std::string& message);
void debug(const std::string& message);
void info(const std::string& message);
void warn(const std::string& message);
void error(const std::string& message);

// Times a scope and reports how long it took when it ends. The log used to be
// silent on the happy path, which meant a slow level load was indistinguishable
// from a fast one; now every phase reports itself.
class ScopedTimer
{
public:
    explicit ScopedTimer(std::string what);
    ~ScopedTimer();

    ScopedTimer(const ScopedTimer&) = delete;
    ScopedTimer& operator=(const ScopedTimer&) = delete;

private:
    std::string what;
    std::chrono::steady_clock::time_point started;
};

} // namespace Log

#endif
