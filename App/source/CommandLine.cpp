/*
Copyright (C) 2003, 2010 - Wolfire Games
Copyright (C) 2010-2017 - Lugaru contributors (see AUTHORS file)

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

#include "CommandLine.hpp"

const option::Descriptor usage[] =
{
	{ UNKNOWN, 0, "", "", option::Arg::None, "USAGE: lugaru [options]\n\n"
										   "Options:" },
	{ VERSION, 0, "v", "version", option::Arg::None, " -v, --version     Print version and exit." },
	{ HELP, 0, "h", "help", option::Arg::None, " -h, --help        Print usage and exit." },
	{ FULLSCREEN, 1, "f", "fullscreen", option::Arg::None, " -f, --fullscreen  Start the game in fullscreen mode." },
	{ FULLSCREEN, 0, "w", "windowed", option::Arg::None, " -w, --windowed    Start the game in windowed mode (default)." },
	{ NOMOUSEGRAB, 1, "", "nomousegrab", option::Arg::None, " --nomousegrab     Disable mousegrab." },
	{ NOMOUSEGRAB, 0, "", "mousegrab", option::Arg::None, " --mousegrab       Enable mousegrab (default)." },
	{ SOUND, 1, "", "nosound", option::Arg::None, " --nosound         Disable sound." },
	{ OPENALINFO, 0, "", "openal-info", option::Arg::None, " --openal-info     Print info about OpenAL at launch." },
	{ SHOWRESOLUTIONS, 0, "", "showresolutions", option::Arg::None, " --showresolutions List the resolutions found by SDL at launch." },
	{ DEVTOOLS, 0, "d", "devtools", option::Arg::None, " -d, --devtools    Enable dev tools: console, level editor and debug info." },
	{ CMD, 0, "c", "command", option::Arg::Optional, " -c, --command    Run this command at game start. May be used to load a map." },
	{ 0, 0, 0, 0, 0, 0 }
};

option::Option commandLineOptions[commandLineOptionsNumber];
option::Option* commandLineOptionsBuffer;