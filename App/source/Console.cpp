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

#include "Console.hpp"

void Console::push(std::string text)
{
	historyShift();
	history.front() = std::move(line);
	line = std::move(text);
	selected = 0;
}

void Console::submit()
{
	if (line.empty()) {
		return;
	}
	historyShift();
	history.front() = std::move(line);
	line.clear();
	selected = 0;
}

void Console::historyShift()
{
	std::copy_backward(history.begin(), history.end() - 1, history.end());
}
