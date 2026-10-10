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

#include "Utils/ImageIO.hpp"
#include "Utils/Folders.hpp"
#include "utils/LoadPng.h"
#include "utils/LoadJpeg.h"
#include "Platform/Platform.hpp"

#include <cstdio>
#include <iostream>

namespace
{
// Generous enough for the largest shipped texture, but no longer a fixed cap:
// loaders call ensureCapacity() with the real decoded size and grow as needed.
constexpr size_t kDefaultImageCapacity = 1024 * 1024 * 4;
} // namespace

ImageRec::ImageRec()
{
	data = (uint8_t*)malloc(kDefaultImageCapacity);
	capacity_ = (data != nullptr) ? kDefaultImageCapacity : 0;
}

ImageRec::~ImageRec()
{
	free(data);
	data = NULL;
	capacity_ = 0;
}

bool ImageRec::ensureCapacity(size_t bytes)
{
	if (bytes <= capacity_) {
		return true;
	}

	uint8_t* resized = (uint8_t*)realloc(data, bytes);
	if (resized == nullptr) {
		return false;
	}
	data = resized;
	capacity_ = bytes;
	return true;
}

bool load_image(const char* file_name, ImageRec& tex, ProgressCallback callback)
{
	//Game::LoadingScreen();
	callback();
	if (tex.data == NULL) {
		return false;
	}

	const char* ptr = strrchr((char*)file_name, '.');
	if (ptr) {
		if (strcasecmp(ptr + 1, "png") == 0) {
			return load_png(file_name, tex);
		}
		else if (strcasecmp(ptr + 1, "jpg") == 0) {
			return load_jpg(file_name, tex);
		}
	}

	std::cerr << "Unsupported image type" << std::endl;
	return false;
}

size_t ImageRec::capacity() const
{
    return capacity_;
}
