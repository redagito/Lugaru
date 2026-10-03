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

#ifndef _COMMANDLINE_HPP_
#define _COMMANDLINE_HPP_

// optionparser.h pulls in <cstdlib> without first establishing namespace std,
// which strict compilers reject. Include the standard headers it relies on up
// front so the header is self-contained.
#include <cstddef>
#include <cstdlib>
#include <string>

#include "Thirdparty/optionparser.h"

enum optionIndex
{
	UNKNOWN,
	VERSION,
	HELP,
	FULLSCREEN,
	NOMOUSEGRAB,
	SOUND,
	OPENALINFO,
	SHOWRESOLUTIONS,
	DEVTOOLS,
	CMD
};

/** Number of options + 1. */
const int commandLineOptionsNumber = 11;

/** Descriptor table used to parse argv. */
extern const option::Descriptor usage[];

/** Parsed command line options, indexed by optionIndex. */
extern option::Option commandLineOptions[commandLineOptionsNumber];

/** Scratch storage for option::Parser; delete with `delete[]`. */
extern option::Option* commandLineOptionsBuffer;

#endif // _COMMANDLINE_HPP_