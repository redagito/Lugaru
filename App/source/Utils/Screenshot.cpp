#include "Utils/Screenshot.h"

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <iostream>

#include <libpng16/png.h>

#include "Graphic/gamegl.hpp"

#include "Utils/Folders.hpp"
#include "Platform/Platform.hpp"
#include "WindowContext.hpp"

bool save_screenshot(const char* file_name)
{
	const char* ptr = strrchr((char*)file_name, '.');
	if (ptr) {
		if (strcasecmp(ptr + 1, "png") == 0) {
			return save_screenshot_png((Folders::getScreenshotDir() + '/' + file_name).c_str());
		}
	}

	std::cerr << "Unsupported image type" << std::endl;
	return false;
}

bool save_screenshot_png(const char* file_name)
{
	FILE* fp = NULL;
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	bool retval = false;

	errno = 0;
	fp = fopen(file_name, "wb");
	if (fp == NULL) {
		perror((std::string("Couldn't open file ") + file_name).c_str());
		return false;
	}

	png_bytep* row_pointers = new png_bytep[WindowContext::kContextHeight];
	png_bytep screenshot = new png_byte[WindowContext::kContextWidth * WindowContext::kContextHeight * 3];
	if ((!screenshot) || (!row_pointers)) {
		goto save_png_done;
	}

	glGetError();
	glReadPixels(0, 0, WindowContext::kContextWidth, WindowContext::kContextHeight,
		GL_RGB, GL_UNSIGNED_BYTE, screenshot);
	if (glGetError() != GL_NO_ERROR) {
		goto save_png_done;
	}

	for (int i = 0; i < WindowContext::kContextHeight; i++) {
		row_pointers[i] = screenshot + ((WindowContext::kContextWidth * ((WindowContext::kContextHeight - 1) - i)) * 3);
	}

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		goto save_png_done;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		goto save_png_done;
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		goto save_png_done;
	}

	png_init_io(png_ptr, fp);

	if (setjmp(png_jmpbuf(png_ptr))) {
		goto save_png_done;
	}

	png_set_IHDR(png_ptr, info_ptr, WindowContext::kContextWidth, WindowContext::kContextHeight,
		8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	png_write_info(png_ptr, info_ptr);

	if (setjmp(png_jmpbuf(png_ptr))) {
		goto save_png_done;
	}

	png_write_image(png_ptr, row_pointers);

	if (setjmp(png_jmpbuf(png_ptr))) {
		goto save_png_done;
	}

	png_write_end(png_ptr, NULL);
	retval = true;

save_png_done:
	png_destroy_write_struct(&png_ptr, &info_ptr);
	delete[] screenshot;
	delete[] row_pointers;
	if (fp) {
		fclose(fp);
	}
	if (!retval) {
#ifdef LUGARU_PLATFORM_WINDOWS
		_unlink(file_name);
#else
		unlink(file_name);
#endif
	}
	return retval;
}
