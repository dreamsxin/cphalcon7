
#include "chart/qrcode/qr.h"

static int qr_dpi = 72;

static void fillRow(unsigned char *row, int num, const unsigned int color[4])
{
	int i;
	for (i = 0; i < num; i++) {
#ifdef WORDS_BIGENDIAN
		row[0] = (unsigned char)color[0] >> 8 * 1;
		row[1] = (unsigned char)color[1] >> 8 * 1;
		row[2] = (unsigned char)color[2] >> 8 * 1;
		row[3] = (unsigned char)color[3] >> 8 * 1;
#else
		row[0] = (unsigned char)color[0];
		row[1] = (unsigned char)color[1];
		row[2] = (unsigned char)color[2];
		row[3] = (unsigned char)color[3];
#endif
		row += 4;
	}
}

int qr_write(FILE *fp, const QRcode *qrcode, int size, int margin, unsigned int fg_color[4], unsigned int bg_color[4], enum QRImageType type)
{
	png_structp png_ptr;
	png_infop info_ptr;
	png_colorp palette = NULL;
	png_byte alpha_values[2];
	unsigned char *row, *p, *q;
	int x, y, xx, yy, bit;
	int realwidth;

	realwidth = (qrcode->width + margin * 2) * size;
	if (type == PNG_TYPE) {
		row = (unsigned char *)malloc((realwidth + 7) / 8);
	} else {
		row = (unsigned char *)malloc(realwidth * 4);
		qr_dpi = 96;
	}
	if (row == NULL) {
		fprintf(stderr, "Failed to allocate memory.\n");
		return FAILURE;
	}

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		fprintf(stderr, "Failed to initialize PNG writer.\n");
		return FAILURE;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		fprintf(stderr, "Failed to initialize PNG write.\n");
		return FAILURE;
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fprintf(stderr, "Failed to write PNG image.\n");
		return FAILURE;
	}

	if (type == PNG_TYPE) {
		palette = (png_colorp)malloc(sizeof(png_color) * 2);
		if (palette == NULL) {
			fprintf(stderr, "Failed to allocate memory.\n");
			return FAILURE;
		}
		palette[0].red = fg_color[0];
		palette[0].green = fg_color[1];
		palette[0].blue = fg_color[2];
		palette[1].red = bg_color[0];
		palette[1].green = bg_color[1];
		palette[1].blue = bg_color[2];
		alpha_values[0] = fg_color[3];
		alpha_values[1] = bg_color[3];
		png_set_PLTE(png_ptr, info_ptr, palette, 2);
		png_set_tRNS(png_ptr, info_ptr, alpha_values, 2, NULL);
	}

	png_init_io(png_ptr, fp);
	if (type == PNG_TYPE) {
		png_set_IHDR(png_ptr, info_ptr,
			realwidth, realwidth,
			1,
			PNG_COLOR_TYPE_PALETTE,
			PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_DEFAULT,
			PNG_FILTER_TYPE_DEFAULT);
	}
	else {
		png_set_IHDR(png_ptr, info_ptr,
			realwidth, realwidth,
			8,
			PNG_COLOR_TYPE_RGB_ALPHA,
			PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_DEFAULT,
			PNG_FILTER_TYPE_DEFAULT);
	}
	png_set_pHYs(png_ptr, info_ptr,
		qr_dpi * INCHES_PER_METER,
		qr_dpi * INCHES_PER_METER,
		PNG_RESOLUTION_METER);
	png_write_info(png_ptr, info_ptr);

	if (type == PNG_TYPE) {
		/* top margin */
		memset(row, 0xff, (realwidth + 7) / 8);
		for (y = 0; y < margin * size; y++) {
			png_write_row(png_ptr, row);
		}

		/* data */
		p = qrcode->data;
		for (y = 0; y < qrcode->width; y++) {
			memset(row, 0xff, (realwidth + 7) / 8);
			q = row;
			q += margin * size / 8;
			bit = 7 - (margin * size % 8);
			for (x = 0; x < qrcode->width; x++) {
				for (xx = 0; xx < size; xx++) {
					*q ^= (*p & 1) << bit;
					bit--;
					if (bit < 0) {
						q++;
						bit = 7;
					}
				}
				p++;
			}
			for (yy = 0; yy < size; yy++) {
				png_write_row(png_ptr, row);
			}
		}
		/* bottom margin */
		memset(row, 0xff, (realwidth + 7) / 8);
		for (y = 0; y < margin * size; y++) {
			png_write_row(png_ptr, row);
		}
	}
	else {
		/* top margin */
		fillRow(row, realwidth, bg_color);
		for (y = 0; y < margin * size; y++) {
			png_write_row(png_ptr, row);
		}

		/* data */
		p = qrcode->data;
		for (y = 0; y < qrcode->width; y++) {
			fillRow(row, realwidth, bg_color);
			for (x = 0; x < qrcode->width; x++) {
				for (xx = 0; xx < size; xx++) {
					if (*p & 1) {
						fillRow(&row[((margin + x) * size + xx) * 4], 1, fg_color);
					}
				}
				p++;
			}
			for (yy = 0; yy < size; yy++) {
				png_write_row(png_ptr, row);
			}
		}
		/* bottom margin */
		fillRow(row, realwidth, bg_color);
		for (y = 0; y < margin * size; y++) {
			png_write_row(png_ptr, row);
		}
	}

	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);

	free(row);
	free(palette);

	return SUCCESS;
}
