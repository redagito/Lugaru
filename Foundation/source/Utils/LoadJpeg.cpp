#include "Utils/LoadJpeg.h"

#include <cstdio>
#include <string>

#include <jpeglib.h>
#include <jerror.h>
#include <setjmp.h>

#include "Utils/ImageIO.hpp"
#include "Utils/Log.hpp"

struct my_error_mgr
{
	struct jpeg_error_mgr pub; /* "public" fields */
	jmp_buf setjmp_buffer;     /* for return to caller */
};
typedef struct my_error_mgr* my_error_ptr;

static void my_error_exit(j_common_ptr cinfo)
{
	struct my_error_mgr* err = (struct my_error_mgr*)cinfo->err;
	longjmp(err->setjmp_buffer, 1);
}

/* stolen from public domain example.c code in libjpg distribution. */
bool load_jpg(const char* file_name, ImageRec& tex)
{
	struct jpeg_decompress_struct cinfo;
	struct my_error_mgr jerr;
	JSAMPROW buffer[1]; /* Output row buffer */
	int row_stride;     /* physical row width in output buffer */
	errno = 0;
	FILE* infile = fopen(file_name, "rb");

	if (infile == NULL) {
		Log::error(std::string("Couldn't open file ") + file_name);
		return false;
	}

	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	if (setjmp(jerr.setjmp_buffer)) {
		jpeg_destroy_decompress(&cinfo);
		fclose(infile);
		return false;
	}

	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, infile);
	(void)jpeg_read_header(&cinfo, TRUE);

	cinfo.out_color_space = JCS_RGB;
	cinfo.quantize_colors = FALSE;
	(void)jpeg_calc_output_dimensions(&cinfo);
	(void)jpeg_start_decompress(&cinfo);

	row_stride = cinfo.output_width * cinfo.output_components;
	tex.sizeX = cinfo.output_width;
	tex.sizeY = cinfo.output_height;
	tex.bpp = 24;

	// Size the destination from the real image dimensions. The buffer used to be
	// a fixed 4 MiB, so any image larger than 1024x1024 overflowed it.
	if (!tex.ensureCapacity((size_t)cinfo.output_width * (size_t)cinfo.output_height * row_stride)) {
		jpeg_destroy_decompress(&cinfo);
		fclose(infile);
		return false;
	}

	while (cinfo.output_scanline < cinfo.output_height) {
		buffer[0] = (JSAMPROW)(char*)tex.data +
			((cinfo.output_height - 1) - cinfo.output_scanline) * row_stride;
		(void)jpeg_read_scanlines(&cinfo, buffer, 1);
	}

	(void)jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	fclose(infile);

	return true;
}
