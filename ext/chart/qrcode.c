/*
  +------------------------------------------------------------------------+
  | Phalcon Framework                                                      |
  +------------------------------------------------------------------------+
  | Copyright (c) 2011-2013 Phalcon Team (http://www.phalconphp.com)       |
  +------------------------------------------------------------------------+
  | This source file is subject to the New BSD License that is bundled     |
  | with this package in the file docs/LICENSE.txt.                        |
  |                                                                        |
  | If you did not receive a copy of the license and are unable to         |
  | obtain it through the world-wide-web, please send an email             |
  | to license@phalconphp.com so we can send you a copy immediately.       |
  +------------------------------------------------------------------------+
  | Authors: Andres Gutierrez <andres@phalconphp.com>                      |
  |          Eduar Carvajal <eduar@phalconphp.com>                         |
  |          ZhuZongXin <dreamsxin@qq.com>                                 |
  +------------------------------------------------------------------------+
*/

#include "chart/qrcode.h"
#include "chart/exception.h"

#ifdef PHALCON_USE_ZBAR
# include <wand/MagickWand.h>
# include <zbar.h>
#endif

#include "kernel/main.h"
#include "kernel/memory.h"

#include "kernel/exception.h"
#include "kernel/object.h"
#include "kernel/fcall.h"
#include "kernel/array.h"
#include "kernel/hash.h"
#include "kernel/concat.h"
#include "kernel/operators.h"
#include "kernel/string.h"
#include "kernel/output.h"

#include <main/php_open_temporary_file.h>

/**
 * Phalcon\Chart\QRcode
 *
 *<code>
 * $qr = new \Phalcon\Chart\QRcode();
 * $ret = $qr->generate('Phalcon framework');
 * $data = $qr->render();
 * $data = $qr->render(NULL, NULL, 'FFCC00', '000000');
 * $ret = $qr->save('unit-tests/assets/qr.png');
 * $ret = $qr->save('unit-tests/assets/qr.png', NULL, NULL, 'FFCC00', '000000');
 * $ret = $qr->scan('unit-tests/assets/qr.png');
 *</code>
 */
zend_class_entry *phalcon_chart_qrcode_ce;

PHP_METHOD(Phalcon_Chart_QRcode, __construct);
PHP_METHOD(Phalcon_Chart_QRcode, generate);
PHP_METHOD(Phalcon_Chart_QRcode, getData);
PHP_METHOD(Phalcon_Chart_QRcode, render);
PHP_METHOD(Phalcon_Chart_QRcode, save);

#if PHALCON_USE_ZBAR
PHP_METHOD(Phalcon_Chart_QRcode, scan);
#endif

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_chart_qrcode_generate, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, version, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, level, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, mode, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, casesensitive, _IS_BOOL, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_chart_qrcode_scan, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_chart_qrcode_render, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, size, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, margin, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, foreground, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, background, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, type, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_chart_qrcode_save, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, size, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, margin, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, foreground, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, background, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, type, IS_LONG, 1)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_chart_qrcode_method_entry[] = {
	PHP_ME(Phalcon_Chart_QRcode, __construct, arginfo_empty, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Chart_QRcode, generate, arginfo_phalcon_chart_qrcode_generate, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Chart_QRcode, getData, arginfo_empty, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Chart_QRcode, render, arginfo_phalcon_chart_qrcode_render, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Chart_QRcode, save, arginfo_phalcon_chart_qrcode_save, ZEND_ACC_PUBLIC)

#if PHALCON_USE_ZBAR
	PHP_ME(Phalcon_Chart_QRcode, scan, arginfo_phalcon_chart_qrcode_scan, ZEND_ACC_PUBLIC)
#endif
	PHP_FE_END
};

int  phalcon_qrcode_handle;

static void phalcon_qr_dtor(zend_resource *rsrc)
{
    phalcon_qrcode *qr = (phalcon_qrcode *) rsrc->ptr;

    if (qr->c)
        QRcode_free (qr->c);
    efree (qr);
}

/**
 * Phalcon\Chart\QRcode initializer
 */
PHALCON_INIT_CLASS(Phalcon_Chart_QRcode){

	PHALCON_REGISTER_CLASS(Phalcon\\Chart, QRcode, chart_qrcode, phalcon_chart_qrcode_method_entry, 0);

    phalcon_qrcode_handle = zend_register_list_destructors_ex(phalcon_qr_dtor, NULL, phalcon_qrcode_handle_name, module_number);

	/* Mode */
	zend_declare_class_constant_long(phalcon_chart_qrcode_ce, SL("MODE_NUL"), QR_MODE_NUL);
	zend_declare_class_constant_long(phalcon_chart_qrcode_ce, SL("MODE_NUM"), QR_MODE_NUM);
	zend_declare_class_constant_long(phalcon_chart_qrcode_ce, SL("MODE_AN"), QR_MODE_AN);
	zend_declare_class_constant_long(phalcon_chart_qrcode_ce, SL("MODE_8"), QR_MODE_8);
	zend_declare_class_constant_long(phalcon_chart_qrcode_ce, SL("MODE_KANJI"), QR_MODE_KANJI);

	/* Level */
	zend_declare_class_constant_long(phalcon_chart_qrcode_ce, SL("LEVEL_L"), QR_ECLEVEL_L);
	zend_declare_class_constant_long(phalcon_chart_qrcode_ce, SL("LEVEL_M"), QR_ECLEVEL_M);
	zend_declare_class_constant_long(phalcon_chart_qrcode_ce, SL("LEVEL_Q"), QR_ECLEVEL_Q);
	zend_declare_class_constant_long(phalcon_chart_qrcode_ce, SL("LEVEL_H"), QR_ECLEVEL_H);

	zend_declare_property_null(phalcon_chart_qrcode_ce, SL("_qr"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_chart_qrcode_ce, SL("_text"), ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_chart_qrcode_ce, SL("_version"), 6, ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_chart_qrcode_ce, SL("_maxVersion"), MQRSPEC_VERSION_MAX, ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_chart_qrcode_ce, SL("_level"), QR_ECLEVEL_H, ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_chart_qrcode_ce, SL("_mode"), QR_MODE_8, ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_chart_qrcode_ce, SL("_casesensitive"), 1, ZEND_ACC_PROTECTED);

	return SUCCESS;
}

/**
 * Phalcon\Chart\QRcode constructor
 *
 *     $qr = new \Phalcon\Chart\QRcode;
 *     $qr->generate('Phalcon is a web framework', 4, \Phalcon\Chart\QRcode::LEVEL_L, \Phalcon\Chart\QRcode::MODE_KANJI, TRUE);
 *     $qr->save('qr.png');
 */
PHP_METHOD(Phalcon_Chart_QRcode, __construct){

}

/**
 * Generate QR data
 *
 * @param string $text
 * @param int $version
 * @param int $level
 * @param int $mode
 * @param boolean $casesensitive
 * @return boolean
 */
PHP_METHOD(Phalcon_Chart_QRcode, generate){

	zval *text, *_version = NULL, *_level = NULL, *_mode = NULL, *_casesensitive = NULL, version = {}, level = {}, mode = {}, casesensitive = {};
	zval zid = {};
	phalcon_qrcode *qr = NULL;

	phalcon_fetch_params(0, 1, 4, &text, &_version, &level, &_mode, &_casesensitive);

	phalcon_update_property(getThis(), SL("_text"), text);

	if (!_version || Z_TYPE_P(_version) == IS_NULL) {
		phalcon_read_property(&version, getThis(), SL("_version"), PH_NOISY|PH_READONLY);
	} else {
		if (Z_LVAL_P(_version) < 1 || Z_LVAL_P(_version) > MQRSPEC_VERSION_MAX) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_chart_exception_ce, "version must be within the range of 1 to 40");
			return;
		}
		ZVAL_COPY_VALUE(&version, _version);
	}

	if (!_level || Z_TYPE_P(_level) == IS_NULL) {
		phalcon_read_property(&level, getThis(), SL("_level"), PH_NOISY|PH_READONLY);
	} else {
		if (Z_LVAL_P(_level) != QR_ECLEVEL_L && Z_LVAL_P(_level) != QR_ECLEVEL_M && Z_LVAL_P(_level) != QR_ECLEVEL_Q && Z_LVAL_P(_level) != QR_ECLEVEL_H) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_chart_exception_ce, "Error level. there are 4 values: LEVEL_L, LEVEL_M, LEVEL_Q, LEVEL_H");
			return;
		}
		ZVAL_COPY_VALUE(&level, _level);
	}


	if (!_mode || Z_TYPE_P(_mode) == IS_NULL) {
		phalcon_read_property(&mode, getThis(), SL("_mode"), PH_NOISY|PH_READONLY);
	} else {
		if (Z_LVAL_P(_mode) != QR_MODE_NUL && Z_LVAL_P(_mode) != QR_MODE_NUM && Z_LVAL_P(_mode) != QR_MODE_8 && Z_LVAL_P(_mode) != QR_MODE_KANJI) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_chart_exception_ce, "Error mode. there are 4 values: MODE_NUL, MODE_NUM, MODE_8, MODE_KANJI");
			return;
		}
		ZVAL_COPY_VALUE(&mode, _mode);
	}

	if (!_casesensitive || Z_TYPE_P(_casesensitive) == IS_NULL) {
		phalcon_read_property(&casesensitive, getThis(), SL("_casesensitive"), PH_NOISY|PH_READONLY);
	} else {
		ZVAL_COPY_VALUE(&casesensitive, _casesensitive);
	}

	qr = (phalcon_qrcode *) emalloc (sizeof (phalcon_qrcode));

	if (Z_LVAL(mode) == QR_MODE_8) {
		qr->c = QRcode_encodeString8bit(Z_STRVAL_P(text), Z_LVAL(version), Z_LVAL(level));
	} else {
		qr->c = QRcode_encodeString(Z_STRVAL_P(text), Z_LVAL(version), Z_LVAL(level), Z_LVAL(mode), zend_is_true(&casesensitive) ? 1 : 0);
	}

	if (qr->c == NULL)  {
		efree(qr);
		RETURN_FALSE;
	} else {
		ZVAL_RES(&zid, zend_register_resource(qr, phalcon_qrcode_handle));
		phalcon_update_property(getThis(), SL("_qr"), &zid);
		RETURN_TRUE;
	}
}

/**
 * Return the qrcode binary string.
 *
 * @return string
 */
PHP_METHOD(Phalcon_Chart_QRcode, getData){

	zval zid = {};
	phalcon_qrcode *qr = NULL;

	phalcon_read_property(&zid, getThis(), SL("_qr"), PH_NOISY|PH_READONLY);

	if (Z_TYPE(zid) == IS_NULL) {
		RETURN_FALSE;
	}

	if ((qr = (phalcon_qrcode *)zend_fetch_resource(Z_RES(zid), phalcon_qrcode_handle_name, phalcon_qrcode_handle)) == NULL) {
		RETURN_FALSE;
	}
	
	RETURN_STRINGL((const char *)qr->c->data, (qr->c->width*qr->c->width));
}

/**
 * Render the image and return the binary string.
 *
 *     $qr = new \Phalcon\Chart\QRcode;
 *     $qr->generate('Phalcon is a web framework');
 *     $data = \Phalcon\Chart\QRcode::render();
 *
 * @param int $size
 * @param int $margin.
 * @param string $foreground
 * @param string $background
 * @param int $type
 * @return string
 */
PHP_METHOD(Phalcon_Chart_QRcode, render){

	zval *size = NULL, *margin = NULL, *foreground=NULL, *background=NULL, *type = NULL;

	phalcon_fetch_params(0, 0, 5, &size, &margin, &foreground, &background, &type);

	if (!size) {
		size = &PHALCON_GLOBAL(z_null);
	}

	if (!margin) {
		margin = &PHALCON_GLOBAL(z_null);
	}

	if (!foreground) {
		foreground = &PHALCON_GLOBAL(z_null);
	}

	if (!background) {
		background = &PHALCON_GLOBAL(z_null);
	}

	if (!type) {
		type = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_CALL_METHOD(return_value, getThis(), "save", &PHALCON_GLOBAL(z_null), size, margin, foreground, background, type);
}

static int phalcon_color_set(unsigned int color[4], const char *value)
{
        int len = strlen(value);
        int count;
        if(len == 6) {
                count = sscanf(value, "%02x%02x%02x%n", &color[0], &color[1], &color[2], &len);
                if(count < 3 || len != 6) {
                        return -1;
                }
                color[3] = 255;
        } else if(len == 8) {
                count = sscanf(value, "%02x%02x%02x%02x%n", &color[0], &color[1], &color[2], &color[3], &len);
                if(count < 4 || len != 8) {
                        return -1;
                }
        } else {
                return -1;
        }
        return 0;
}

/**
 * Save the image
 *
 *     $qr = new \Phalcon\Chart\QRcode;
 *     $qr->generate('Phalcon is a web framework', 4, \Phalcon\Chart\QRcode::LEVEL_L, \Phalcon\Chart\QRcode::MODE_KANJI, TRUE);
 *     $qr->save('qr.png');
 *
 * @param filename $filename
 * @param size $size
 * @param margin $margin.
 * @return boolean
 */
PHP_METHOD(Phalcon_Chart_QRcode, save){

	zval *filename, *size = NULL, *margin = NULL, *foreground=NULL, *background=NULL, *type=NULL;
	zval zid = {}, exception_message = {};
	phalcon_qrcode *qr = NULL;
	zend_string *path;
	FILE *fp = NULL;
	static unsigned int fg_color[4] = {0, 0, 0, 255};
	static unsigned int bg_color[4] = {255, 255, 255, 255};
	enum QRImageType qtype = PNG_TYPE;
	long s = 3, m = 4;

	phalcon_fetch_params(0, 0, 6, &filename, &size, &margin, &foreground, &background, &type);

	if (!filename) {
		filename = &PHALCON_GLOBAL(z_null);
	}

	if (foreground && zend_is_true(foreground)) {
		PHALCON_SEPARATE_PARAM(foreground);
		convert_to_string(foreground);

		if(phalcon_color_set(fg_color, Z_STRVAL_P(foreground))) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_chart_exception_ce, "Invalid foreground color value");
			return;
		}
	}

	if (background && zend_is_true(background)) {
		PHALCON_SEPARATE_PARAM(background);
		convert_to_string(background);

		if(phalcon_color_set(bg_color, Z_STRVAL_P(background))) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_chart_exception_ce, "Invalid background color value");
			return;
		}
	}

	if (size && Z_TYPE_P(size) == IS_LONG) {
		s = Z_LVAL_P(size);
	}
	if (margin && Z_TYPE_P(margin) == IS_LONG) {
		m = Z_LVAL_P(margin);
	}
	if (s <= 0 || s > 10) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_chart_exception_ce, "size must be within the range of 1 to 10");
		return;
	}
	if (m < 0 || m > 10) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_chart_exception_ce, "margin must be within the range of 0 to 10");
		return;
	}

	if (type && Z_TYPE_P(type) == IS_LONG) {
		qtype = Z_LVAL_P(type);
	}

	phalcon_read_property(&zid, getThis(), SL("_qr"), PH_NOISY|PH_READONLY);

	if (Z_TYPE(zid) == IS_NULL) {
		RETURN_FALSE;
	}

	if ((qr = (phalcon_qrcode *)zend_fetch_resource(Z_RES(zid), phalcon_qrcode_handle_name, phalcon_qrcode_handle)) == NULL) {
		RETURN_FALSE;
	}

	if (Z_TYPE_P(filename) != IS_STRING) {
		fp = php_open_temporary_file(NULL, NULL, &path);
		if (!fp) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_chart_exception_ce, "Unable to open temporary file for writing");
			return;
		}
	} else {
		fp = VCWD_FOPEN(Z_STRVAL_P(filename), "wb");
		if (!fp) {
			PHALCON_CONCAT_SVS(&exception_message, "Unable to open '", filename, "' for writing");
			PHALCON_THROW_EXCEPTION_ZVAL(phalcon_chart_exception_ce, &exception_message);
			return;
		}
	}

	if (qr_write(fp, qr->c, s, m, fg_color, bg_color, qtype) == FAILURE) {
		if (Z_TYPE_P(filename) != IS_STRING) {
			VCWD_UNLINK((const char *)ZSTR_VAL(path));
			zend_string_release(path);
		}
		fclose (fp);
		RETURN_FALSE;
	}

	fflush(fp);

	if (Z_TYPE_P(filename) != IS_STRING) {
		int len;
		char buf[4096];
		fseek (fp, 0, SEEK_SET);

		phalcon_ob_start();

		while ((len = fread(buf, 1, sizeof(buf), fp)) > 0) {
			php_write(buf, len);
		}

		phalcon_ob_get_contents(return_value);
		phalcon_ob_end_clean();

		fclose (fp);
		VCWD_UNLINK((const char *)ZSTR_VAL(path));
	} else {
		fclose (fp);
		RETURN_TRUE;
	}
}

#ifdef PHALCON_USE_ZBAR
static zbar_image_t *_php_zbarcode_image_create(unsigned long width, unsigned long height, unsigned char *image_data)
{
	zbar_image_t *image = zbar_image_create();

	if (!image)
		return NULL;

	zbar_image_set_format(image, *(int*)"Y800");
	zbar_image_set_size(image, width, height);
	zbar_image_set_data(image, (void *)image_data, width * height, zbar_image_free_data);
	return image;
}

static zbar_image_t *_php_zbarcode_get_page(MagickWand *wand)
{
	unsigned long width, height;
	unsigned char *image_data;

	if (MagickSetImageDepth(wand, 8) == MagickFalse) {
		return NULL;
	}

	if (MagickSetImageFormat(wand, "GRAY") == MagickFalse) {
		return NULL;
	}

	width  = MagickGetImageWidth(wand);
	height = MagickGetImageHeight(wand);

	image_data = emalloc(width * height);

	if (!MagickExportImagePixels(wand, 0, 0, width, height, "I", CharPixel, image_data)) {
		return NULL;
	}

	return _php_zbarcode_image_create(width, height, image_data);
}

static void _php_zbarcode_scan_page(zbar_image_scanner_t *scanner, zbar_image_t *image, zend_bool extended, zval *return_array)
{
	const zbar_symbol_t *symbol;

	array_init(return_array);

	/* scan the image for barcodes */
	zbar_scan_image(scanner, image);

	/* extract results */
	symbol = zbar_image_first_symbol(image);

	/* Loop through all all symbols */
	for(; symbol; symbol = zbar_symbol_next(symbol)) {
		zval fromtext = {}, totext = {}, from = {}, to = {}, symbol_array = {}, loc_array = {}, coords = {};
		zbar_symbol_type_t symbol_type;
		const char *data;
		const char *type;
		int quality;
		unsigned int loc_size, i;

		array_init(&symbol_array);

		/* Get symbol type and data in it */
		symbol_type = zbar_symbol_get_type(symbol);
		data = zbar_symbol_get_data(symbol);
		type = zbar_get_symbol_name(symbol_type);
		quality = zbar_symbol_get_quality(symbol);

#ifdef PHALCON_USE_PHP_MBSTRING
		ZVAL_STRING(&fromtext, data);
		ZVAL_STRING(&to, "shift-jis");
		ZVAL_STRING(&from, "utf-8");
		ZVAL_STRING(&fromtext, data);

		phalcon_convert_encoding(&totext, &fromtext, &to, &from);
		phalcon_array_update_str(&symbol_array, SL("data"), &totext, PH_COPY);
#else
        if (phalcon_function_exists_ex(SS("mb_convert_encoding")) == SUCCESS) {
			ZVAL_STRING(&fromtext, data);
			ZVAL_STRING(&to, "shift-jis");
			ZVAL_STRING(&from, "utf-8");
			ZVAL_STRING(&fromtext, data);

			PHALCON_CALL_FUNCTION(&totext, "mb_convert_encoding", &fromtext, &to, &from);
			phalcon_array_update_str(&symbol_array, SL("data"), &totext, PH_COPY);
        } else {
			phalcon_array_update_str_str(&symbol_array, SL("data"), (char *)data, strlen(data), PH_COPY);
		}
#endif
		phalcon_array_update_str_str(&symbol_array, SL("type"), (char *)type, strlen(type), PH_COPY);
		phalcon_array_update_str_long(&symbol_array, SL("quality"), quality, 0);

		if (extended) {
			array_init(&loc_array);
			loc_size = zbar_symbol_get_loc_size(symbol);

			for (i = 0; i < loc_size; i++) {
				array_init(&coords);
				phalcon_array_update_str_long(&coords, SL("x"), zbar_symbol_get_loc_x(symbol, i), PH_COPY);
				phalcon_array_update_str_long(&coords, SL("y"), zbar_symbol_get_loc_y(symbol, i), PH_COPY);

				phalcon_array_append(&loc_array, &coords, PH_COPY);
			}
			phalcon_array_update_str(&symbol_array, SL("location"), &loc_array, PH_COPY);
		}
		phalcon_array_append(return_array, &symbol_array, PH_COPY);
	}
}

/**
 * Scan the image.
 *
 *     $qr = new \Phalcon\Chart\QRcode;
 *     $ret = $qr->san('qr.png');
 *
 * @param string filename
 * @return string
 */
PHP_METHOD(Phalcon_Chart_QRcode, scan){

	zval *filename, *enhance = NULL, *extended = NULL;
	MagickWand *magick_wand;
	zbar_image_scanner_t *zbar_scanner;
	zbar_image_t *zbar_page;
	zend_bool ext = 0;
	long i = 1, e = 0, image_count = 0;

	phalcon_fetch_params(0, 1, 3, &filename, &enhance, &extended);

	if (enhance && Z_TYPE_P(enhance) == IS_LONG) {
		e = Z_LVAL_P(enhance);
	}

	if (extended) {
		ext = zend_is_true(extended) ? 1 : 0;
	}

	magick_wand = NewMagickWand();

	if (e & 1) {
		MagickSetResolution(magick_wand, 200, 200);
	}

	if (MagickReadImage(magick_wand, Z_STRVAL_P(filename)) == MagickFalse) {
		ClearMagickWand(magick_wand);
		DestroyMagickWand(magick_wand);
		RETURN_FALSE;
	}

	if (e & 2) {
		MagickEnhanceImage(magick_wand);
	}

	if (e & 4) {
		MagickSharpenImage(magick_wand, 0, 0.5);
	}

	image_count = MagickGetNumberImages(magick_wand);

	if (image_count == 0) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_chart_exception_ce, "The image object does not contain images");
		return;
	}

	zbar_scanner = zbar_image_scanner_create();

	if (image_count == 1) {
		if (MagickSetIteratorIndex(magick_wand, 0) == MagickFalse) {
			zbar_image_scanner_destroy(zbar_scanner);
			DestroyMagickWand(magick_wand);
			PHALCON_THROW_EXCEPTION_STR(phalcon_chart_exception_ce, "Failed to set the page number");
			return;
		}

		/* Read page */
		zbar_page = _php_zbarcode_get_page(magick_wand);

		if (!zbar_page) {
			zbar_image_scanner_destroy(zbar_scanner);
			DestroyMagickWand(magick_wand);
			PHALCON_THROW_EXCEPTION_STR(phalcon_chart_exception_ce, "Failed to get the page");
			return;
		}

		/* Scan the page for barcodes */
		_php_zbarcode_scan_page(zbar_scanner, zbar_page, ext, return_value);
	} else {
		array_init(return_value);

		MagickResetIterator(magick_wand);
		while (MagickNextImage(magick_wand) != MagickFalse) {
			zval page_array = {};

			/* Read the current page */
			zbar_page = _php_zbarcode_get_page(magick_wand);

			/* Reading current page failed */
			if (!zbar_page) {
				i++;
				continue;
			}

			/* Scan the page for barcodes */
			ZVAL_NULL(&page_array);
			_php_zbarcode_scan_page(zbar_scanner, zbar_page, ext, &page_array);
			add_index_zval(return_value, i++, &page_array);
		}
	}

	zbar_image_scanner_destroy(zbar_scanner);
	DestroyMagickWand(magick_wand);
}
#endif
