#include "Utils/LoadPng.h"

#include <libpng16/png.h>
#include <string>
#include <iostream>

#include "Utils/ImageIO.hpp"
#include "Platform/Platform.hpp"

#ifdef LUGARU_PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#define Polygon WinPolygon
#include <windows.h>
#undef Polygon
#include <GL/gl.h>
#else
#include "Graphic/gamegl.hpp"
#endif

/* stolen from public domain example.c code in libpng distribution. */
bool load_png(const char* file_name, ImageRec& tex)
{
	bool hasalpha = false;
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type;
	bool retval = false;
	png_byte** row_pointers = NULL;
	errno = 0;
	FILE* fp = fopen(file_name, "rb");

	if (fp == NULL) {
		perror((std::string("Couldn't open file ") + file_name).c_str());
		return false;
	}

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		goto png_done;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		goto png_done;
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		goto png_done;
	}

	png_init_io(png_ptr, fp);
	png_read_png(png_ptr, info_ptr,
		PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING,
		NULL);
	png_get_IHDR(png_ptr, info_ptr, &width, &height,
		&bit_depth, &color_type, &interlace_type, NULL, NULL);

	if (bit_depth != 8) { // transform SHOULD handle this...
		goto png_done;
	}

	if (color_type & PNG_COLOR_MASK_PALETTE) { // !!! FIXME?
		goto png_done;
	}

	if ((color_type & PNG_COLOR_MASK_COLOR) == 0) { // !!! FIXME?
		goto png_done;
	}

	hasalpha = ((color_type & PNG_COLOR_MASK_ALPHA) != 0);
	row_pointers = png_get_rows(png_ptr, info_ptr);
	if (!row_pointers) {
		goto png_done;
	}

	// Size the destination from the real image dimensions. The buffer used to be
	// a fixed 4 MiB, so any image larger than 1024x1024 overflowed it.
	if (!tex.ensureCapacity((size_t)width * (size_t)height * 4)) {
		goto png_done;
	}

	if (!hasalpha) {
		png_byte* dst = tex.data;
		for (int i = height - 1; i >= 0; i--) {
			png_byte* src = row_pointers[i];
			for (unsigned j = 0; j < width; j++) {
				dst[0] = src[0];
				dst[1] = src[1];
				dst[2] = src[2];
				dst[3] = 0xFF;
				src += 3;
				dst += 4;
			}
		}
	}

	else {
		png_byte* dst = tex.data;
		int pitch = width * 4;
		for (int i = height - 1; i >= 0; i--, dst += pitch) {
			memcpy(dst, row_pointers[i], pitch);
		}
	}

	tex.sizeX = width;
	tex.sizeY = height;
	tex.bpp = 32;
	retval = true;

png_done:
	if (!retval) {
		std::cerr << "There was a problem loading " << file_name << std::endl;
	}
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	if (fp) {
		fclose(fp);
	}
	return (retval);
}
