
/*
  +------------------------------------------------------------------------+
  | Phalcon Framework                                                      |
  +------------------------------------------------------------------------+
  | Copyright (c) 2011-2014 Phalcon Team (http://www.phalconphp.com)       |
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

#include "image/vips.h"
#include "image/exception.h"

#include "kernel/main.h"
#include "kernel/exception.h"
#include "kernel/operators.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"
#include "kernel/object.h"
#include "kernel/array.h"
#include "kernel/string.h"

#include "internal/arginfo.h"

#include <main/SAPI.h>

//#define VIPS_DEBUG 1

#include <vips/vips.h>
#include <vips/debug.h>
#include <vips/vector.h>

#define ADD_ELEMENTS(TYPE, APPEND, N) { \
	TYPE *p = (TYPE *) arr; \
	size_t i; \
	\
	for (i = 0; i < (N); i++) \
		APPEND(return_value, p[i]); \
}

#define phalcon_image_vips_type_name "GObject"

#define PHALCON_THROW_VIPS_EXCEPTION(format) \
	do { \
		if (strlen(vips_error_buffer()) > 0) { \
			PHALCON_THROW_EXCEPTION_FORMAT(phalcon_image_exception_ce, format, vips_error_buffer()); \
			vips_error_clear(); \
		} \
		RETURN_LONG(-1); \
	} while (0)

#define PHALCON_MM_THROW_VIPS_EXCEPTION(format) \
	do { \
		if (strlen(vips_error_buffer()) > 0) { \
			PHALCON_THROW_EXCEPTION_FORMAT(phalcon_image_exception_ce, format, vips_error_buffer()); \
			vips_error_clear(); \
		} \
		RETURN_MM_LONG(-1); \
	} while (0)

/* True global resources - no need for thread safety here */
static int le_vips_gobject;

static void phalcon_vips_free_gobject(zend_resource *rsrc)
{
	VIPS_DEBUG_MSG("php_free_gobject: %p\n", rsrc->ptr);

	g_object_unref((GObject *) rsrc->ptr);
}

/* Track stuff during a call to a vips operation in one of these.
 */
typedef struct _VipsPhpCall {
	/* Parameters.
	 */
	const char *operation_name;
	zval *instance;
	const char *option_string;
	int argc;
	zval *argv;

	/* The operation we are calling.
	 */
	VipsOperation *operation;

	/* The num of args this operation needs from php. This does not include the
	 * @instance zval.
	 */
	int args_required;

	/* If we've already used the instance zval.
	 */
	gboolean used_instance;

	/* Extra php array of optional args.
	 */
	zval *options;

	/* The first image arg ... the thing we expand constants to match.
	 */
	VipsImage *match_image;

} VipsPhpCall;

static void
vips_php_call_free(VipsPhpCall *call)
{
	VIPS_DEBUG_MSG("vips_php_call_free:\n");

	VIPS_UNREF(call->operation);
	g_free(call);
}

static VipsPhpCall *
vips_php_call_new(const char *operation_name, zval *instance, 
	const char *option_string, int argc, zval *argv)
{
	VipsPhpCall *call;

	VIPS_DEBUG_MSG("vips_php_call_new: %s\n", operation_name );
	VIPS_DEBUG_MSG("    option_string = \"%s\", argc = %d\n", 
			option_string, argc);

	call = g_new0( VipsPhpCall, 1 );
	call->operation_name = operation_name;
	call->instance = instance;
	call->option_string = option_string;
	call->argc = argc;
	call->argv = argv;

	if (!(call->operation = vips_operation_new(operation_name))) {
		vips_php_call_free(call);
		return NULL;
	}

	return call;
}

/* Get a non-reference zval. In php7, zvalues can be references to other zvals
 * ... chase down a chain of refs to get a real zval.
 * the ref pointer chain.
 */
static inline zval *
zval_get_nonref(zval *zvalue)
{
	while (Z_TYPE_P(zvalue) == IS_REFERENCE)
		zvalue = Z_REFVAL_P(zvalue);

	return zvalue;
}

/* First pass over our arguments: find the first image arg and note as
 * match_image.
 */
static void
vips_php_analyze_arg(VipsPhpCall *call, zval *zvalue)
{
	zvalue = zval_get_nonref(zvalue); 

	if (Z_TYPE_P(zvalue) == IS_ARRAY) {
		const int n = zend_hash_num_elements(Z_ARRVAL_P(zvalue));

		int i;

		for (i = 0; i < n; i++) { 
			zval *item = zend_hash_index_find(Z_ARRVAL_P(zvalue), i);

			if (item) {
				vips_php_analyze_arg(call, item);
			}
		}
	}
	else if (Z_TYPE_P(zvalue) == IS_RESOURCE) {
		VipsImage *image;

		if( (image = (VipsImage *)zend_fetch_resource(Z_RES_P(zvalue), 
			phalcon_image_vips_type_name, le_vips_gobject)) != NULL) {
			if (!call->match_image) {
				call->match_image = image;
			}
		}
	}
}

static int
vips_php_blob_free(void *buf, void *area)
{
	g_free(buf);

	return 0;
}

/* Expand a constant (eg. 12, "12" or [1, 2, 3]) into an image using 
 * @match_image as a guide.
 */
static VipsImage *
expand_constant(VipsImage *match_image, zval *constant)
{
	VipsImage *result;
	VipsImage *x;

	if (vips_black(&result, 1, 1, NULL)) {
		return NULL;
	}

	constant = zval_get_nonref(constant); 

	if (Z_TYPE_P(constant) == IS_ARRAY) {
		const int n = zend_hash_num_elements(Z_ARRVAL_P(constant));

		double *ones;
		double *offsets;
		int i;

		ones = VIPS_ARRAY(result, n, double);
		offsets = VIPS_ARRAY(result, n, double);

		for (i = 0; i < n; i++) {
			zval *ele;

			ones[i] = 1.0;

			if ((ele = zend_hash_index_find(Z_ARRVAL_P(constant), i)) != NULL) {
				offsets[i] = zval_get_double(ele);
			}
		}

		if (vips_linear(result, &x, ones, offsets, n, NULL)) {
			return NULL;
		}
		g_object_unref(result);
		result = x;
	}
	else {
		if (vips_linear1(result, &x, 1.0, zval_get_double(constant), NULL)) {
			return NULL;
		}
		g_object_unref(result);
		result = x;
	}

	if (vips_cast(result, &x, match_image->BandFmt, NULL)) {
		return NULL;
	}
	g_object_unref(result);
	result = x;

	if (vips_embed(result, &x, 0, 0, match_image->Xsize, match_image->Ysize, 
		"extend", VIPS_EXTEND_COPY,
		NULL)) {
		return NULL;
	}
	g_object_unref(result);
	result = x;

	result->Type = match_image->Type;
	result->Xres = match_image->Xres;
	result->Yres = match_image->Yres;
	result->Xoffset = match_image->Xoffset;
	result->Yoffset = match_image->Yoffset;

	return result;
}

/* Is a zval a rectangular 2D array.
 */
static gboolean
is_2D(zval *array)
{
	int height;
	zval *row;
	int width;
	int y;

	array = zval_get_nonref(array); 

	if (Z_TYPE_P(array) != IS_ARRAY) {
		return FALSE;
	}

	height = zend_hash_num_elements(Z_ARRVAL_P(array));
	if ((row = zend_hash_index_find(Z_ARRVAL_P(array), 0)) == NULL ||
		!(row = zval_get_nonref(row)) || 
		Z_TYPE_P(row) != IS_ARRAY) { 
		return FALSE;
	}
	width = zend_hash_num_elements(Z_ARRVAL_P(row));

	for (y = 1; y < height; y++) {
		if ((row = zend_hash_index_find(Z_ARRVAL_P(array), y)) == NULL ||
			!(row = zval_get_nonref(row)) ||
			Z_TYPE_P(row) != IS_ARRAY ||
			zend_hash_num_elements(Z_ARRVAL_P(row)) != width) {
			return FALSE;
		}
	}

	return TRUE;
}

/* Make a vips matrix image from a 2D zval. @array must have passed is_2D()
 * before calling this.
 */
static VipsImage *
matrix_from_zval(zval *array)
{
	int width;
	int height;
	zval *row;
	VipsImage *mat;
	int x, y;

	array = zval_get_nonref(array); 

	height = zend_hash_num_elements(Z_ARRVAL_P(array));
	row = zend_hash_index_find(Z_ARRVAL_P(array), 0);
	row = zval_get_nonref(row); 
	g_assert(Z_TYPE_P(row) == IS_ARRAY);
	width = zend_hash_num_elements(Z_ARRVAL_P(row));
	mat = vips_image_new_matrix(width, height);

	for (y = 0; y < height; y++) {
		row = zend_hash_index_find(Z_ARRVAL_P(array), y);
		row = zval_get_nonref(row); 
		g_assert(Z_TYPE_P(row) == IS_ARRAY);
		g_assert(zend_hash_num_elements(Z_ARRVAL_P(row)) == width);

		for (x = 0; x < width; x++) {
			zval *ele;

			ele = zend_hash_index_find(Z_ARRVAL_P(row), x);
			*VIPS_MATRIX(mat, x, y) = zval_get_double(ele);
		}
	}

	return mat;
}

/* Turn a zval into an image. An image stays an image, a 2D array of numbers 
 * becomes a matrix image, a 1D array or a simple constant is expanded to 
 * match @match_image.
 */
static VipsImage *
imageize(VipsImage *match_image, zval *zvalue)
{
	VipsImage *image;

	zvalue = zval_get_nonref(zvalue); 

	if (Z_TYPE_P(zvalue) == IS_RESOURCE &&
		(image = (VipsImage *) 
			zend_fetch_resource(Z_RES_P(zvalue), phalcon_image_vips_type_name, le_vips_gobject))) { 
		return image;
	}
	else if (is_2D(zvalue)) {
		return matrix_from_zval(zvalue);
	}
	else if (match_image) {
		return expand_constant(match_image, zvalue);
	}
	else {
		php_error_docref(NULL, E_WARNING, "not a VipsImage");
		return NULL;
	}
}

static int
zval_to_array_image(VipsImage *match_image, zval *zvalue, GValue *gvalue)
{
	VipsImage **arr;
	VipsImage *image;
	int n;
	int i;

	zvalue = zval_get_nonref(zvalue); 

	if (Z_TYPE_P(zvalue) == IS_ARRAY) {
		n = zend_hash_num_elements(Z_ARRVAL_P(zvalue));
	}
	else {
		n = 1;
	}

	vips_value_set_array_image(gvalue, n);
	arr = vips_value_get_array_image(gvalue, NULL);

	if (Z_TYPE_P(zvalue) == IS_ARRAY) {
		for (i = 0; i < n; i++) {
			zval *ele;

			ele = zend_hash_index_find(Z_ARRVAL_P(zvalue), i);
			if (!ele) {
				php_error_docref(NULL, E_WARNING, "element missing from array");
				return -1;
			}

			if (!(image = imageize(match_image, ele))) {
				return -1;
			}

			arr[i] = image;
			g_object_ref(image);
		}
	}
	else {
		if (!(image = imageize(match_image, zvalue))) {
			return -1;
		}

		arr[0] = image;
		g_object_ref(image);
	}

	return 0;
}

/* Set a gvalue from a php value. 
 *
 * You must set the type of the gvalue before calling this to hint what kind 
 * of gvalue to make. For example if type is an enum, a zval string will be 
 * used to look up the enum nick.
 *
 * If non-NULL, @match_image is used to turn constants into images. 
 */
static int
vips_php_zval_to_gval(VipsImage *match_image, zval *zvalue, GValue *gvalue)
{
	GType type = G_VALUE_TYPE(gvalue);

	/* The fundamental type ... eg. G_TYPE_ENUM for a VIPS_TYPE_KERNEL, or
	 * G_TYPE_OBJECT for VIPS_TYPE_IMAGE().
	 */
	GType fundamental = G_TYPE_FUNDAMENTAL(type);

	VipsImage *image;
	zend_string *zstr;
	int enum_value;

	switch (fundamental) {
		case G_TYPE_STRING:
			/* These are GStrings, vips refstrings are handled by boxed, see 
			 * below.
			 */
			zstr = zval_get_string(zvalue);
			g_value_set_string(gvalue, ZSTR_VAL(zstr));
			zend_string_release(zstr); 
			break;

		case G_TYPE_OBJECT:
			if (!(image = imageize(match_image, zvalue))) {
				return -1;
			}
			g_value_set_object(gvalue, image);
			break;

		case G_TYPE_INT:
			g_value_set_int(gvalue, zval_get_long(zvalue));
			break;

		case G_TYPE_UINT64:
			g_value_set_uint64(gvalue, zval_get_long(zvalue));
			break;

		case G_TYPE_BOOLEAN:
			g_value_set_boolean(gvalue, zval_get_long(zvalue));
			break;

		case G_TYPE_ENUM:
			zvalue = zval_get_nonref(zvalue); 

			if (Z_TYPE_P(zvalue) == IS_LONG) {
				enum_value = Z_LVAL_P(zvalue);
			}
			else if (Z_TYPE_P(zvalue) == IS_DOUBLE) {
				enum_value = Z_DVAL_P(zvalue);
			}
			else {
				zstr = zval_get_string(zvalue);
				enum_value = vips_enum_from_nick("enum", type, ZSTR_VAL(zstr));
				if (enum_value < 0) {
					zend_string_release(zstr); 
					return -1;
				}
				zend_string_release(zstr); 
			}
			g_value_set_enum(gvalue, enum_value);
			break;

		case G_TYPE_FLAGS:
			g_value_set_flags(gvalue, zval_get_long(zvalue));
			break;

		case G_TYPE_DOUBLE:
			g_value_set_double(gvalue, zval_get_double(zvalue));
			break;

		case G_TYPE_BOXED:
			if (type == VIPS_TYPE_REF_STRING) {
				zstr = zval_get_string(zvalue);
				vips_value_set_ref_string(gvalue, ZSTR_VAL(zstr));
				zend_string_release(zstr); 
			}
			else if (type == VIPS_TYPE_BLOB) {
				void *buf;

				zvalue = zval_get_nonref(zvalue); 

				zstr = zval_get_string(zvalue);
				buf = g_malloc(ZSTR_LEN(zstr));
				memcpy(buf, ZSTR_VAL(zstr), ZSTR_LEN(zstr));
				zend_string_release(zstr); 

				vips_value_set_blob(gvalue, 
					vips_php_blob_free, buf, Z_STRLEN_P(zvalue));
			}
			else if (type == VIPS_TYPE_ARRAY_INT) {
				int *arr;
				int n;
				int i;

				zvalue = zval_get_nonref(zvalue); 

				if (Z_TYPE_P(zvalue) == IS_ARRAY) {
					n = zend_hash_num_elements(Z_ARRVAL_P(zvalue));
				}
				else {
					n = 1;
				}

				vips_value_set_array_int(gvalue, NULL, n);
				arr = vips_value_get_array_int(gvalue, NULL);

				if (Z_TYPE_P(zvalue) == IS_ARRAY) {
					for (i = 0; i < n; i++) {
						zval *ele;

						ele = zend_hash_index_find(Z_ARRVAL_P(zvalue), i);
						if (ele) { 
							arr[i] = zval_get_long(ele);
						}
					}
				}
				else {
					arr[0] = zval_get_long(zvalue);
				}
			}
			else if (type == VIPS_TYPE_ARRAY_DOUBLE) {
				double *arr;
				int n;
				int i;

				zvalue = zval_get_nonref(zvalue); 

				if (Z_TYPE_P(zvalue) == IS_ARRAY) {
					n = zend_hash_num_elements(Z_ARRVAL_P(zvalue));
				}
				else {
					n = 1;
				}

				vips_value_set_array_double(gvalue, NULL, n);
				arr = vips_value_get_array_double(gvalue, NULL);

				if (Z_TYPE_P(zvalue) == IS_ARRAY) {
					for (i = 0; i < n; i++) {
						zval *ele;

						ele = zend_hash_index_find(Z_ARRVAL_P(zvalue), i);
						if (ele) { 
							arr[i] = zval_get_double(ele);
						}
					}
				}
				else {
					arr[0] = zval_get_double(zvalue);
				}
			}
			else if (type == VIPS_TYPE_ARRAY_IMAGE) {
				if (zval_to_array_image(match_image, zvalue, gvalue)) {
					return -1;
				}
			}
			else {
				g_warning( "%s: unimplemented boxed type %s", 
					G_STRLOC, g_type_name(type) );
			}
			break;

		default:
			g_warning( "%s: unimplemented GType %s", 
				G_STRLOC, g_type_name(fundamental) );
			break;
	}

	return 0;
}

static int
vips_php_set_value(VipsPhpCall *call, 
	GParamSpec *pspec, VipsArgumentFlags flags, zval *zvalue)
{
	const char *name = g_param_spec_get_name(pspec);
	GType pspec_type = G_PARAM_SPEC_VALUE_TYPE(pspec);
	GValue gvalue = { 0 };

	g_value_init(&gvalue, pspec_type);
	if (vips_php_zval_to_gval(call->match_image, zvalue, &gvalue)) {
		g_value_unset(&gvalue);
		return -1;
	}

	/* If we are setting a MODIFY VipsArgument with an image, we need to take a
	 * copy.
	 */
	if (g_type_is_a(pspec_type, VIPS_TYPE_IMAGE) &&
		(flags & VIPS_ARGUMENT_MODIFY)) {
		VipsImage *image;
		VipsImage *memory;

		VIPS_DEBUG_MSG("vips_php_set_value: copying image\n");

		image = (VipsImage *) g_value_get_object(&gvalue);
		memory = vips_image_new_memory();
		if (vips_image_write(image, memory)) {
			g_object_unref(memory);
			g_value_unset(&gvalue);
			return -1;
		}
		g_value_unset(&gvalue);
		g_value_init(&gvalue, pspec_type);
		g_value_set_object(&gvalue, memory);
	}

#ifdef VIPS_DEBUG
{
	char *str_value;

	str_value = g_strdup_value_contents(&gvalue);
	VIPS_DEBUG_MSG("    %s.%s = %s\n", call->operation_name, name, str_value);
	g_free(str_value);
}
#endif/*VIPS_DEBUG*/

	g_object_set_property(G_OBJECT(call->operation), name, &gvalue);
	g_value_unset(&gvalue);

	return 0;
}

static void *
vips_php_set_required_input(VipsObject *object, 
	GParamSpec *pspec, VipsArgumentClass *argument_class, 
	VipsArgumentInstance *argument_instance, 
	void *a, void *b)
{
	VipsPhpCall *call = (VipsPhpCall *) a;

	if ((argument_class->flags & VIPS_ARGUMENT_REQUIRED) &&
		(argument_class->flags & VIPS_ARGUMENT_CONSTRUCT) &&
		(argument_class->flags & VIPS_ARGUMENT_INPUT) &&
		!(argument_class->flags & VIPS_ARGUMENT_DEPRECATED) &&
		!argument_instance->assigned) {
		zval *arg;

		/* If this arg needs an image, we use instance, if we can.
		 */
		arg = NULL;
		if (G_PARAM_SPEC_VALUE_TYPE(pspec) == VIPS_TYPE_IMAGE &&
			call->instance &&
			!call->used_instance) {
			arg = call->instance;
			call->used_instance = TRUE;
		}
		else if (call->args_required < call->argc) {
			/* Pick the next zval off the supplied arg list.
			 */
			arg = &call->argv[call->args_required];
			call->args_required += 1;
		}
				
		if (arg &&
			vips_php_set_value(call, pspec, argument_class->flags, arg)) {
			return call;
		}
	}

	return NULL;
}

/* Set all optional arguments.
 */
static int
vips_php_set_optional_input(VipsPhpCall *call, zval *options)
{
	zend_string *key;
	zval *value;

	VIPS_DEBUG_MSG("vips_php_set_optional_input:\n");

	options = zval_get_nonref(options); 

	ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(options), key, value) {
		char *name;
		GParamSpec *pspec;
		VipsArgumentClass *argument_class;
		VipsArgumentInstance *argument_instance;

		if (key == NULL) {
			continue;
		}

		name = ZSTR_VAL(key);
		if (vips_object_get_argument(VIPS_OBJECT(call->operation), name,
			&pspec, &argument_class, &argument_instance)) {
			return -1;
		}

		if (!(argument_class->flags & VIPS_ARGUMENT_REQUIRED) &&
			(argument_class->flags & VIPS_ARGUMENT_INPUT) &&
			!(argument_class->flags & VIPS_ARGUMENT_DEPRECATED) &&
			vips_php_set_value(call, pspec, argument_class->flags, value)) {
			return -1;
		}
	} ZEND_HASH_FOREACH_END();

	return 0;
}

/* Set a php zval from a gvalue. 
 */
static int
vips_php_gval_to_zval(GValue *gvalue, zval *zvalue)
{
	GType type = G_VALUE_TYPE(gvalue);

	/* The fundamental type ... eg. G_TYPE_ENUM for a VIPS_TYPE_KERNEL, or
	 * G_TYPE_OBJECT for VIPS_TYPE_IMAGE().
	 */
	GType fundamental = G_TYPE_FUNDAMENTAL(type);

	const char *str;
	GObject *gobject;
	zend_resource *resource;
	int enum_value;

	switch (fundamental) {
		case G_TYPE_STRING:
			/* These are GStrings, vips refstrings are handled by boxed, see 
			 * below.
			 */
			str = g_value_get_string(gvalue);
			ZVAL_STRING(zvalue, str);
			break;

		case G_TYPE_OBJECT:
			gobject = g_value_get_object(gvalue);
			resource = zend_register_resource(gobject, le_vips_gobject);
			ZVAL_RES(zvalue, resource);
			break;

		case G_TYPE_INT:
			ZVAL_LONG(zvalue, g_value_get_int(gvalue));
			break;

		case G_TYPE_UINT64:
			ZVAL_LONG(zvalue, g_value_get_uint64(gvalue));
			break;

		case G_TYPE_BOOLEAN:
			ZVAL_LONG(zvalue, g_value_get_boolean(gvalue));
			break;

		case G_TYPE_ENUM:
			enum_value = g_value_get_enum(gvalue);
			str = vips_enum_nick(type, enum_value);
			ZVAL_STRING(zvalue, str);
			break;

		case G_TYPE_FLAGS:
			ZVAL_LONG(zvalue, g_value_get_flags(gvalue));
			break;

		case G_TYPE_DOUBLE:
			ZVAL_DOUBLE(zvalue, g_value_get_double(gvalue));
			break;

		case G_TYPE_BOXED:
			if (type == VIPS_TYPE_REF_STRING ||
				type == VIPS_TYPE_BLOB) {
				const char *str;
				size_t str_len;

				str = vips_value_get_ref_string(gvalue, &str_len);
				ZVAL_STRINGL(zvalue, str, str_len);
			}
			else if (type == VIPS_TYPE_ARRAY_DOUBLE) {
				double *arr;
				int n;
				int i;

				arr = vips_value_get_array_double(gvalue, &n);
				array_init(zvalue);
				for (i = 0; i < n; i++) {
					add_next_index_double(zvalue, arr[i]);
				}
			}
			else if (type == VIPS_TYPE_ARRAY_INT) {
				int *arr;
				int n;
				int i;

				arr = vips_value_get_array_int(gvalue, &n);
				array_init(zvalue);
				for (i = 0; i < n; i++) {
					add_next_index_long(zvalue, arr[i]);
				}
			}
			else if (type == VIPS_TYPE_ARRAY_IMAGE) {
				VipsImage **arr;
				int n;
				int i;

				arr = vips_value_get_array_image(gvalue, &n);
				array_init(zvalue);
				for (i = 0; i < n; i++) {
					zval x;

					g_object_ref(arr[i]);
					resource = zend_register_resource(arr[i], le_vips_gobject);
					ZVAL_RES(&x, resource);
					add_next_index_zval(zvalue, &x);
				}
			}
			else {
				g_warning( "%s: unimplemented boxed type %s", 
					G_STRLOC, g_type_name(type));
			}
			break;

		default:
			g_warning( "%s: unimplemented GType %s", 
				G_STRLOC, g_type_name(fundamental));
			break;
	}

	return 0;
}

static int
vips_php_get_value(VipsPhpCall *call, GParamSpec *pspec, zval *zvalue)
{
	const char *name = g_param_spec_get_name(pspec);
	GType pspec_type = G_PARAM_SPEC_VALUE_TYPE(pspec);
	GValue gvalue = { 0 }; 

	g_value_init(&gvalue, pspec_type);
	g_object_get_property(G_OBJECT(call->operation), name, &gvalue);
	if (vips_php_gval_to_zval(&gvalue, zvalue)) {
		g_value_unset(&gvalue);
		return -1;
	}

#ifdef VIPS_DEBUG
{
	char *str_value;

	str_value = g_strdup_value_contents(&gvalue);
	VIPS_DEBUG_MSG("    %s.%s = %s\n", call->operation_name, name, str_value);
	g_free(str_value);
}
#endif/*VIPS_DEBUG*/

	g_value_unset(&gvalue);

	return 0;
}

static void *
vips_php_get_required_output(VipsObject *object, 
	GParamSpec *pspec, VipsArgumentClass *argument_class, 
	VipsArgumentInstance *argument_instance, 
	void *a, void *b)
{
	VipsPhpCall *call = (VipsPhpCall *) a;
	zval *return_value = (zval *) b;

	/* We get output objects, and we get input objects that are tagged as
	 * MODIFY --- they are copied on set, see above.
	 */
	if ((argument_class->flags & VIPS_ARGUMENT_REQUIRED) &&
		(argument_class->flags & (VIPS_ARGUMENT_OUTPUT| VIPS_ARGUMENT_MODIFY)) &&
		!(argument_class->flags & VIPS_ARGUMENT_DEPRECATED)) { 
		const char *name = g_param_spec_get_name(pspec);
		zval zvalue;

		if (vips_php_get_value(call, pspec, &zvalue)) {
			return call;
		}
		add_assoc_zval(return_value, name, &zvalue);
	}

	return NULL;
}

static int
vips_php_get_optional_output(VipsPhpCall *call, zval *options, 
	zval *return_value)
{
	zend_string *key;
	zval *value;

	VIPS_DEBUG_MSG("vips_php_get_optional_output:\n");

	options = zval_get_nonref(options); 

	ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(options), key, value) {
		char *name;
		GParamSpec *pspec;
		VipsArgumentClass *argument_class;
		VipsArgumentInstance *argument_instance;

		if (key == NULL) {
			continue;
		}

		/* value should always be TRUE. 
		 */
		value = zval_get_nonref(value); 
		if (Z_TYPE_P(value) != IS_TRUE) {
			continue;
		}

		name = ZSTR_VAL(key);
		if (vips_object_get_argument(VIPS_OBJECT(call->operation), name,
			&pspec, &argument_class, &argument_instance)) {
			return -1;
		}

		if (!(argument_class->flags & VIPS_ARGUMENT_REQUIRED) &&
			(argument_class->flags & VIPS_ARGUMENT_OUTPUT) &&
			!(argument_class->flags & VIPS_ARGUMENT_DEPRECATED)) {
			zval zvalue;

			if (vips_php_get_value(call, pspec, &zvalue)) {
				return -1;
			}

			add_assoc_zval(return_value, name, &zvalue);
		}
	} ZEND_HASH_FOREACH_END();

	return 0;
}

/* Call any vips operation, with the arguments coming from an array of zval. 
 * argv can have an extra final arg, which is an associative array of 
 * optional arguments. 
 */
static int
vips_php_call_array(const char *operation_name, zval *instance, 
	const char *option_string, int argc, zval *argv, zval *return_value)
{
	VipsPhpCall *call;
	int i;

	VIPS_DEBUG_MSG("vips_php_call_array:\n");

	if (!(call = vips_php_call_new(operation_name, instance, option_string,
		argc, argv))) {
		return -1;
	}

	/* Some initial analysis of our args. Loop over them all, including the
	 * special 'instance' arg.
	 */
	VIPS_DEBUG_MSG("vips_php_call_array: analyzing input args ...\n");
	if (call->instance) {
		vips_php_analyze_arg(call, call->instance);
	}
	for (i = 0; i < argc; i++) {
		vips_php_analyze_arg(call, &call->argv[i]);
	}

	/* Set str options before vargs options, so the user can't
	 * override things we set deliberately.
	 */
	VIPS_DEBUG_MSG("vips_php_call_array: setting args from option_string ...\n");
	if (option_string &&
		vips_object_set_from_string(VIPS_OBJECT(call->operation), 
			option_string)) {
		vips_object_unref_outputs(VIPS_OBJECT(call->operation));
		vips_php_call_free(call);
		return -1;
	}

	/* Set all required input args from argv.
	 */
	VIPS_DEBUG_MSG("vips_php_call_array: setting required input args ...\n");
	if (vips_argument_map(VIPS_OBJECT(call->operation), 
		vips_php_set_required_input, call, NULL)) {
		vips_object_unref_outputs(VIPS_OBJECT(call->operation));
		vips_php_call_free(call);
		return -1;
	}

	/* args_required must match argc, or we allow one extra final arg for options.
	 */
	VIPS_DEBUG_MSG("vips_php_call_array: testing argc ...\n");
	if (call->argc == call->args_required + 1) {
		/* Make sure it really is an array.
		 */
		if (zend_parse_parameter(0, call->argc - 1, &call->argv[call->argc - 1],
			"a", &call->options) == FAILURE) {
			vips_object_unref_outputs(VIPS_OBJECT(call->operation));
			vips_php_call_free(call);
			return -1;
		}
	}
	else if (call->argc != call->args_required) {
		php_error_docref(NULL, E_WARNING, 
			"operation %s expects %d arguments, but you supplied %d",
			call->operation_name, call->args_required, call->argc);
		vips_object_unref_outputs(VIPS_OBJECT(call->operation));
		vips_php_call_free(call);
		return -1;
	}

	/* Set all optional arguments.
	 */
	VIPS_DEBUG_MSG("vips_php_call_array: setting optional input args ...\n");
	if (call->options &&
		vips_php_set_optional_input(call, call->options)) {
		vips_object_unref_outputs(VIPS_OBJECT(call->operation));
		vips_php_call_free(call);
		return -1;
	}

	/* Look up in cache and build.
	 */
	VIPS_DEBUG_MSG("vips_php_call_array: building ...\n");
	if (vips_cache_operation_buildp(&call->operation)) {
		VIPS_DEBUG_MSG("vips_php_call_array: call failed!\n");
		vips_object_unref_outputs(VIPS_OBJECT(call->operation));
		vips_php_call_free(call);
		return -1;
	}

	/* Walk args again, getting required output.
	 */
	VIPS_DEBUG_MSG("vips_php_call_array: getting required output ...\n");
	array_init(return_value);
	if (vips_argument_map(VIPS_OBJECT(call->operation), 
		vips_php_get_required_output, call, return_value)) {
		vips_object_unref_outputs(VIPS_OBJECT(call->operation));
		vips_php_call_free(call);
		return -1;
	}

	/* And optional output.
	 */
	VIPS_DEBUG_MSG("vips_php_call_array: getting optional output ...\n");
	if (call->options &&
		vips_php_get_optional_output(call, call->options, return_value)) {
		vips_object_unref_outputs(VIPS_OBJECT(call->operation));
		vips_php_call_free(call);
		return -1;
	}

	vips_php_call_free(call);

	VIPS_DEBUG_MSG("vips_php_call_array: success!\n");

	return 0;
}

/**
 * Phalcon\Image\Vips
 *
 * Image manipulation support. Allows images to be resized, cropped, etc.
 *
 *<code>
 *	$image = Phalcon\Image\Vips::call('black', 100, 100);
 *	$image->save();
 *</code>
 */
zend_class_entry *phalcon_image_vips_ce;

PHP_METHOD(Phalcon_Image_Vips, __construct);
PHP_METHOD(Phalcon_Image_Vips, getVipsImage);
PHP_METHOD(Phalcon_Image_Vips, __get);
PHP_METHOD(Phalcon_Image_Vips, __set);
PHP_METHOD(Phalcon_Image_Vips, __isset);
PHP_METHOD(Phalcon_Image_Vips, __toString);

PHP_METHOD(Phalcon_Image_Vips, identity);
PHP_METHOD(Phalcon_Image_Vips, black);
PHP_METHOD(Phalcon_Image_Vips, text);
PHP_METHOD(Phalcon_Image_Vips, xyz);
PHP_METHOD(Phalcon_Image_Vips, grey);
PHP_METHOD(Phalcon_Image_Vips, gaussmat);
PHP_METHOD(Phalcon_Image_Vips, logmat);
PHP_METHOD(Phalcon_Image_Vips, newFromArray);
PHP_METHOD(Phalcon_Image_Vips, newFromBuffer);
PHP_METHOD(Phalcon_Image_Vips, newFromMemory);
PHP_METHOD(Phalcon_Image_Vips, newFromFile);

PHP_METHOD(Phalcon_Image_Vips, crop);
PHP_METHOD(Phalcon_Image_Vips, smartcrop);
PHP_METHOD(Phalcon_Image_Vips, resize);
PHP_METHOD(Phalcon_Image_Vips, rotate);
PHP_METHOD(Phalcon_Image_Vips, composite);
PHP_METHOD(Phalcon_Image_Vips, composite2);

PHP_METHOD(Phalcon_Image_Vips, embed);
PHP_METHOD(Phalcon_Image_Vips, ifthenelse);

PHP_METHOD(Phalcon_Image_Vips, getpoint);
PHP_METHOD(Phalcon_Image_Vips, relational);
PHP_METHOD(Phalcon_Image_Vips, relationalConst);
PHP_METHOD(Phalcon_Image_Vips, add);
PHP_METHOD(Phalcon_Image_Vips, subtract);
PHP_METHOD(Phalcon_Image_Vips, multiply);
PHP_METHOD(Phalcon_Image_Vips, divide);
PHP_METHOD(Phalcon_Image_Vips, linear);
PHP_METHOD(Phalcon_Image_Vips, max);
PHP_METHOD(Phalcon_Image_Vips, min);
PHP_METHOD(Phalcon_Image_Vips, math);
PHP_METHOD(Phalcon_Image_Vips, math2);
PHP_METHOD(Phalcon_Image_Vips, boolean);

PHP_METHOD(Phalcon_Image_Vips, drawImage);
PHP_METHOD(Phalcon_Image_Vips, drawMask);
PHP_METHOD(Phalcon_Image_Vips, drawSmudge);
PHP_METHOD(Phalcon_Image_Vips, drawFlood);
PHP_METHOD(Phalcon_Image_Vips, drawLine);
PHP_METHOD(Phalcon_Image_Vips, drawRect);
PHP_METHOD(Phalcon_Image_Vips, drawCircle);

PHP_METHOD(Phalcon_Image_Vips, writeToFile);
PHP_METHOD(Phalcon_Image_Vips, writeToBuffer);
PHP_METHOD(Phalcon_Image_Vips, writeToMemory);
PHP_METHOD(Phalcon_Image_Vips, writeToArray);

PHP_METHOD(Phalcon_Image_Vips, call);
PHP_METHOD(Phalcon_Image_Vips, new_from_file);
PHP_METHOD(Phalcon_Image_Vips, new_from_buffer);
PHP_METHOD(Phalcon_Image_Vips, new_from_array);
PHP_METHOD(Phalcon_Image_Vips, interpolate_new);
PHP_METHOD(Phalcon_Image_Vips, write_to_file);
PHP_METHOD(Phalcon_Image_Vips, write_to_buffer);
PHP_METHOD(Phalcon_Image_Vips, copy_memory);
PHP_METHOD(Phalcon_Image_Vips, new_from_memory);
PHP_METHOD(Phalcon_Image_Vips, write_to_memory);
PHP_METHOD(Phalcon_Image_Vips, write_to_array);
PHP_METHOD(Phalcon_Image_Vips, foreign_find_load);
PHP_METHOD(Phalcon_Image_Vips, foreign_find_load_buffer);
PHP_METHOD(Phalcon_Image_Vips, get);
PHP_METHOD(Phalcon_Image_Vips, get_typeof);
PHP_METHOD(Phalcon_Image_Vips, set);
PHP_METHOD(Phalcon_Image_Vips, remove);
PHP_METHOD(Phalcon_Image_Vips, error_buffer);
PHP_METHOD(Phalcon_Image_Vips, cache_set_max);
PHP_METHOD(Phalcon_Image_Vips, cache_get_max);
PHP_METHOD(Phalcon_Image_Vips, cache_set_max_mem);
PHP_METHOD(Phalcon_Image_Vips, cache_get_max_mem);
PHP_METHOD(Phalcon_Image_Vips, cache_set_max_files);
PHP_METHOD(Phalcon_Image_Vips, cache_get_max_files);
PHP_METHOD(Phalcon_Image_Vips, cache_get_size);
PHP_METHOD(Phalcon_Image_Vips, concurrency_set);
PHP_METHOD(Phalcon_Image_Vips, concurrency_get);
PHP_METHOD(Phalcon_Image_Vips, version);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips___construct, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, image, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_identity, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_black, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, height, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_text, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_xyz, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, height, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_grey, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, height, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_gaussmat, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, sigma, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO(0, min_ampl, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_logmat, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, sigma, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO(0, min_ampl, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_newfromarray, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, coefficients, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, scale, IS_DOUBLE, 1)
	ZEND_ARG_TYPE_INFO(0, offset, IS_DOUBLE, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_newfrombuffer, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, buffer, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, option_string, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_newfrommemory, 0, 0, 5)
	ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, height, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, bands, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, format, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_newfromfile, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_crop, 0, 0, 4)
	ZEND_ARG_TYPE_INFO(0, left, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, top, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, height, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_smartcrop, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, height, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_resize, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, scale, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_rotate, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, angle, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_composite, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, overlays, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, modes, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_composite2, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, overlay, IS_OBJECT, 0)
	ZEND_ARG_TYPE_INFO(0, mode, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_embed, 0, 0, 4)
	ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, y, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, height, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_ifthenelse, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, condthen, IS_OBJECT, 0)
	ZEND_ARG_TYPE_INFO(0, condelse, IS_OBJECT, 0)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()


ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_getpoint, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, y, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_relational, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, right, IS_OBJECT, 0)
	ZEND_ARG_TYPE_INFO(0, relational, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_relationalconst, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, right, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, relational, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_add, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, other, IS_OBJECT, 0)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_subtract, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, other, IS_OBJECT, 0)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_multiply, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, other, IS_OBJECT, 0)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_divide, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, other, IS_OBJECT, 0)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_linear, 0, 0, 2)
	ZEND_ARG_INFO(0, a)
	ZEND_ARG_INFO(0, b)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_max, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_min, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_math, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, operation, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_math2, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, right, IS_OBJECT, 0)
	ZEND_ARG_TYPE_INFO(0, operation, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_boolean, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, right, IS_OBJECT, 0)
	ZEND_ARG_TYPE_INFO(0, operation, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()


ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_drawimage, 0, 0, 3)
	ZEND_ARG_TYPE_INFO(0, sub, IS_OBJECT, 0)
	ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, y, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_drawmask, 0, 0, 4)
	ZEND_ARG_TYPE_INFO(0, mask, IS_OBJECT, 0)
	ZEND_ARG_TYPE_INFO(0, color, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, y, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_drawsmudge, 0, 0, 4)
	ZEND_ARG_TYPE_INFO(0, left, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, top, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, height, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_drawflood, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, y, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_drawline, 0, 0, 5)
	ZEND_ARG_TYPE_INFO(0, color, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, x1, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, y1, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, x2, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, y2, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_drawrect, 0, 0, 5)
	ZEND_ARG_TYPE_INFO(0, color, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, left, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, top, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, height, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_drawcircle, 0, 0, 4)
	ZEND_ARG_TYPE_INFO(0, color, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, cx, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, cy, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, radius, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_writetofile, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_writetobuffer, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, suffix, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_call, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, operation_name, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, image, IS_RESOURCE, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_new_from_file, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_new_from_buffer, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, buffer, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, option_string, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_new_from_array, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, coefficients, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, scale, IS_DOUBLE, 1)
	ZEND_ARG_TYPE_INFO(0, offset, IS_DOUBLE, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_interpolate_new, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_write_to_file, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, image, IS_RESOURCE, 0)
	ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_write_to_buffer, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, image, IS_RESOURCE, 0)
	ZEND_ARG_TYPE_INFO(0, suffix, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_copy_memory, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, image, IS_RESOURCE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_new_from_memory, 0, 0, 5)
	ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, height, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, bands, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, format, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_write_to_memory, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, image, IS_RESOURCE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_write_to_array, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, image, IS_RESOURCE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_foreign_find_load, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_foreign_find_load_buffer, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, buffer, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_get, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, image, IS_RESOURCE, 0)
	ZEND_ARG_TYPE_INFO(0, field, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_get_typeof, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, image, IS_RESOURCE, 0)
	ZEND_ARG_TYPE_INFO(0, field, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_set, 0, 0, 3)
	ZEND_ARG_TYPE_INFO(0, image, IS_RESOURCE, 0)
	ZEND_ARG_TYPE_INFO(0, field, IS_STRING, 0)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_remove, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, image, IS_RESOURCE, 0)
	ZEND_ARG_TYPE_INFO(0, field, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_cache_set_max, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, value, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_cache_set_max_mem, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, value, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_cache_set_max_files, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, value, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_vips_concurrency_set, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, value, IS_LONG, 0)
ZEND_END_ARG_INFO()


static const zend_function_entry phalcon_image_vips_method_entry[] = {
	PHP_ME(Phalcon_Image_Vips, __construct, arginfo_phalcon_image_vips___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Image_Vips, getVipsImage, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Vips, __get, arginfo___getref, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Vips, __set, arginfo___set, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Vips, __isset, arginfo___isset, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Vips, __toString, NULL, ZEND_ACC_PUBLIC)

	PHP_ME(Phalcon_Image_Vips, identity, arginfo_phalcon_image_vips_identity, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Vips, black, arginfo_phalcon_image_vips_black, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Vips, text, arginfo_phalcon_image_vips_text, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Vips, xyz, arginfo_phalcon_image_vips_xyz, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Vips, grey, arginfo_phalcon_image_vips_grey, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Vips, gaussmat, arginfo_phalcon_image_vips_gaussmat, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Vips, logmat, arginfo_phalcon_image_vips_logmat, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Vips, newFromArray, arginfo_phalcon_image_vips_newfromarray, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Vips, newFromBuffer, arginfo_phalcon_image_vips_newfrombuffer, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Vips, newFromMemory, arginfo_phalcon_image_vips_newfrommemory, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Vips, newFromFile, arginfo_phalcon_image_vips_newfromfile, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)

	PHP_ME(Phalcon_Image_Vips, crop, arginfo_phalcon_image_vips_crop, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Vips, smartcrop, arginfo_phalcon_image_vips_smartcrop, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Vips, resize, arginfo_phalcon_image_vips_resize, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Vips, rotate, arginfo_phalcon_image_vips_rotate, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Vips, composite, arginfo_phalcon_image_vips_composite, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Vips, composite2, arginfo_phalcon_image_vips_composite2, ZEND_ACC_PUBLIC)

	PHP_ME(Phalcon_Image_Vips, embed, arginfo_phalcon_image_vips_embed, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Vips, ifthenelse, arginfo_phalcon_image_vips_ifthenelse, ZEND_ACC_PUBLIC)

	PHP_ME(Phalcon_Image_Vips, getpoint, arginfo_phalcon_image_vips_getpoint, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Vips, relational, arginfo_phalcon_image_vips_relational, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Vips, relationalConst, arginfo_phalcon_image_vips_relationalconst, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Vips, add, arginfo_phalcon_image_vips_add, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Vips, subtract, arginfo_phalcon_image_vips_subtract, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Vips, multiply, arginfo_phalcon_image_vips_multiply, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Vips, divide, arginfo_phalcon_image_vips_divide, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Vips, linear, arginfo_phalcon_image_vips_linear, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Vips, max, arginfo_phalcon_image_vips_max, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Vips, min, arginfo_phalcon_image_vips_min, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Vips, math, arginfo_phalcon_image_vips_math, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Vips, math2, arginfo_phalcon_image_vips_math2, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Vips, boolean, arginfo_phalcon_image_vips_boolean, ZEND_ACC_PUBLIC)

	PHP_ME(Phalcon_Image_Vips, drawImage, arginfo_phalcon_image_vips_drawimage, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Vips, drawMask, arginfo_phalcon_image_vips_drawmask, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Vips, drawSmudge, arginfo_phalcon_image_vips_drawsmudge, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Vips, drawFlood, arginfo_phalcon_image_vips_drawflood, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Vips, drawLine, arginfo_phalcon_image_vips_drawline, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Vips, drawRect, arginfo_phalcon_image_vips_drawrect, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Vips, drawCircle, arginfo_phalcon_image_vips_drawcircle, ZEND_ACC_PUBLIC)

	PHP_ME(Phalcon_Image_Vips, writeToFile, arginfo_phalcon_image_vips_writetofile, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Vips, writeToBuffer, arginfo_phalcon_image_vips_writetobuffer, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Vips, writeToMemory, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Vips, writeToArray, NULL, ZEND_ACC_PUBLIC)

	PHP_ME(Phalcon_Image_Vips, call, arginfo_phalcon_image_vips_call, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Image_Vips, new_from_file, arginfo_phalcon_image_vips_new_from_file, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Image_Vips, new_from_buffer, arginfo_phalcon_image_vips_new_from_buffer, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Image_Vips, new_from_array, arginfo_phalcon_image_vips_new_from_array, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Image_Vips, interpolate_new, arginfo_phalcon_image_vips_interpolate_new, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Image_Vips, write_to_file, arginfo_phalcon_image_vips_write_to_file, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Image_Vips, write_to_buffer, arginfo_phalcon_image_vips_write_to_buffer, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Image_Vips, copy_memory, arginfo_phalcon_image_vips_copy_memory, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Image_Vips, new_from_memory, arginfo_phalcon_image_vips_new_from_memory, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Image_Vips, write_to_memory, arginfo_phalcon_image_vips_write_to_memory, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Image_Vips, write_to_array, arginfo_phalcon_image_vips_write_to_array, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Image_Vips, foreign_find_load, arginfo_phalcon_image_vips_foreign_find_load, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Image_Vips, foreign_find_load_buffer, arginfo_phalcon_image_vips_foreign_find_load_buffer, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Image_Vips, get, arginfo_phalcon_image_vips_get, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Image_Vips, get_typeof, arginfo_phalcon_image_vips_get_typeof, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Image_Vips, set, arginfo_phalcon_image_vips_set, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Image_Vips, remove, arginfo_phalcon_image_vips_remove, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Image_Vips, error_buffer, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Image_Vips, cache_set_max, arginfo_phalcon_image_vips_cache_set_max, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Image_Vips, cache_get_max, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Image_Vips, cache_set_max_mem, arginfo_phalcon_image_vips_cache_set_max_mem, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Image_Vips, cache_get_max_mem, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Image_Vips, cache_set_max_files, arginfo_phalcon_image_vips_cache_set_max_files, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Image_Vips, cache_get_max_files, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Image_Vips, cache_get_size, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Image_Vips, concurrency_set, arginfo_phalcon_image_vips_concurrency_set, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Image_Vips, concurrency_get, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Image_Vips, version, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_FE_END
};

/**
 * Phalcon\Image\Vips initializer
 */
PHALCON_INIT_CLASS(Phalcon_Image_Vips){

	PHALCON_REGISTER_CLASS(Phalcon\\Image, Vips, image_vips, phalcon_image_vips_method_entry, 0);

	zend_declare_property_null(phalcon_image_vips_ce, SL("_image"), ZEND_ACC_PROTECTED);

	zend_declare_class_constant_string(phalcon_image_vips_ce, SL("OP_RELATIONAL_EQUAL"), "equal");
	zend_declare_class_constant_string(phalcon_image_vips_ce, SL("OP_RELATIONAL_NOTEQ"), "noteq");
	zend_declare_class_constant_string(phalcon_image_vips_ce, SL("OP_RELATIONAL_LESS"), "less");
	zend_declare_class_constant_string(phalcon_image_vips_ce, SL("OP_RELATIONAL_LESSEQ"),  "lesseq");
	zend_declare_class_constant_string(phalcon_image_vips_ce, SL("OP_RELATIONAL_MORE"),  "more");
	zend_declare_class_constant_string(phalcon_image_vips_ce, SL("OP_RELATIONAL_MOREEQ"),  "moreeq");

	zend_declare_class_constant_string(phalcon_image_vips_ce, SL("OP_MATH_SIN"), "sin");
	zend_declare_class_constant_string(phalcon_image_vips_ce, SL("OP_MATH_COS"), "cos");
	zend_declare_class_constant_string(phalcon_image_vips_ce, SL("OP_MATH_TAN"), "tan");
	zend_declare_class_constant_string(phalcon_image_vips_ce, SL("OP_MATH_ASIN"), "asin");
	zend_declare_class_constant_string(phalcon_image_vips_ce, SL("OP_MATH_ACOS"), "acos");
	zend_declare_class_constant_string(phalcon_image_vips_ce, SL("OP_MATH_ATAN"), "atan");
	zend_declare_class_constant_string(phalcon_image_vips_ce, SL("OP_MATH_LOG"), "log");
	zend_declare_class_constant_string(phalcon_image_vips_ce, SL("OP_MATH_LOG10"), "log10");
	zend_declare_class_constant_string(phalcon_image_vips_ce, SL("OP_MATH_EXP"), "exp");
	zend_declare_class_constant_string(phalcon_image_vips_ce, SL("OP_MATH_EXP10"), "exp10");

	zend_declare_class_constant_string(phalcon_image_vips_ce, SL("OP_MATH2_POW"), "pow");
	zend_declare_class_constant_string(phalcon_image_vips_ce, SL("OP_MATH2_WOP"), "wop");

	zend_declare_class_constant_string(phalcon_image_vips_ce, SL("OP_BOOLEAN_AND"), "and");
	zend_declare_class_constant_string(phalcon_image_vips_ce, SL("OP_BOOLEAN_OR"), "or");
	zend_declare_class_constant_string(phalcon_image_vips_ce, SL("OP_BOOLEAN_EOR"), "eor");
	zend_declare_class_constant_string(phalcon_image_vips_ce, SL("OP_BOOLEAN_LSHIFT"),  "lshift");
	zend_declare_class_constant_string(phalcon_image_vips_ce, SL("OP_BOOLEAN_RSHIFT"),  "rshift");

	zend_declare_class_constant_string(phalcon_image_vips_ce, SL("BLENDMODE_CLEAR"),  "clear");
	zend_declare_class_constant_string(phalcon_image_vips_ce, SL("BLENDMODE_SOURCE"),  "source");
	zend_declare_class_constant_string(phalcon_image_vips_ce, SL("BLENDMODE_OVER"),  "over");
	zend_declare_class_constant_string(phalcon_image_vips_ce, SL("BLENDMODE_IN"),  "in");
	zend_declare_class_constant_string(phalcon_image_vips_ce, SL("BLENDMODE_OUT"),  "out");
	zend_declare_class_constant_string(phalcon_image_vips_ce, SL("BLENDMODE_ATOP"),  "atop");
	zend_declare_class_constant_string(phalcon_image_vips_ce, SL("BLENDMODE_DEST"),  "dest");
	zend_declare_class_constant_string(phalcon_image_vips_ce, SL("BLENDMODE_DEST_OVER"),  "dest-over");
	zend_declare_class_constant_string(phalcon_image_vips_ce, SL("BLENDMODE_DEST_IN"),  "dest-in");
	zend_declare_class_constant_string(phalcon_image_vips_ce, SL("BLENDMODE_DEST_OUT"),  "dest-out");
	zend_declare_class_constant_string(phalcon_image_vips_ce, SL("BLENDMODE_DEST_ATOP"),  "dest-atop");
	zend_declare_class_constant_string(phalcon_image_vips_ce, SL("BLENDMODE_XOR"),  "xor");
	zend_declare_class_constant_string(phalcon_image_vips_ce, SL("BLENDMODE_ADD"),  "add");
	zend_declare_class_constant_string(phalcon_image_vips_ce, SL("BLENDMODE_SATURATE"),  "saturate");
	zend_declare_class_constant_string(phalcon_image_vips_ce, SL("BLENDMODE_MULTIPLY"),  "multiply");
	zend_declare_class_constant_string(phalcon_image_vips_ce, SL("BLENDMODE_SCREEN"),  "screen");
	zend_declare_class_constant_string(phalcon_image_vips_ce, SL("BLENDMODE_OVERLAY"),  "overlay");
	zend_declare_class_constant_string(phalcon_image_vips_ce, SL("BLENDMODE_DARKEN"),  "darken");
	zend_declare_class_constant_string(phalcon_image_vips_ce, SL("BLENDMODE_LIGHTEN"),  "lighten");
	zend_declare_class_constant_string(phalcon_image_vips_ce, SL("BLENDMODE_COLOUR_DODGE"),  "colour-dodge");
	zend_declare_class_constant_string(phalcon_image_vips_ce, SL("BLENDMODE_COLOUR_BURN"),  "colour-burn");
	zend_declare_class_constant_string(phalcon_image_vips_ce, SL("BLENDMODE_HARD_LIGHT"),  "hard-light");
	zend_declare_class_constant_string(phalcon_image_vips_ce, SL("BLENDMODE_SOFT_LIGHT"),  "soft-light");
	zend_declare_class_constant_string(phalcon_image_vips_ce, SL("BLENDMODE_DIFFERENCE"),  "difference");
	zend_declare_class_constant_string(phalcon_image_vips_ce, SL("BLENDMODE_EXCLUSION"),  "exclusion");

	if (strcmp(sapi_module.name, "apache2handler") == 0) {
#ifdef VIPS_SONAME
		if (!dlopen(VIPS_SONAME, RTLD_LAZY | RTLD_NODELETE)) 
#else /*!VIPS_SONAME*/
		if (!dlopen("libvips.so.42", RTLD_LAZY | RTLD_NODELETE)) 
#endif /*VIPS_SONAME*/
		{
			sapi_module.sapi_error(E_WARNING, "unable to lock libvips -- graceful may be unreliable");
		}
	}
	// VIPSHOME
	if (PHALCON_GLOBAL(vips).home) {
		if (VIPS_INIT(PHALCON_GLOBAL(vips).home)) {
#ifdef VIPS_DEBUG
			printf( "VIPS_INIT failed\n" );
#endif /*VIPS_DEBUG*/
			return FAILURE;
		}
	} else {
		if (VIPS_INIT(PHP_PHALCON_EXTNAME)) {
#ifdef VIPS_DEBUG
			printf( "VIPS_INIT failed\n" );
#endif /*VIPS_DEBUG*/
			return FAILURE;
		}
	}

	le_vips_gobject = zend_register_list_destructors_ex(phalcon_vips_free_gobject, NULL, phalcon_image_vips_type_name, module_number);
#ifdef VIPS_DEBUG
	printf( "enabling vips leak testing ...\n" );
	vips_leak_set( TRUE ); 
#endif /*VIPS_DEBUG*/

	return SUCCESS;
}

/**
 * Wrap a Image around an underlying vips resource.
 *
 * Don't call this yourself, users should stick to (for example)
 *
 * @param resource $image The underlying vips image resource that this class should wrap.
 *
 * @internal
 */
PHP_METHOD(Phalcon_Image_Vips, __construct)
{
	zval *image;
	const char *resource_type;
	int len = strlen(phalcon_image_vips_type_name), len2 = 0;

	phalcon_fetch_params(0, 1, 0, &image);

	if (Z_TYPE_P(image) != IS_RESOURCE) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "image parameter should be a resource");
		return;
	}
	resource_type = zend_rsrc_list_get_rsrc_type(Z_RES_P(image));
	len2 = resource_type ? strlen(resource_type) : 0;
	len = len > len2 ? len2 : len;
	if (!resource_type || memcmp(resource_type, phalcon_image_vips_type_name, len) != 0) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "image parameter not vips image");
		return;
	}

	phalcon_update_property(getThis(), SL("_image"), image);
}

/**
 * Return vips image resource
 *
 * @return resource
 */
PHP_METHOD(Phalcon_Image_Vips, getVipsImage)
{
	RETURN_MEMBER(getThis(), "_image");
}

/**
 * Get any property from the underlying image.
 *
 * @param string $name The property name.
 *
 * @throws Exception
 *
 * @return mixed
 */
PHP_METHOD(Phalcon_Image_Vips, __get)
{
	zval *property, image, ret = {};

	phalcon_fetch_params(0, 1, 0, &property);

	phalcon_read_property(&image, getThis(), SL("_image"), PH_NOISY|PH_READONLY);

	PHALCON_CALL_CE_STATIC(&ret, phalcon_image_vips_ce, "get", &image, property);
	if (!phalcon_array_isset_fetch_str(return_value, &ret, SL("out"), PH_COPY)) {
		RETURN_NULL();
	}
	zval_ptr_dtor(&ret);
}

/**
 * Set any property on the underlying image.
 *
 * @param string $name  The property name.
 * @param mixed  $value The value to set for this property.
 *
 * @return void
 */
PHP_METHOD(Phalcon_Image_Vips, __set)
{
	zval *property, *value, image;

	phalcon_fetch_params(0, 2, 0, &property, &value);

	phalcon_read_property(&image, getThis(), SL("_image"), PH_NOISY|PH_READONLY);

	PHALCON_CALL_CE_STATIC(return_value, phalcon_image_vips_ce, "set", &image, property, value);
}

/**
 * Check if the GType of a property from the underlying image exists.
 *
 * @param string $name The property name.
 *
 * @return bool
 */
PHP_METHOD(Phalcon_Image_Vips, __isset)
{
	zval *key, image, ret = {};

	phalcon_fetch_params(0, 1, 0, &key);

	phalcon_read_property(&image, getThis(), SL("_image"), PH_NOISY|PH_READONLY);

	PHALCON_CALL_CE_STATIC(&ret, phalcon_image_vips_ce, "get_typeof", &image, key);
	if (Z_TYPE(ret) == IS_LONG && Z_LVAL(ret) !=  0) {
		RETURN_TRUE;
	}
	RETURN_FALSE;
}

/**
 * Makes a string-ified version of the Image.
 *
 * @return string
 */
PHP_METHOD(Phalcon_Image_Vips, __toString){

	zval image, property = {}, value = {}, ret = {};

	PHALCON_MM_INIT();
	array_init(&ret);
	PHALCON_MM_ADD_ENTRY(&ret);
	phalcon_read_property(&image, getThis(), SL("_image"), PH_NOISY|PH_READONLY);

	PHALCON_MM_ZVAL_STRING(&property, "width");
	PHALCON_CALL_METHOD(&value, getThis(), "__get", &property);
	phalcon_array_update(&ret, &property, &value, 0);

	PHALCON_MM_ZVAL_STRING(&property, "height");
	PHALCON_CALL_METHOD(&value, getThis(), "__get", &property);
	phalcon_array_update(&ret, &property, &value, 0);

	PHALCON_MM_ZVAL_STRING(&property, "bands");
	PHALCON_CALL_METHOD(&value, getThis(), "__get", &property);
	phalcon_array_update(&ret, &property, &value, 0);

	PHALCON_MM_ZVAL_STRING(&property, "format");
	PHALCON_CALL_METHOD(&value, getThis(), "__get", &property);
	phalcon_array_update(&ret, &property, &value, 0);

	PHALCON_MM_ZVAL_STRING(&property, "interpretation");
	PHALCON_CALL_METHOD(&value, getThis(), "__get", &property);
	phalcon_array_update(&ret, &property, &value, 0);

	RETURN_MM_ON_FAILURE(phalcon_json_encode(return_value, &ret, 0));

	RETURN_MM();
}

/**
 * Make a 1D image where pixel values are indexes.
 *
 * If 'ushort' is TRUE, we make a 16-bit LUT, ie. 0 - 65535 values;
 * otherwise it's 8-bit (0 - 255)
 * <code>
 *  $lut = Phalcon\Image\Vips::identity(['ushort' => true]);
 * </code>
 */
PHP_METHOD(Phalcon_Image_Vips, identity)
{
	zval *options = NULL, *argv = NULL, ret = {}, image = {};
	int argc = 0, flag;

	phalcon_fetch_params(0, 0, 1, &options);

	if (options && Z_TYPE_P(options) == IS_ARRAY) {
		argc = 1;
		argv = (zval *)emalloc(argc * sizeof(zval));
		ZVAL_COPY_VALUE(&argv[0], options);
	}

	if (vips_php_call_array("identity", NULL, "", argc, argv, &ret)) {
		if (argv) {
			efree(argv);
		}
		PHALCON_THROW_VIPS_EXCEPTION("Make a 1D image where pixel values are indexes failed (%s)");
		return;
	}
	if (argv) {
		efree(argv);
	}
	if (!phalcon_array_isset_fetch_str(&image, &ret, SL("out"), PH_READONLY)) {
		zval_ptr_dtor(&ret);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Make a 1D image where pixel values are indexes failed");
		return;
	}

	object_init_ex(return_value, phalcon_image_vips_ce);
	PHALCON_CALL_METHOD_FLAG(flag, NULL, return_value, "__construct", &image);
	zval_ptr_dtor(&ret);
	if (flag != SUCCESS) {
		zval_ptr_dtor(return_value);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Make a 1D image where pixel values are indexes failed");
		return;
	}
}

/**
 * Make a black image
 */
PHP_METHOD(Phalcon_Image_Vips, black)
{
	zval *width, *height, *options = NULL, *argv, ret = {}, image = {};
	int argc = 2, flag;

	phalcon_fetch_params(0, 2, 1, &width, &height, &options);

	if (options && Z_TYPE_P(options) == IS_ARRAY) {
		argc = 3;
	}

	argv = (zval *)emalloc(argc * sizeof(zval));
	ZVAL_COPY_VALUE(&argv[0], width);
	ZVAL_COPY_VALUE(&argv[1], height);
	if (argc == 3) {
		ZVAL_COPY_VALUE(&argv[2], options);
	}

	if (vips_php_call_array("black", NULL, "", argc, argv, &ret)) {
		efree(argv);
		PHALCON_THROW_VIPS_EXCEPTION("Make a black image failed (%s)");
		return;
	}
	efree(argv);
	if (!phalcon_array_isset_fetch_str(&image, &ret, SL("out"), PH_READONLY)) {
		zval_ptr_dtor(&ret);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Make a black image failed");
		return;
	}

	object_init_ex(return_value, phalcon_image_vips_ce);
	PHALCON_CALL_METHOD_FLAG(flag, NULL, return_value, "__construct", &image);
	zval_ptr_dtor(&ret);
	if (flag != SUCCESS) {
		zval_ptr_dtor(return_value);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Make a black image failed");
		return;
	}
}

/**
 * Make a text image
 */
PHP_METHOD(Phalcon_Image_Vips, text)
{
	zval *text, *options = NULL, *argv, ret = {}, image = {};
	int argc = 1, flag;

	phalcon_fetch_params(0, 1, 1, &text, &options);

	if (options && Z_TYPE_P(options) == IS_ARRAY) {
		argc = 2;
	}

	argv = (zval *)emalloc(argc * sizeof(zval));
	ZVAL_COPY_VALUE(&argv[0], text);
	if (argc == 2) {
		ZVAL_COPY_VALUE(&argv[1], options);
	}

	if (vips_php_call_array("text", NULL, "", argc, argv, &ret)) {
		efree(argv);
		PHALCON_THROW_VIPS_EXCEPTION("make a text image failed (%s)");
		return;
	}
	efree(argv);
	if (!phalcon_array_isset_fetch_str(&image, &ret, SL("out"), PH_READONLY)) {
		zval_ptr_dtor(&ret);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "make a text image failed");
		return;
	}

	object_init_ex(return_value, phalcon_image_vips_ce);
	PHALCON_CALL_METHOD_FLAG(flag, NULL, return_value, "__construct", &image);
	zval_ptr_dtor(&ret);
	if (flag != SUCCESS) {
		zval_ptr_dtor(return_value);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "make a text image failed");
		return;
	}
}

/**
 * Create a two-band uint32 image
 */
PHP_METHOD(Phalcon_Image_Vips, xyz)
{
	zval *width, *height, *options = NULL, *argv, ret = {}, image = {};
	int argc = 2, flag;

	phalcon_fetch_params(0, 2, 1, &width, &height, &options);

	if (options && Z_TYPE_P(options) == IS_ARRAY) {
		argc = 3;
	}

	argv = (zval *)emalloc(argc * sizeof(zval));
	ZVAL_COPY_VALUE(&argv[0], width);
	ZVAL_COPY_VALUE(&argv[0], height);
	if (argc == 3) {
		ZVAL_COPY_VALUE(&argv[1], options);
	}

	if (vips_php_call_array("xyz", NULL, "", argc, argv, &ret)) {
		efree(argv);
		PHALCON_THROW_VIPS_EXCEPTION("Create a two-band uint32 image failed (%s)");
		return;
	}
	efree(argv);
	if (!phalcon_array_isset_fetch_str(&image, &ret, SL("out"), PH_READONLY)) {
		zval_ptr_dtor(&ret);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Create a two-band uint32 image failed");
		return;
	}

	object_init_ex(return_value, phalcon_image_vips_ce);
	PHALCON_CALL_METHOD_FLAG(flag, NULL, return_value, "__construct", &image);
	zval_ptr_dtor(&ret);
	if (flag != SUCCESS) {
		zval_ptr_dtor(return_value);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Create a two-band uint32 image failed");
		return;
	}
}

/**
 * Create a one-band float image
 */
PHP_METHOD(Phalcon_Image_Vips, grey)
{
	zval *width, *height, *options = NULL, *argv, ret = {}, image = {};
	int argc = 2, flag;

	phalcon_fetch_params(0, 2, 1, &width, &height, &options);

	if (options && Z_TYPE_P(options) == IS_ARRAY) {
		argc = 3;
	}

	argv = (zval *)emalloc(argc * sizeof(zval));
	ZVAL_COPY_VALUE(&argv[0], width);
	ZVAL_COPY_VALUE(&argv[0], height);
	if (argc == 3) {
		ZVAL_COPY_VALUE(&argv[1], options);
	}

	if (vips_php_call_array("grey", NULL, "", argc, argv, &ret)) {
		efree(argv);
		PHALCON_THROW_VIPS_EXCEPTION("Create a one-band float image failed (%s)");
		return;
	}
	efree(argv);
	if (!phalcon_array_isset_fetch_str(&image, &ret, SL("out"), PH_READONLY)) {
		zval_ptr_dtor(&ret);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Create a one-band float image failed");
		return;
	}

	object_init_ex(return_value, phalcon_image_vips_ce);
	PHALCON_CALL_METHOD_FLAG(flag, NULL, return_value, "__construct", &image);
	zval_ptr_dtor(&ret);
	if (flag != SUCCESS) {
		zval_ptr_dtor(return_value);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Create a one-band float image failed");
		return;
	}
}

/**
 * Creates a circularly symmetric Gaussian image
 */
PHP_METHOD(Phalcon_Image_Vips, gaussmat)
{
	zval *sigma, *min_ampl, *options = NULL, *argv, ret = {}, image = {};
	int argc = 2, flag;

	phalcon_fetch_params(0, 2, 1, &sigma, &min_ampl, &options);

	if (options && Z_TYPE_P(options) == IS_ARRAY) {
		argc = 3;
	}

	argv = (zval *)emalloc(argc * sizeof(zval));
	ZVAL_COPY_VALUE(&argv[0], sigma);
	ZVAL_COPY_VALUE(&argv[0], min_ampl);
	if (argc == 3) {
		ZVAL_COPY_VALUE(&argv[1], options);
	}

	if (vips_php_call_array("gaussmat", NULL, "", argc, argv, &ret)) {
		efree(argv);
		PHALCON_THROW_VIPS_EXCEPTION("Creates a circularly symmetric Gaussian image failed (%s)");
		return;
	}
	efree(argv);
	if (!phalcon_array_isset_fetch_str(&image, &ret, SL("out"), PH_READONLY)) {
		zval_ptr_dtor(&ret);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Creates a circularly symmetric Gaussian image failed");
		return;
	}

	object_init_ex(return_value, phalcon_image_vips_ce);
	PHALCON_CALL_METHOD_FLAG(flag, NULL, return_value, "__construct", &image);
	zval_ptr_dtor(&ret);
	if (flag != SUCCESS) {
		zval_ptr_dtor(return_value);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Creates a circularly symmetric Gaussian image failed");
		return;
	}
}

/**
 * Creates a circularly symmetric Laplacian image
 */
PHP_METHOD(Phalcon_Image_Vips, logmat)
{
	zval *sigma, *min_ampl, *options = NULL, *argv, ret = {}, image = {};
	int argc = 2, flag;

	phalcon_fetch_params(0, 2, 1, &sigma, &min_ampl, &options);

	if (options && Z_TYPE_P(options) == IS_ARRAY) {
		argc = 3;
	}

	argv = (zval *)emalloc(argc * sizeof(zval));
	ZVAL_COPY_VALUE(&argv[0], sigma);
	ZVAL_COPY_VALUE(&argv[0], min_ampl);
	if (argc == 3) {
		ZVAL_COPY_VALUE(&argv[1], options);
	}

	if (vips_php_call_array("logmat", NULL, "", argc, argv, &ret)) {
		efree(argv);
		PHALCON_THROW_VIPS_EXCEPTION("Creates a circularly symmetric Laplacian image failed (%s)");
		return;
	}
	efree(argv);
	if (!phalcon_array_isset_fetch_str(&image, &ret, SL("out"), PH_READONLY)) {
		zval_ptr_dtor(&ret);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Creates a circularly symmetric Laplacian image failed");
		return;
	}

	object_init_ex(return_value, phalcon_image_vips_ce);
	PHALCON_CALL_METHOD_FLAG(flag, NULL, return_value, "__construct", &image);
	zval_ptr_dtor(&ret);
	if (flag != SUCCESS) {
		zval_ptr_dtor(return_value);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Creates a circularly symmetric Laplacian image failed");
		return;
	}
}

/**
 * Create a new Image from a php array.
 *
 * 2D arrays become 2D images. 1D arrays become 2D images with height 1.
 *
 * @param array $array  The array to make the image from.
 * @param float $scale  The "scale" metadata item. Useful for integer convolution masks.
 * @param float $offset The "offset" metadata item. Useful for integer convolution masks.
 *
 * @throws Exception
 *
 * @return Image A new Image.
 */
PHP_METHOD(Phalcon_Image_Vips, newFromArray)
{
	zval *coefficients, *scale = NULL, *offset = NULL, image = {};
	int flag;
	phalcon_fetch_params(0, 1, 2, &coefficients, &scale, &offset);

	if (!scale) {
		scale = &PHALCON_GLOBAL(z_null);
	}

	if (!offset) {
		offset = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_CALL_CE_STATIC_FLAG(flag, &image, phalcon_image_vips_ce, "new_from_array", coefficients, scale, offset);
	if (flag != SUCCESS) {
		RETURN_FALSE;
	}
	object_init_ex(return_value, phalcon_image_vips_ce);
	PHALCON_CALL_METHOD_FLAG(flag, NULL, return_value, "__construct", &image);
	zval_ptr_dtor(&image);
	if (flag != SUCCESS) {
		zval_ptr_dtor(return_value);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Create a new Image from a php array failed");
		return;
	}
}

/**
 * Create a new Image from a compressed image held as a string.
 *
 * @param string $buffer        The formatted image to open.
 * @param string $option_string Any text-style options to pass to the
 *     selected loader.
 * @param array  $options       Any options to pass on to the load operation.
 *
 * @throws Exception
 *
 * @return Image A new Image.
 */
PHP_METHOD(Phalcon_Image_Vips, newFromBuffer)
{
	zval *buffer, *option_string = NULL, *options = NULL, ret = {}, image = {};
	int flag;
	phalcon_fetch_params(0, 1, 2, &buffer, &option_string, &options);

	if (!option_string) {
		option_string = &PHALCON_GLOBAL(z_null);
	}

	if (!options) {
		options = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_CALL_CE_STATIC_FLAG(flag, &ret, phalcon_image_vips_ce, "new_from_buffer", buffer, option_string, options);
	if (flag != SUCCESS) {
		RETURN_FALSE;
	}
	if (!phalcon_array_isset_fetch_str(&image, &ret, SL("out"), PH_READONLY)) {
		zval_ptr_dtor(&ret);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Create a new Image from a compressed image held as a string failed");
		return;
	}
	object_init_ex(return_value, phalcon_image_vips_ce);
	PHALCON_CALL_METHOD_FLAG(flag, NULL, return_value, "__construct", &image);
	zval_ptr_dtor(&ret);
	if (flag != SUCCESS) {
		zval_ptr_dtor(return_value);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Create a new Image from a compressed image held as a string failed");
		return;
	}
}

/**
 * Wraps an Image around an area of memory containing a C-style array.
 *
 * @param string $data   C-style array.
 * @param int    $width  Image width in pixels.
 * @param int    $height Image height in pixels.
 * @param int    $bands  Number of bands.
 * @param string $format Band format. (@see BandFormat)
 *
 * @throws Exception
 *
 * @return Image A new Image.
 */
PHP_METHOD(Phalcon_Image_Vips, newFromMemory)
{
	zval *data, *width, *height, *bands, *format, ret = {}, image = {};
	int flag;
	phalcon_fetch_params(0, 5, 0, &data, &width, &height, &bands, &format);

	PHALCON_CALL_CE_STATIC_FLAG(flag, &ret, phalcon_image_vips_ce, "new_from_memory", data, width, height, bands, format);
	if (flag != SUCCESS) {
		RETURN_FALSE;
	}
	if (!phalcon_array_isset_fetch_str(&image, &ret, SL("out"), PH_READONLY)) {
		zval_ptr_dtor(&ret);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Create a new Image from a compressed image held as a string failed");
		return;
	}
	object_init_ex(return_value, phalcon_image_vips_ce);
	PHALCON_CALL_METHOD_FLAG(flag, NULL, return_value, "__construct", &image);
	zval_ptr_dtor(&ret);
	if (flag != SUCCESS) {
		zval_ptr_dtor(return_value);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Create a new Image from a compressed image held as a string failed");
		return;
	}
}

/**
 * Create a new Image from a file on disc.
 *
 * @param string $filename The file to open.
 * @param array  $options  Any options to pass on to the load operation.
 *
 * @throws Exception
 *
 * @return Image A new Image.
 */
PHP_METHOD(Phalcon_Image_Vips, newFromFile)
{
	zval *filename, *options = NULL, ret = {}, image = {};
	int flag;
	phalcon_fetch_params(0, 1, 1, &filename, &options);

	if (!options) {
		options = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_CALL_CE_STATIC_FLAG(flag, &ret, phalcon_image_vips_ce, "new_from_file", filename, options);
	if (flag != SUCCESS) {
		RETURN_FALSE;
	}
	if (!phalcon_array_isset_fetch_str(&image, &ret, SL("out"), PH_READONLY)) {
		zval_ptr_dtor(&ret);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Create a new Image from a file on disc failed");
		return;
	}
	object_init_ex(return_value, phalcon_image_vips_ce);
	PHALCON_CALL_METHOD_FLAG(flag, NULL, return_value, "__construct", &image);
	zval_ptr_dtor(&ret);
	if (flag != SUCCESS) {
		zval_ptr_dtor(return_value);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Create a new Image from a file on disc failed");
		return;
	}
}

/**
 * Extract an area from an image
 */
PHP_METHOD(Phalcon_Image_Vips, crop)
{
	zval *left, *top, *width, *height, *options = NULL, image = {}, *argv, ret = {}, image2 = {};
	int argc = 5, flag;

	phalcon_fetch_params(0, 4, 1, &left, &top, &width, &height, &options);

	if (options && Z_TYPE_P(options) == IS_ARRAY) {
		argc = 6;
	}

	phalcon_read_property(&image, getThis(), SL("_image"), PH_NOISY|PH_READONLY);

	argv = (zval *)emalloc(argc * sizeof(zval));
	ZVAL_COPY_VALUE(&argv[0], &image);
	ZVAL_COPY_VALUE(&argv[1], left);
	ZVAL_COPY_VALUE(&argv[2], top);
	ZVAL_COPY_VALUE(&argv[3], width);
	ZVAL_COPY_VALUE(&argv[4], height);
	if (argc == 6) {
		ZVAL_COPY_VALUE(&argv[5], options);
	}
	if (vips_php_call_array("crop", NULL, "", argc, argv, &ret)) {
		efree(argv);
		PHALCON_THROW_VIPS_EXCEPTION("Extract an area from an image failed (%s)");
	}
	efree(argv);
	if (!phalcon_array_isset_fetch_str(&image2, &ret, SL("out"), PH_READONLY)) {
		zval_ptr_dtor(&ret);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Extract an area from an image failed");
		return;
	}

	object_init_ex(return_value, phalcon_image_vips_ce);
	PHALCON_CALL_METHOD_FLAG(flag, NULL, return_value, "__construct", &image2);
	zval_ptr_dtor(&ret);
	if (flag != SUCCESS) {
		zval_ptr_dtor(return_value);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Extract an area from an image failed");
		return;
	}
}

/**
 * Extract an area from an image
 */
PHP_METHOD(Phalcon_Image_Vips, smartcrop)
{
	zval *width, *height, *options = NULL, image = {}, *argv, ret = {}, image2 = {};
	int argc = 3, flag;

	phalcon_fetch_params(0, 2, 1, &width, &height, &options);

	if (options && Z_TYPE_P(options) == IS_ARRAY) {
		argc = 4;
	}

	phalcon_read_property(&image, getThis(), SL("_image"), PH_NOISY|PH_READONLY);

	argv = (zval *)emalloc(argc * sizeof(zval));
	ZVAL_COPY_VALUE(&argv[0], &image);
	ZVAL_COPY_VALUE(&argv[1], width);
	ZVAL_COPY_VALUE(&argv[2], height);
	if (argc == 4) {
		ZVAL_COPY_VALUE(&argv[3], options);
	}
	if (vips_php_call_array("smartcrop", NULL, "", argc, argv, &ret)) {
		efree(argv);
		
		PHALCON_THROW_VIPS_EXCEPTION("Extract an area from an image failed (%s)");
	}
	efree(argv);
	if (!phalcon_array_isset_fetch_str(&image2, &ret, SL("out"), PH_READONLY)) {
		zval_ptr_dtor(&ret);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Extract an area from an image failed");
		return;
	}

	object_init_ex(return_value, phalcon_image_vips_ce);
	PHALCON_CALL_METHOD_FLAG(flag, NULL, return_value, "__construct", &image2);
	zval_ptr_dtor(&ret);
	if (flag != SUCCESS) {
		zval_ptr_dtor(return_value);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Extract an area from an image failed");
		return;
	}
}

/**
 * Resize an image
 */
PHP_METHOD(Phalcon_Image_Vips, resize)
{
	zval *scale, *options = NULL, image = {}, *argv, ret = {}, image2 = {};
	int argc = 2, flag;

	phalcon_fetch_params(0, 1, 1, &scale, &options);

	if (options && Z_TYPE_P(options) == IS_ARRAY) {
		argc = 3;
	}

	phalcon_read_property(&image, getThis(), SL("_image"), PH_NOISY|PH_READONLY);

	argv = (zval *)emalloc(argc * sizeof(zval));
	ZVAL_COPY_VALUE(&argv[0], &image);
	ZVAL_COPY_VALUE(&argv[1], scale);
	if (argc == 3) {
		ZVAL_COPY_VALUE(&argv[2], options);
	}
	if (vips_php_call_array("resize", NULL, "", argc, argv, &ret)) {
		efree(argv);
		PHALCON_THROW_VIPS_EXCEPTION("Resize an image failed (%s)");
	}
	efree(argv);
	if (!phalcon_array_isset_fetch_str(&image2, &ret, SL("out"), PH_READONLY)) {
		zval_ptr_dtor(&ret);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Resize an image failed");
		return;
	}

	object_init_ex(return_value, phalcon_image_vips_ce);
	PHALCON_CALL_METHOD_FLAG(flag, NULL, return_value, "__construct", &image2);
	zval_ptr_dtor(&ret);
	if (flag != SUCCESS) {
		zval_ptr_dtor(return_value);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Resize an image failed");
		return;
	}
}

/**
 * Rotate an image by a number of degrees
 */
PHP_METHOD(Phalcon_Image_Vips, rotate)
{
	zval *angle, *options = NULL, image = {}, *argv, ret = {}, image2 = {};
	int argc = 2, flag;

	phalcon_fetch_params(0, 1, 1, &angle, &options);

	if (options && Z_TYPE_P(options) == IS_ARRAY) {
		argc = 3;
	}

	phalcon_read_property(&image, getThis(), SL("_image"), PH_NOISY|PH_READONLY);

	argv = (zval *)emalloc(argc * sizeof(zval));
	ZVAL_COPY_VALUE(&argv[0], &image);
	ZVAL_COPY_VALUE(&argv[1], angle);
	if (argc == 3) {
		ZVAL_COPY_VALUE(&argv[2], options);
	}
	if (vips_php_call_array("rotate", NULL, "", argc, argv, &ret)) {
		efree(argv);

		PHALCON_THROW_VIPS_EXCEPTION("Rotate an image by a number of degrees failed (%s)");
	}
	efree(argv);
	if (!phalcon_array_isset_fetch_str(&image2, &ret, SL("out"), PH_READONLY)) {
		zval_ptr_dtor(&ret);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Rotate an image by a number of degrees failed");
		return;
	}

	object_init_ex(return_value, phalcon_image_vips_ce);
	PHALCON_CALL_METHOD_FLAG(flag, NULL, return_value, "__construct", &image2);
	zval_ptr_dtor(&ret);
	if (flag != SUCCESS) {
		zval_ptr_dtor(return_value);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Rotate an image by a number of degrees failed");
		return;
	}
}

/**
 * Composite $other on top of $this with $mode.
 *
 * @param array $overlays
 * @param array $modes
 */
PHP_METHOD(Phalcon_Image_Vips, composite)
{
	zval *overlays, *modes, *options = NULL, image = {}, *overlay = NULL, overlayimages = {};
	zval *argv, ret = {}, image2 = {};
	int argc = 3, image_num = 1, flag;

	phalcon_fetch_params(1, 2, 1, &overlays, &modes, &options);

	if (options && Z_TYPE_P(options) == IS_ARRAY) {
		argc = 4;
	}

	array_init(&overlayimages);
	PHALCON_MM_ADD_ENTRY(&overlayimages);

	phalcon_read_property(&image, getThis(), SL("_image"), PH_NOISY|PH_READONLY);
	phalcon_array_append(&overlayimages, &image, PH_COPY);
	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(overlays), overlay) {
		zval overlayimage = {};
		PHALCON_MM_VERIFY_CLASS_EX(overlay, phalcon_image_vips_ce, phalcon_image_exception_ce);
		
		phalcon_read_property(&overlayimage, overlay, SL("_image"), PH_NOISY|PH_READONLY);
		phalcon_array_append(&overlayimages, &overlayimage, PH_COPY);
		image_num++;
	} ZEND_HASH_FOREACH_END();

	argv = (zval *)emalloc(argc * sizeof(zval));
	ZVAL_COPY_VALUE(&argv[0], &overlayimages);
	ZVAL_LONG(&argv[1], image_num);
	ZVAL_COPY_VALUE(&argv[2], modes);
	if (argc == 4) {
		ZVAL_COPY_VALUE(&argv[3], options);
	}
	if (vips_php_call_array("composite", NULL, "", argc, argv, &ret)) {
		efree(argv);

		PHALCON_MM_THROW_VIPS_EXCEPTION("Composite an image failed (%s)");
	}
	efree(argv);
	PHALCON_MM_ADD_ENTRY(&ret);
	if (!phalcon_array_isset_fetch_str(&image2, &ret, SL("out"), PH_READONLY)) {
		zval_ptr_dtor(&ret);
		PHALCON_MM_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Composite an image failed");
		return;
	}

	object_init_ex(return_value, phalcon_image_vips_ce);
	PHALCON_CALL_METHOD_FLAG(flag, NULL, return_value, "__construct", &image2);
	if (flag != SUCCESS) {
		zval_ptr_dtor(return_value);
		PHALCON_MM_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Composite an image failed");
		return;
	}
	RETURN_MM();
}

/**
 * Composite $other on top of $this with $mode.
 */
PHP_METHOD(Phalcon_Image_Vips, composite2)
{
	zval *overlay, *mode, *options = NULL, image = {}, overlayimage = {}, *argv, ret = {}, image2 = {};
	int argc = 3, flag;

	phalcon_fetch_params(0, 2, 1, &overlay, &mode, &options);

	PHALCON_VERIFY_CLASS_EX(overlay, phalcon_image_vips_ce, phalcon_image_exception_ce);

	if (options && Z_TYPE_P(options) == IS_ARRAY) {
		argc = 4;
	}

	phalcon_read_property(&image, getThis(), SL("_image"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&overlayimage, overlay, SL("_image"), PH_NOISY|PH_READONLY);

	argv = (zval *)emalloc(argc * sizeof(zval));
	ZVAL_COPY_VALUE(&argv[0], &image);
	ZVAL_COPY_VALUE(&argv[1], &overlayimage);
	ZVAL_COPY_VALUE(&argv[2], mode);
	if (argc == 4) {
		ZVAL_COPY_VALUE(&argv[3], options);
	}
	if (vips_php_call_array("composite2", NULL, "", argc, argv, &ret)) {
		efree(argv);

		PHALCON_THROW_VIPS_EXCEPTION("Composite an image failed (%s)");
	}
	efree(argv);
	if (!phalcon_array_isset_fetch_str(&image2, &ret, SL("out"), PH_READONLY)) {
		zval_ptr_dtor(&ret);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Composite an image failed");
		return;
	}

	object_init_ex(return_value, phalcon_image_vips_ce);
	PHALCON_CALL_METHOD_FLAG(flag, NULL, return_value, "__construct", &image2);
	zval_ptr_dtor(&ret);
	if (flag != SUCCESS) {
		zval_ptr_dtor(return_value);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Composite an image failed");
		return;
	}
}

/**
 * Embed an image in a larger image
 */
PHP_METHOD(Phalcon_Image_Vips, embed)
{
	zval *x, *y, *width, *height, image = {}, *argv, ret = {}, image2 = {};
	int argc = 5, flag;

	phalcon_fetch_params(0, 4, 0, &x, &y, &width, &height);

	phalcon_read_property(&image, getThis(), SL("_image"), PH_NOISY|PH_READONLY);

	argv = (zval *)emalloc(argc * sizeof(zval));
	ZVAL_COPY_VALUE(&argv[0], &image);
	ZVAL_COPY_VALUE(&argv[1], x);
	ZVAL_COPY_VALUE(&argv[2], y);
	ZVAL_COPY_VALUE(&argv[3], width);
	ZVAL_COPY_VALUE(&argv[4], height);

	if (vips_php_call_array("embed", NULL, "", argc, argv, &ret)) {
		efree(argv);
		PHALCON_THROW_VIPS_EXCEPTION("embed an image in a larger image failed (%s)");
		return;
	}
	efree(argv);
	if (!phalcon_array_isset_fetch_str(&image2, &ret, SL("out"), PH_READONLY)) {
		zval_ptr_dtor(&ret);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "embed an image in a larger image failed");
		return;
	}

	object_init_ex(return_value, phalcon_image_vips_ce);
	PHALCON_CALL_METHOD_FLAG(flag, NULL, return_value, "__construct", &image2);
	zval_ptr_dtor(&ret);
	if (flag != SUCCESS) {
		zval_ptr_dtor(return_value);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "embed an image in a larger image failed");
		return;
	}
}

/**
 * Use $this as a condition image to pick pixels from either $then or $else.
 *
 * @param mixed $then    The true side of the operator
 * @param mixed $else    The false side of the operator.
 * @param array $options An array of options to pass to the operation.
 *
 * @throws Exception
 *
 * @return Image A new image.
 */
PHP_METHOD(Phalcon_Image_Vips, ifthenelse)
{
	zval *condthen, *condelse, *options = NULL, image = {}, *argv, ret = {}, thenimage = {}, elseimage = {}, image2 = {};
	int argc = 3, flag;

	phalcon_fetch_params(0, 2, 1, &condthen, &condelse, &options);

	PHALCON_VERIFY_CLASS_EX(condthen, phalcon_image_vips_ce, phalcon_image_exception_ce);
	PHALCON_VERIFY_CLASS_EX(condelse, phalcon_image_vips_ce, phalcon_image_exception_ce);

	phalcon_read_property(&image, getThis(), SL("_image"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&thenimage, condthen, SL("_image"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&elseimage, condelse, SL("_image"), PH_NOISY|PH_READONLY);

	if (options && Z_TYPE_P(options) == IS_ARRAY) {
		argc = 4;
	}

	argv = (zval *)emalloc(argc * sizeof(zval));
	ZVAL_COPY_VALUE(&argv[0], &image);
	ZVAL_COPY_VALUE(&argv[1], &thenimage);
	ZVAL_COPY_VALUE(&argv[2], &elseimage);
	if (argc == 4) {
		ZVAL_COPY_VALUE(&argv[3], options);
	}

	if (vips_php_call_array("ifthenelse", NULL, "", argc, argv, &ret)) {
		efree(argv);
		PHALCON_THROW_VIPS_EXCEPTION("ifthenelse an image failed (%s)");
	}
	efree(argv);

	if (!phalcon_array_isset_fetch_str(&image2, &ret, SL("out"), PH_READONLY)) {
		zval_ptr_dtor(&ret);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "ifthenelse an image failed");
		return;
	}
	object_init_ex(return_value, phalcon_image_vips_ce);
	PHALCON_CALL_METHOD_FLAG(flag, NULL, return_value, "__construct", &image2);
	zval_ptr_dtor(&ret);
	if (flag != SUCCESS) {
		zval_ptr_dtor(return_value);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "ifthenelse an image failed");
		return;
	}
}

/**
 * Reads a single pixel on an image.
 *
 *@return array
 */
PHP_METHOD(Phalcon_Image_Vips, getpoint)
{
	zval *x, *y, image = {}, *argv, ret = {};
	int argc = 3;

	phalcon_fetch_params(0, 2, 0, &x, &y);

	phalcon_read_property(&image, getThis(), SL("_image"), PH_NOISY|PH_READONLY);

	argv = (zval *)emalloc(argc * sizeof(zval));
	ZVAL_COPY_VALUE(&argv[0], &image);
	ZVAL_COPY_VALUE(&argv[1], x);
	ZVAL_COPY_VALUE(&argv[2], y);

	if (vips_php_call_array("getpoint", NULL, "", argc, argv, &ret)) {
		efree(argv);
		PHALCON_THROW_VIPS_EXCEPTION("paint an image into another image failed (%s)");
		return;
	}
	efree(argv);
	if (!phalcon_array_isset_fetch_str(return_value, &ret, SL("out-array"), PH_COPY)) {
		zval_ptr_dtor(&ret);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "paint an image into another image failed");
		return;
	}
	zval_ptr_dtor(&ret);
}

/**
 * Perform various relational operations on pairs of images. 
 *
 * @param Phalcon\Image\Vips $other	The right-hand side of the operator.
 * @param string $relational
 *
 * @throws Exception
 *
 * @return Phalcon\Image\Vips
 */
PHP_METHOD(Phalcon_Image_Vips, relational)
{
	zval *right, *op, image = {}, *argv, ret = {}, rightimage = {}, image2 = {};
	int argc = 3, flag;

	phalcon_fetch_params(0, 2, 0, &right, &op);

	PHALCON_VERIFY_CLASS_EX(right, phalcon_image_vips_ce, phalcon_image_exception_ce);

	phalcon_read_property(&image, getThis(), SL("_image"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&rightimage, right, SL("_image"), PH_NOISY|PH_READONLY);

	argv = (zval *)emalloc(argc * sizeof(zval));
	ZVAL_COPY_VALUE(&argv[0], &image);
	ZVAL_COPY_VALUE(&argv[1], &rightimage);
	ZVAL_COPY_VALUE(&argv[2], op);

	if (vips_php_call_array("relational", NULL, "", argc, argv, &ret)) {
		efree(argv);
		PHALCON_THROW_VIPS_EXCEPTION("relational operation on two images failed (%s)");
		return;
	}
	efree(argv);

	if (!phalcon_array_isset_fetch_str(&image2, &ret, SL("out"), PH_READONLY)) {
		zval_ptr_dtor(&ret);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "relational operation on two images failed");
		return;
	}
	object_init_ex(return_value, phalcon_image_vips_ce);
	PHALCON_CALL_METHOD_FLAG(flag, NULL, return_value, "__construct", &image2);
	zval_ptr_dtor(&ret);
	if (flag != SUCCESS) {
		zval_ptr_dtor(return_value);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "relational operation on two images failed");
		return;
	}
}

PHP_METHOD(Phalcon_Image_Vips, relationalConst)
{
	zval *right, *op, image = {}, *argv, ret = {}, image2 = {};
	int argc = 3, flag;

	phalcon_fetch_params(0, 2, 0, &right, &op);

	phalcon_read_property(&image, getThis(), SL("_image"), PH_NOISY|PH_READONLY);

	argv = (zval *)emalloc(argc * sizeof(zval));
	ZVAL_COPY_VALUE(&argv[0], &image);
	ZVAL_COPY_VALUE(&argv[1], op);
	ZVAL_COPY_VALUE(&argv[2], right);

	if (vips_php_call_array("relational_const", NULL, "", argc, argv, &ret)) {
		efree(argv);
		PHALCON_THROW_VIPS_EXCEPTION("relational operation on two images failed (%s)");
		return;
	}
	efree(argv);

	if (!phalcon_array_isset_fetch_str(&image2, &ret, SL("out"), PH_READONLY)) {
		zval_ptr_dtor(&ret);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "relational operation on two images failed");
		return;
	}
	object_init_ex(return_value, phalcon_image_vips_ce);
	PHALCON_CALL_METHOD_FLAG(flag, NULL, return_value, "__construct", &image2);
	zval_ptr_dtor(&ret);
	if (flag != SUCCESS) {
		zval_ptr_dtor(return_value);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "relational operation on two images failed");
		return;
	}
}

/**
 * Add $other to this image.
 *
 * @param mixed $other   The thing to add to this image.
 * @param array $options An array of options to pass to the operation.
 *
 * @throws Exception
 *
 * @return Image A new image.
 */
PHP_METHOD(Phalcon_Image_Vips, add)
{
	zval *other, *options = NULL, image = {}, *argv, otherimage = {}, ret = {}, image2 = {};
	int argc = 2, flag;

	phalcon_fetch_params(0, 1, 1, &other, &options);

	PHALCON_VERIFY_CLASS_EX(other, phalcon_image_vips_ce, phalcon_image_exception_ce);
	if (options && Z_TYPE_P(options) == IS_ARRAY) {
		argc = 3;
	}

	phalcon_read_property(&image, getThis(), SL("_image"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&otherimage, other, SL("_image"), PH_NOISY|PH_READONLY);

	argv = (zval *)emalloc(argc * sizeof(zval));
	ZVAL_COPY_VALUE(&argv[0], &image);
	ZVAL_COPY_VALUE(&argv[1], &otherimage);
	if (argc == 3) {
		ZVAL_COPY_VALUE(&argv[2], options);
	}

	if (vips_php_call_array("add", NULL, "", argc, argv, &ret)) {
		efree(argv);
		PHALCON_THROW_VIPS_EXCEPTION("Add $other to this image failed (%s)");
		return;
	}
	efree(argv);

	if (!phalcon_array_isset_fetch_str(&image2, &ret, SL("out"), PH_READONLY)) {
		zval_ptr_dtor(&ret);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Add $other to this image failed");
		return;
	}
	object_init_ex(return_value, phalcon_image_vips_ce);
	PHALCON_CALL_METHOD_FLAG(flag, NULL, return_value, "__construct", &image2);
	zval_ptr_dtor(&ret);
	if (flag != SUCCESS) {
		zval_ptr_dtor(return_value);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Add $other to this image failed");
		return;
	}
}

/**
 * Subtract $other from this image.
 *
 * @param mixed $other   The thing to subtract from this image.
 * @param array $options An array of options to pass to the operation.
 *
 * @throws Exception
 *
 * @return Image A new image.
 */
PHP_METHOD(Phalcon_Image_Vips, subtract)
{
	zval *other, *options = NULL, image = {}, *argv, otherimage = {}, ret = {}, image2 = {};
	int argc = 2, flag;

	phalcon_fetch_params(0, 1, 1, &other, &options);

	PHALCON_VERIFY_CLASS_EX(other, phalcon_image_vips_ce, phalcon_image_exception_ce);
	if (options && Z_TYPE_P(options) == IS_ARRAY) {
		argc = 3;
	}

	phalcon_read_property(&image, getThis(), SL("_image"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&otherimage, other, SL("_image"), PH_NOISY|PH_READONLY);

	argv = (zval *)emalloc(argc * sizeof(zval));
	ZVAL_COPY_VALUE(&argv[0], &image);
	ZVAL_COPY_VALUE(&argv[1], &otherimage);
	if (argc == 3) {
		ZVAL_COPY_VALUE(&argv[2], options);
	}

	if (vips_php_call_array("subtract", NULL, "", argc, argv, &ret)) {
		efree(argv);
		PHALCON_THROW_VIPS_EXCEPTION("Subtract $other to this image failed (%s)");
		return;
	}
	efree(argv);

	if (!phalcon_array_isset_fetch_str(&image2, &ret, SL("out"), PH_READONLY)) {
		zval_ptr_dtor(&ret);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Subtract $other to this image failed");
		return;
	}
	object_init_ex(return_value, phalcon_image_vips_ce);
	PHALCON_CALL_METHOD_FLAG(flag, NULL, return_value, "__construct", &image2);
	zval_ptr_dtor(&ret);
	if (flag != SUCCESS) {
		zval_ptr_dtor(return_value);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Subtract $other to this image failed");
		return;
	}
}

/**
 * Multiply this image by $other.
 *
 * @param mixed $other   The thing to multiply this image by.
 * @param array $options An array of options to pass to the operation.
 *
 * @throws Exception
 *
 * @return Image A new image.
 */
PHP_METHOD(Phalcon_Image_Vips, multiply)
{
	zval *other, *options = NULL, image = {}, *argv, otherimage = {}, ret = {}, image2 = {};
	int argc = 2, flag;

	phalcon_fetch_params(0, 1, 1, &other, &options);

	PHALCON_VERIFY_CLASS_EX(other, phalcon_image_vips_ce, phalcon_image_exception_ce);
	if (options && Z_TYPE_P(options) == IS_ARRAY) {
		argc = 3;
	}

	phalcon_read_property(&image, getThis(), SL("_image"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&otherimage, other, SL("_image"), PH_NOISY|PH_READONLY);

	argv = (zval *)emalloc(argc * sizeof(zval));
	ZVAL_COPY_VALUE(&argv[0], &image);
	ZVAL_COPY_VALUE(&argv[1], &otherimage);
	if (argc == 3) {
		ZVAL_COPY_VALUE(&argv[2], options);
	}

	if (vips_php_call_array("multiply", NULL, "", argc, argv, &ret)) {
		efree(argv);
		PHALCON_THROW_VIPS_EXCEPTION("Multiply this image by $other failed (%s)");
		return;
	}
	efree(argv);

	if (!phalcon_array_isset_fetch_str(&image2, &ret, SL("out"), PH_READONLY)) {
		zval_ptr_dtor(&ret);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Multiply this image by $other failed");
		return;
	}
	object_init_ex(return_value, phalcon_image_vips_ce);
	PHALCON_CALL_METHOD_FLAG(flag, NULL, return_value, "__construct", &image2);
	zval_ptr_dtor(&ret);
	if (flag != SUCCESS) {
		zval_ptr_dtor(return_value);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Multiply this image by $other failed");
		return;
	}
}

/**
 * Divide this image by $other.
 *
 * @param mixed $other   The thing to divide this image by.
 * @param array $options An array of options to pass to the operation.
 *
 * @throws Exception
 *
 * @return Image A new image.
 */
PHP_METHOD(Phalcon_Image_Vips, divide)
{
	zval *other, *options = NULL, image = {}, *argv, otherimage = {}, ret = {}, image2 = {};
	int argc = 2, flag;

	phalcon_fetch_params(0, 1, 1, &other, &options);

	PHALCON_VERIFY_CLASS_EX(other, phalcon_image_vips_ce, phalcon_image_exception_ce);
	if (options && Z_TYPE_P(options) == IS_ARRAY) {
		argc = 3;
	}

	phalcon_read_property(&image, getThis(), SL("_image"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&otherimage, other, SL("_image"), PH_NOISY|PH_READONLY);

	argv = (zval *)emalloc(argc * sizeof(zval));
	ZVAL_COPY_VALUE(&argv[0], &image);
	ZVAL_COPY_VALUE(&argv[1], &otherimage);
	if (argc == 3) {
		ZVAL_COPY_VALUE(&argv[2], options);
	}

	if (vips_php_call_array("divide", NULL, "", argc, argv, &ret)) {
		efree(argv);
		PHALCON_THROW_VIPS_EXCEPTION("Divide this image by $other failed (%s)");
		return;
	}
	efree(argv);

	if (!phalcon_array_isset_fetch_str(&image2, &ret, SL("out"), PH_READONLY)) {
		zval_ptr_dtor(&ret);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Divide this image by $other failed");
		return;
	}
	object_init_ex(return_value, phalcon_image_vips_ce);
	PHALCON_CALL_METHOD_FLAG(flag, NULL, return_value, "__construct", &image2);
	zval_ptr_dtor(&ret);
	if (flag != SUCCESS) {
		zval_ptr_dtor(return_value);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Divide this image by $other failed");
		return;
	}
}

/**
 * Calculate (a * in + b).
 */
PHP_METHOD(Phalcon_Image_Vips, linear)
{
	zval *a, *b, *options = NULL, image = {}, *argv, op_value = {}, ret = {}, image2 = {};
	int argc = 3, flag;

	phalcon_fetch_params(0, 2, 1, &a, &b, &options);

	if (Z_TYPE_P(a) != IS_LONG && Z_TYPE_P(a) != IS_DOUBLE && Z_TYPE_P(a) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "a parameter must be float or float[]");
		return;
	}

	if (Z_TYPE_P(b) != IS_LONG && Z_TYPE_P(b) != IS_DOUBLE && Z_TYPE_P(b) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "b parameter must be float or float[]");
		return;
	}

	if (options && Z_TYPE_P(options) == IS_ARRAY) {
		argc = 4;
	}

	phalcon_read_property(&image, getThis(), SL("_image"), PH_NOISY|PH_READONLY);

	argv = (zval *)emalloc(argc * sizeof(zval));
	ZVAL_COPY_VALUE(&argv[0], &image);
	ZVAL_COPY_VALUE(&argv[1], a);
	ZVAL_COPY_VALUE(&argv[2], b);
	if (argc == 4) {
		ZVAL_COPY_VALUE(&argv[3], options);
	}

	if (vips_php_call_array("linear", NULL, "", argc, argv, &ret)) {
		zval_ptr_dtor(&op_value);
		efree(argv);
		PHALCON_THROW_VIPS_EXCEPTION("calculate (a * in + b) failed (%s)");
		return;
	}
	zval_ptr_dtor(&op_value);
	efree(argv);

	if (!phalcon_array_isset_fetch_str(&image2, &ret, SL("out"), PH_READONLY)) {
		zval_ptr_dtor(&ret);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "calculate (a * in + b) failed");
		return;
	}
	object_init_ex(return_value, phalcon_image_vips_ce);
	PHALCON_CALL_METHOD_FLAG(flag, NULL, return_value, "__construct", &image2);
	zval_ptr_dtor(&ret);
	if (flag != SUCCESS) {
		zval_ptr_dtor(return_value);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "calculate (a * in + b) failed");
		return;
	}
}

/**
 * Find image maximum
 *
 * @param array $options An array of options to pass to the operation.
 *
 * @return float
 */
PHP_METHOD(Phalcon_Image_Vips, max)
{
	zval *options = NULL, image = {}, *argv, ret = {};
	int argc = 1;

	phalcon_fetch_params(0, 0, 1, &options);

	if (options && Z_TYPE_P(options) == IS_ARRAY) {
		argc = 2;
	}

	phalcon_read_property(&image, getThis(), SL("_image"), PH_NOISY|PH_READONLY);

	argv = (zval *)emalloc(argc * sizeof(zval));
	ZVAL_COPY_VALUE(&argv[0], &image);
	if (argc == 2) {
		ZVAL_COPY_VALUE(&argv[1], options);
	}

	if (vips_php_call_array("max", NULL, "", argc, argv, &ret)) {
		efree(argv);
		PHALCON_THROW_VIPS_EXCEPTION("Find image maximum failed (%s)");
		return;
	}
	efree(argv);

	if (!phalcon_array_isset_fetch_str(return_value, &ret, SL("out"), PH_READONLY)) {
		zval_ptr_dtor(&ret);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Find image maximum failed");
		return;
	}
	zval_ptr_dtor(&ret);
}

/**
 * Find image minimum
 *
 * @param array $options An array of options to pass to the operation.
 *
 * @return float
 */
PHP_METHOD(Phalcon_Image_Vips, min)
{
	zval *options = NULL, image = {}, *argv, ret = {};
	int argc = 1;

	phalcon_fetch_params(0, 0, 1, &options);

	if (options && Z_TYPE_P(options) == IS_ARRAY) {
		argc = 2;
	}

	phalcon_read_property(&image, getThis(), SL("_image"), PH_NOISY|PH_READONLY);

	argv = (zval *)emalloc(argc * sizeof(zval));
	ZVAL_COPY_VALUE(&argv[0], &image);
	if (argc == 2) {
		ZVAL_COPY_VALUE(&argv[1], options);
	}

	if (vips_php_call_array("max", NULL, "", argc, argv, &ret)) {
		efree(argv);
		PHALCON_THROW_VIPS_EXCEPTION("Find image minimum failed (%s)");
		return;
	}
	efree(argv);

	if (!phalcon_array_isset_fetch_str(return_value, &ret, SL("out"), PH_READONLY)) {
		zval_ptr_dtor(&ret);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Find image minimum failed");
		return;
	}
	zval_ptr_dtor(&ret);
}

/**
 * Apply a math operation to an image.
 */
PHP_METHOD(Phalcon_Image_Vips, math)
{
	zval *operation, *options = NULL, image = {}, *argv, ret = {}, image2 = {};
	int argc = 2, flag;

	phalcon_fetch_params(0, 1, 1, &operation, &options);

	if (options && Z_TYPE_P(options) == IS_ARRAY) {
		argc = 3;
	}

	phalcon_read_property(&image, getThis(), SL("_image"), PH_NOISY|PH_READONLY);

	argv = (zval *)emalloc(argc * sizeof(zval));
	ZVAL_COPY_VALUE(&argv[0], &image);
	ZVAL_COPY_VALUE(&argv[1], operation);
	if (argc == 3) {
		ZVAL_COPY_VALUE(&argv[2], options);
	}

	if (vips_php_call_array("math", NULL, "", argc, argv, &ret)) {
		efree(argv);
		PHALCON_THROW_VIPS_EXCEPTION("Apply a math operation to an image failed (%s)");
		return;
	}
	efree(argv);
	if (!phalcon_array_isset_fetch_str(&image2, &ret, SL("out"), PH_READONLY)) {
		zval_ptr_dtor(&ret);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Apply a math operation to an image failed");
		return;
	}

	object_init_ex(return_value, phalcon_image_vips_ce);
	PHALCON_CALL_METHOD_FLAG(flag, NULL, return_value, "__construct", &image2);
	zval_ptr_dtor(&ret);
	if (flag != SUCCESS) {
		zval_ptr_dtor(return_value);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Apply a math operation to an image failed");
		return;
	}
}

/**
 * Binary math operations.
 */
PHP_METHOD(Phalcon_Image_Vips, math2)
{
	zval *right, *operation, *options = NULL, image = {}, *argv, rightimage = {}, ret = {}, image2 = {};
	int argc = 3, flag;

	phalcon_fetch_params(0, 1, 1, &right, &operation, &options);

	if (options && Z_TYPE_P(options) == IS_ARRAY) {
		argc = 4;
	}

	phalcon_read_property(&image, getThis(), SL("_image"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&rightimage, right, SL("_image"), PH_NOISY|PH_READONLY);

	argv = (zval *)emalloc(argc * sizeof(zval));
	ZVAL_COPY_VALUE(&argv[0], &image);
	ZVAL_COPY_VALUE(&argv[1], &rightimage);
	ZVAL_COPY_VALUE(&argv[2], operation);
	if (argc == 4) {
		ZVAL_COPY_VALUE(&argv[3], options);
	}

	if (vips_php_call_array("math2", NULL, "", argc, argv, &ret)) {
		efree(argv);
		PHALCON_THROW_VIPS_EXCEPTION("Binary math operations failed (%s)");
		return;
	}
	efree(argv);
	if (!phalcon_array_isset_fetch_str(&image2, &ret, SL("out"), PH_READONLY)) {
		zval_ptr_dtor(&ret);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Binary math operations failed");
		return;
	}

	object_init_ex(return_value, phalcon_image_vips_ce);
	PHALCON_CALL_METHOD_FLAG(flag, NULL, return_value, "__construct", &image2);
	zval_ptr_dtor(&ret);
	if (flag != SUCCESS) {
		zval_ptr_dtor(return_value);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Binary math operations failed");
		return;
	}
}

/**
 * Boolean operation on two images.
 */
PHP_METHOD(Phalcon_Image_Vips, boolean)
{
	zval *right, *operation, *options = NULL, image = {}, *argv, rightimage = {}, ret = {}, image2 = {};
	int argc = 3, flag;

	phalcon_fetch_params(0, 1, 1, &right, &operation, &options);

	if (options && Z_TYPE_P(options) == IS_ARRAY) {
		argc = 4;
	}

	phalcon_read_property(&image, getThis(), SL("_image"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&rightimage, right, SL("_image"), PH_NOISY|PH_READONLY);

	argv = (zval *)emalloc(argc * sizeof(zval));
	ZVAL_COPY_VALUE(&argv[0], &image);
	ZVAL_COPY_VALUE(&argv[1], &rightimage);
	ZVAL_COPY_VALUE(&argv[2], operation);
	if (argc == 4) {
		ZVAL_COPY_VALUE(&argv[3], options);
	}

	if (vips_php_call_array("boolean", NULL, "", argc, argv, &ret)) {
		efree(argv);
		PHALCON_THROW_VIPS_EXCEPTION("Boolean operation on two images failed (%s)");
		return;
	}
	efree(argv);
	if (!phalcon_array_isset_fetch_str(&image2, &ret, SL("out"), PH_READONLY)) {
		zval_ptr_dtor(&ret);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Boolean operation on two images failed");
		return;
	}

	object_init_ex(return_value, phalcon_image_vips_ce);
	PHALCON_CALL_METHOD_FLAG(flag, NULL, return_value, "__construct", &image2);
	zval_ptr_dtor(&ret);
	if (flag != SUCCESS) {
		zval_ptr_dtor(return_value);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Boolean operation on two images failed");
		return;
	}
}

/**
 * Paint an image into another image
 */
PHP_METHOD(Phalcon_Image_Vips, drawImage)
{
	zval *sub, *x, *y, *options = NULL, image = {}, *argv, ret = {}, sub_image = {}, image2 = {};
	int argc = 4, flag;

	phalcon_fetch_params(0, 3, 1, &sub, &x, &y, &options);
	
	PHALCON_VERIFY_CLASS_EX(sub, phalcon_image_vips_ce, phalcon_image_exception_ce);

	if (options && Z_TYPE_P(options) == IS_ARRAY) {
		argc = 5;
	}

	phalcon_read_property(&image, getThis(), SL("_image"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&sub_image, sub, SL("_image"), PH_NOISY|PH_READONLY);

	argv = (zval *)emalloc(argc * sizeof(zval));
	ZVAL_COPY_VALUE(&argv[0], &image);
	ZVAL_COPY_VALUE(&argv[1], &sub_image);
	ZVAL_COPY_VALUE(&argv[2], x);
	ZVAL_COPY_VALUE(&argv[3], y);
	if (argc == 5) {
		ZVAL_COPY_VALUE(&argv[4], options);
	}

	if (vips_php_call_array("draw_image", NULL, "", argc, argv, &ret)) {
		efree(argv);
		PHALCON_THROW_VIPS_EXCEPTION("paint an image into another image failed (%s)");
		return;
	}
	efree(argv);
	if (!phalcon_array_isset_fetch_str(&image2, &ret, SL("image"), PH_READONLY)) {
		zval_ptr_dtor(&ret);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "paint an image into another image failed");
		return;
	}

	object_init_ex(return_value, phalcon_image_vips_ce);
	PHALCON_CALL_METHOD_FLAG(flag, NULL, return_value, "__construct", &image2);
	zval_ptr_dtor(&ret);
	if (flag != SUCCESS) {
		zval_ptr_dtor(return_value);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "paint an image into another image failed");
		return;
	}
}

/**
 * Draw a mask on an image
 */
PHP_METHOD(Phalcon_Image_Vips, drawMask)
{
	zval *mask, *color, *x, *y, image = {}, *argv, ret = {}, mask_image = {}, image2 = {};
	int argc = 5, flag;

	phalcon_fetch_params(0, 4, 0, &mask, &color, &x, &y);
	
	PHALCON_VERIFY_CLASS_EX(mask, phalcon_image_vips_ce, phalcon_image_exception_ce);

	if (PHALCON_IS_EMPTY_ARR(color)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "color parameter should must not be empty");
		return;
	}

	phalcon_read_property(&image, getThis(), SL("_image"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&mask_image, mask, SL("_image"), PH_NOISY|PH_READONLY);

	argv = (zval *)emalloc(argc * sizeof(zval));
	ZVAL_COPY_VALUE(&argv[0], &image);
	ZVAL_COPY_VALUE(&argv[1], color);
	ZVAL_COPY_VALUE(&argv[2], &mask_image);
	ZVAL_COPY_VALUE(&argv[3], x);
	ZVAL_COPY_VALUE(&argv[4], y);

	if (vips_php_call_array("draw_mask", NULL, "", argc, argv, &ret)) {
		efree(argv);
		PHALCON_THROW_VIPS_EXCEPTION("draw a mask on an image failed (%s)");
		return;
	}
	efree(argv);
	if (!phalcon_array_isset_fetch_str(&image2, &ret, SL("image"), PH_READONLY)) {
		zval_ptr_dtor(&ret);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "draw a mask on an image failed");
		return;
	}

	object_init_ex(return_value, phalcon_image_vips_ce);
	PHALCON_CALL_METHOD_FLAG(flag, NULL, return_value, "__construct", &image2);
	zval_ptr_dtor(&ret);
	if (flag != SUCCESS) {
		zval_ptr_dtor(return_value);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "draw a mask on an image failed");
		return;
	}
}

/**
 * Blur a rectangle on an image.
 */
PHP_METHOD(Phalcon_Image_Vips, drawSmudge)
{
	zval *left, *top, *width, *height, image = {}, *argv, ret = {}, image2 = {};
	int argc = 5, flag;

	phalcon_fetch_params(0, 4, 0, &left, &top, &width, &height);

	phalcon_read_property(&image, getThis(), SL("_image"), PH_NOISY|PH_READONLY);

	argv = (zval *)emalloc(argc * sizeof(zval));
	ZVAL_COPY_VALUE(&argv[0], &image);
	ZVAL_COPY_VALUE(&argv[1], left);
	ZVAL_COPY_VALUE(&argv[2], top);
	ZVAL_COPY_VALUE(&argv[3], width);
	ZVAL_COPY_VALUE(&argv[4], height);

	if (vips_php_call_array("draw_smudge", NULL, "", argc, argv, &ret)) {
		efree(argv);
		PHALCON_THROW_VIPS_EXCEPTION("blur a rectangle on an image failed (%s)");
		return;
	}
	efree(argv);
	if (!phalcon_array_isset_fetch_str(&image2, &ret, SL("image"), PH_READONLY)) {
		zval_ptr_dtor(&ret);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "blur a rectangle on an image failed");
		return;
	}

	object_init_ex(return_value, phalcon_image_vips_ce);
	PHALCON_CALL_METHOD_FLAG(flag, NULL, return_value, "__construct", &image2);
	zval_ptr_dtor(&ret);
	if (flag != SUCCESS) {
		zval_ptr_dtor(return_value);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "blur a rectangle on an image failed");
		return;
	}
}

/**
 * Flood-fill an area.
 */
PHP_METHOD(Phalcon_Image_Vips, drawFlood)
{
	zval *x, *y, *options = NULL, image = {}, *argv, ret = {}, image2 = {};
	int argc = 3, flag;

	phalcon_fetch_params(0, 2, 1, &x, &y, &options);

	if (options && Z_TYPE_P(options) == IS_ARRAY) {
		argc = 4;
	}

	phalcon_read_property(&image, getThis(), SL("_image"), PH_NOISY|PH_READONLY);

	argv = (zval *)emalloc(argc * sizeof(zval));
	ZVAL_COPY_VALUE(&argv[0], &image);
	ZVAL_COPY_VALUE(&argv[1], x);
	ZVAL_COPY_VALUE(&argv[2], y);
	if (argc == 4) {
		ZVAL_COPY_VALUE(&argv[3], options);
	}

	if (vips_php_call_array("draw_flood", NULL, "", argc, argv, &ret)) {
		efree(argv);
		PHALCON_THROW_VIPS_EXCEPTION("flood-fill an area failed (%s)");
		return;
	}
	efree(argv);
	if (!phalcon_array_isset_fetch_str(&image2, &ret, SL("image"), PH_READONLY)) {
		zval_ptr_dtor(&ret);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "flood-fill an area failed");
		return;
	}

	object_init_ex(return_value, phalcon_image_vips_ce);
	PHALCON_CALL_METHOD_FLAG(flag, NULL, return_value, "__construct", &image2);
	zval_ptr_dtor(&ret);
	if (flag != SUCCESS) {
		zval_ptr_dtor(return_value);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "flood-fill an area failed");
		return;
	}
}

/**
 * Draw a line on an image.
 */
PHP_METHOD(Phalcon_Image_Vips, drawLine)
{
	zval *color, *x1, *y1, *x2, *y2, image = {}, *argv, ret = {}, image2 = {};
	int argc = 6, flag;

	phalcon_fetch_params(0, 5, 0, &color, &x1, &y1, &x2, &y2);

	if (PHALCON_IS_EMPTY_ARR(color)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "color parameter should must not be empty");
		return;
	}

	phalcon_read_property(&image, getThis(), SL("_image"), PH_NOISY|PH_READONLY);

	argv = (zval *)emalloc(argc * sizeof(zval));
	ZVAL_COPY_VALUE(&argv[0], &image);
	ZVAL_COPY_VALUE(&argv[1], color);
	ZVAL_COPY_VALUE(&argv[2], x1);
	ZVAL_COPY_VALUE(&argv[3], y1);
	ZVAL_COPY_VALUE(&argv[4], x2);
	ZVAL_COPY_VALUE(&argv[5], y2);

	if (vips_php_call_array("draw_line", NULL, "", argc, argv, &ret)) {
		efree(argv);
		PHALCON_THROW_VIPS_EXCEPTION("draw a line on an image failed (%s)");
		return;
	}
	efree(argv);
	if (!phalcon_array_isset_fetch_str(&image2, &ret, SL("image"), PH_READONLY)) {
		zval_ptr_dtor(&ret);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "draw a line on an image failed");
		return;
	}

	object_init_ex(return_value, phalcon_image_vips_ce);
	PHALCON_CALL_METHOD_FLAG(flag, NULL, return_value, "__construct", &image2);
	zval_ptr_dtor(&ret);
	if (flag != SUCCESS) {
		zval_ptr_dtor(return_value);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "draw a line on an image failed");
		return;
	}
}

/**
 * Paint a rectangle on an image.
 */
PHP_METHOD(Phalcon_Image_Vips, drawRect)
{
	zval *color, *left, *top, *width, *height, *options = NULL, image = {}, *argv, ret = {}, image2 = {};
	int argc = 6, flag;

	phalcon_fetch_params(0, 5, 1, &color, &left, &top, &width, &height, &options);

	if (PHALCON_IS_EMPTY_ARR(color)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "color parameter should must not be empty");
		return;
	}

	if (options && Z_TYPE_P(options) == IS_ARRAY) {
		argc = 7;
	}

	phalcon_read_property(&image, getThis(), SL("_image"), PH_NOISY|PH_READONLY);

	argv = (zval *)emalloc(argc * sizeof(zval));
	ZVAL_COPY_VALUE(&argv[0], &image);
	ZVAL_COPY_VALUE(&argv[1], color);
	ZVAL_COPY_VALUE(&argv[2], left);
	ZVAL_COPY_VALUE(&argv[3], top);
	ZVAL_COPY_VALUE(&argv[4], width);
	ZVAL_COPY_VALUE(&argv[5], height);
	if (argc == 7) {
		ZVAL_COPY_VALUE(&argv[6], options);
	}
	if (vips_php_call_array("draw_rect", NULL, "", argc, argv, &ret)) {
		efree(argv);
		PHALCON_THROW_VIPS_EXCEPTION("draw a rectangle on an image failed (%s)");
		return;
	}
	efree(argv);
	if (!phalcon_array_isset_fetch_str(&image2, &ret, SL("image"), PH_READONLY)) {
		zval_ptr_dtor(&ret);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "draw a rectangle on an image failed");
		return;
	}

	object_init_ex(return_value, phalcon_image_vips_ce);
	PHALCON_CALL_METHOD_FLAG(flag, NULL, return_value, "__construct", &image2);
	zval_ptr_dtor(&ret);
	if (flag != SUCCESS) {
		zval_ptr_dtor(return_value);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "draw a rectangle on an image failed");
		return;
	}
}

/**
 * Draw a circle on an image.
 */
PHP_METHOD(Phalcon_Image_Vips, drawCircle)
{
	zval *color, *cx, *cy, *radius, *options = NULL, image = {}, *argv, ret = {}, image2 = {};
	int argc = 5, flag;

	phalcon_fetch_params(0, 4, 1, &color, &cx, &cy, &radius, &options);

	if (PHALCON_IS_EMPTY_ARR(color)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "color parameter should must not be empty");
		return;
	}

	if (options && Z_TYPE_P(options) == IS_ARRAY) {
		argc = 6;
	}

	phalcon_read_property(&image, getThis(), SL("_image"), PH_NOISY|PH_READONLY);

	argv = (zval *)emalloc(argc * sizeof(zval));
	ZVAL_COPY_VALUE(&argv[0], &image);
	ZVAL_COPY_VALUE(&argv[1], color);
	ZVAL_COPY_VALUE(&argv[2], cx);
	ZVAL_COPY_VALUE(&argv[3], cy);
	ZVAL_COPY_VALUE(&argv[4], radius);
	if (argc == 6) {
		ZVAL_COPY_VALUE(&argv[5], options);
	}

	if (vips_php_call_array("draw_circle", NULL, "", argc, argv, &ret)) {
		efree(argv);
		PHALCON_THROW_VIPS_EXCEPTION("draw a circle on an image failed (%s)");
		return;
	}
	efree(argv);
	if (!phalcon_array_isset_fetch_str(&image2, &ret, SL("image"), PH_READONLY)) {
		zval_ptr_dtor(&ret);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "draw a circle on an image failed");
		return;
	}

	object_init_ex(return_value, phalcon_image_vips_ce);
	PHALCON_CALL_METHOD_FLAG(flag, NULL, return_value, "__construct", &image2);
	zval_ptr_dtor(&ret);
	if (flag != SUCCESS) {
		zval_ptr_dtor(return_value);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "draw a circle on an image failed");
		return;
	}
}

/**
 * Write an image to a file.
 *
 * @param string $filename The file to write the image to.
 * @param array  $options  Any options to pass on to the selected save operation.
 *
 * @throws Exception
 *
 * @return void
 */
PHP_METHOD(Phalcon_Image_Vips, writeToFile)
{
	zval *filename, *options = NULL, image = {};

	phalcon_fetch_params(0, 1, 1, &filename, &options);

	if (!options) {
		options = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&image, getThis(), SL("_image"), PH_NOISY|PH_READONLY);

	PHALCON_CALL_CE_STATIC(return_value, phalcon_image_vips_ce, "write_to_file", &image, filename, options);
	if (Z_TYPE_P(return_value) == IS_LONG && Z_LVAL_P(return_value) == -1) {
		PHALCON_THROW_VIPS_EXCEPTION("Write an image to a file (%s)");
	}
}

/**
 * Write an image to a formatted string.
 *
 * @param string $suffix  The file type suffix, eg. ".jpg".
 * @param array  $options Any options to pass on to the selected save operation.
 *
 * @throws Exception
 *
 * @return string The formatted image.
 */
PHP_METHOD(Phalcon_Image_Vips, writeToBuffer)
{
	zval *suffix, *options = NULL, image = {}, ret = {};

	phalcon_fetch_params(0, 1, 1, &suffix, &options);

	if (!options) {
		options = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&image, getThis(), SL("_image"), PH_NOISY|PH_READONLY);

	PHALCON_CALL_CE_STATIC(&ret, phalcon_image_vips_ce, "write_to_buffer", &image, suffix, options);
	if (Z_TYPE(ret) == IS_LONG && Z_LVAL(ret) == -1) {
		PHALCON_THROW_VIPS_EXCEPTION("Write an image to a formatted string (%s)");
	}

	if (!phalcon_array_isset_fetch_str(return_value, &ret, SL("buffer"), PH_COPY)) {
		zval_ptr_dtor(&ret);
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Write an image to a formatted string failed");
		return;
	}
	zval_ptr_dtor(&ret);
}

/**
 * Write an image to a large memory array.
 *
 * @throws Exception
 *
 * @return string The memory array.
 */
PHP_METHOD(Phalcon_Image_Vips, writeToMemory)
{
	zval image = {};

	phalcon_read_property(&image, getThis(), SL("_image"), PH_NOISY|PH_READONLY);

	PHALCON_CALL_CE_STATIC(return_value, phalcon_image_vips_ce, "write_to_memory", &image);
	if (Z_TYPE_P(return_value) == IS_LONG && Z_LVAL_P(return_value) == -1) {
		PHALCON_THROW_VIPS_EXCEPTION("Write an image to a PHP array (%s)");
	}
}

/**
 * Write an image to a PHP array.
 *
 * Pixels are written as a simple one-dimensional array, for example, if
 * you write:
 *
 * ```php
 * $result = $image->crop(100, 100, 10, 1)->writeToArray();
 * ```
 *
 * This will crop out 10 pixels and write them to the array. If `$image`
 * is an RGB image, then `$array` will contain 30 numbers, with the first
 * three being R, G and B for the first pixel.
 *
 * You'll need to slice and repack the array if you want more dimensions.
 *
 * This method is much faster than repeatedly calling `getpoint()`. It
 * will use a lot of memory.
 *
 * @throws Exception
 *
 * @return array The pixel values as a PHP array.
 */
PHP_METHOD(Phalcon_Image_Vips, writeToArray)
{
	zval image = {};

	phalcon_read_property(&image, getThis(), SL("_image"), PH_NOISY|PH_READONLY);

	PHALCON_CALL_CE_STATIC(return_value, phalcon_image_vips_ce, "write_to_array", &image);
	if (Z_TYPE_P(return_value) == IS_LONG && Z_LVAL_P(return_value) == -1) {
		PHALCON_THROW_VIPS_EXCEPTION("Write an image to a PHP array (%s)");
	}
}

/**
 * Call any vips operation
 *
 * @param string $operation_name
 * @param resource $instance
 *
 * @return mixed
 **/
PHP_METHOD(Phalcon_Image_Vips, call)
{
	int argc;
	zval *argv;
	char *operation_name;
	size_t operation_name_len;
	zval *instance;

	VIPS_DEBUG_MSG("vips_call:\n");

	argc = ZEND_NUM_ARGS();

	if (argc < 1) {
		WRONG_PARAM_COUNT;
	}

	argv = (zval *)emalloc(argc * sizeof(zval));

	if (zend_get_parameters_array_ex(argc, argv) == FAILURE) {
		efree(argv);
		WRONG_PARAM_COUNT;
	}

	if (zend_parse_parameter(0, 0, &argv[0], 
		"s", &operation_name, &operation_name_len) == FAILURE) {
		efree(argv);
		RETURN_LONG(-1);
	}

	if (zend_parse_parameter(0, 1, &argv[1], "r!", &instance) == FAILURE) {
		efree(argv);
		RETURN_LONG(-1);
	}

	if (vips_php_call_array(operation_name, instance, 
		"", argc - 2, argv + 2, return_value)) {
		efree(argv);
		RETURN_LONG(-1);
	}

	efree(argv);
}

/**
 * Open an image from a filename
 *
 * @param string $filename
 * @param array $options
 *
 * @return mixed
 **/
PHP_METHOD(Phalcon_Image_Vips, new_from_file)
{
	char *name;
	size_t name_len;
	zval *options;
	char filename[VIPS_PATH_MAX];
	char option_string[VIPS_PATH_MAX];
	const char *operation_name;
	zval argv[2];
	int argc;

	VIPS_DEBUG_MSG("vips_image_new_from_file:\n");

	options = NULL;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "p|a!", 
		&name, &name_len, &options) == FAILURE) {
		RETURN_LONG(-1);
	}
	VIPS_DEBUG_MSG("vips_image_new_from_file: name = %s\n", name);

	vips__filename_split8(name, filename, option_string);
	if (!(operation_name = vips_foreign_find_load(filename))) {
		RETURN_LONG(-1);
	}

	ZVAL_STRING(&argv[0], filename);
	argc = 1;
	if (options && Z_TYPE_P(options) == IS_ARRAY) {
		ZVAL_ARR(&argv[1], Z_ARR_P(options));
		argc += 1;
	}

	if (vips_php_call_array(operation_name, NULL, 
		option_string, argc, argv, return_value)) {
		zval_dtor(&argv[0]);
		RETURN_LONG(-1);
	}

	zval_dtor(&argv[0]);
}

/**
 * Open an image from a string
 *
 * @param string $buffer
 * @param string $string_option
 * @param array $options
 *
 * @return mixed
 **/
PHP_METHOD(Phalcon_Image_Vips, new_from_buffer)
{
	char *buffer;
	size_t buffer_len;
	char *option_string;
	size_t option_string_len;
	zval *options;
	const char *operation_name;
	zval argv[2];
	int argc;

	VIPS_DEBUG_MSG("vips_image_new_from_buffer:\n");

	option_string = NULL;
	options = NULL;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s|sa!", 
		&buffer, &buffer_len, &option_string, &option_string_len, 
		&options) == FAILURE) {
		RETURN_LONG(-1);
	}

	if (!(operation_name = vips_foreign_find_load_buffer(buffer, buffer_len))) {
		RETURN_LONG(-1);
	}

	ZVAL_STRINGL(&argv[0], buffer, buffer_len);
	argc = 1;
	if (options && Z_TYPE_P(options) == IS_ARRAY) {
		ZVAL_ARR(&argv[1], Z_ARR_P(options));
		argc += 1;
	}

	if (vips_php_call_array(operation_name, NULL, 
		option_string, argc, argv, return_value)) {
		zval_dtor(&argv[0]);
		RETURN_LONG(-1);
	}

	zval_dtor(&argv[0]);
}

/**
 * Open an image from a array
 *
 * @param array $coefficients
 * @param double $scale
 * @param double $offset
 *
 * @return mixed
 **/
PHP_METHOD(Phalcon_Image_Vips, new_from_array)
{
	zval *array;
	double scale;
	double offset;
	int width;
	int height;
	VipsImage *mat;
	int x;
	zval *row;

	VIPS_DEBUG_MSG("vips_image_new_from_array:\n");

	scale = 1.0;
	offset = 0.0;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "a|dd", 
		&array, &scale, &offset) == FAILURE) {
		return;
	}

	height = zend_hash_num_elements(Z_ARRVAL_P(array));
	if ((row = zend_hash_index_find(Z_ARRVAL_P(array), 0)) == NULL) {
		php_error_docref(NULL, E_WARNING, "no element zero");
		return;
	}
	if (is_2D(array)) {
		mat =  matrix_from_zval(array);
	}
	else {
		/* 1D array.
		 */
		width = height;
		height = 1;

		mat = vips_image_new_matrix(width, height);

		for (x = 0; x < width; x++) {
			zval *ele;

			ele = zend_hash_index_find(Z_ARRVAL_P(array), x);
			if (ele) { 
				*VIPS_MATRIX(mat, x, 0) = zval_get_double(ele);
			}
		}
	}

	vips_image_set_double(mat, "scale", scale);
	vips_image_set_double(mat, "offset", offset);

	RETURN_RES(zend_register_resource(mat, le_vips_gobject));
}

/**
 * Make a new interpolator
 *
 * @param string $name
 *
 * @return mixed
 **/
PHP_METHOD(Phalcon_Image_Vips, interpolate_new)
{
	char *name;
	size_t name_len;
	VipsInterpolate *interp;

	VIPS_DEBUG_MSG("vips_interpolate_new:\n");

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "p", 
		&name, &name_len) == FAILURE) {
		return;
	}
	VIPS_DEBUG_MSG("vips_interpolate_new: name = %s\n", name);

	if (!(interp = vips_interpolate_new(name)))
		return;

	RETURN_RES(zend_register_resource(interp, le_vips_gobject));
}

/**
 * Write an image to a filename
 *
 * @param resource $image
 * @param string $filename
 * @param array $options
 *
 * @return mixed
 **/
PHP_METHOD(Phalcon_Image_Vips, write_to_file)
{
	zval *IM;
	char *filename;
	size_t filename_len;
	zval *options = NULL;
	VipsImage *image;
	char path_string[VIPS_PATH_MAX];
	char option_string[VIPS_PATH_MAX];
	const char *operation_name;
	zval argv[2];
	int argc;

	VIPS_DEBUG_MSG("vips_image_write_to_file:\n");

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rp|a!", 
		&IM, &filename, &filename_len, &options) == FAILURE) {
		RETURN_LONG(-1);
	}

	if ((image = (VipsImage *)zend_fetch_resource(Z_RES_P(IM), 
		phalcon_image_vips_type_name, le_vips_gobject)) == NULL) {
		RETURN_LONG(-1);
	}

	VIPS_DEBUG_MSG("\t%p -> %s\n", image, filename);

	vips__filename_split8(filename, path_string, option_string);
	if (!(operation_name = vips_foreign_find_save(path_string))) {
		RETURN_LONG(-1);
	}

	ZVAL_STRINGL(&argv[0], filename, filename_len);
	argc = 1;
	if (options && Z_TYPE_P(options) == IS_ARRAY) {
		ZVAL_ARR(&argv[1], Z_ARR_P(options));
		argc += 1;
	}

	if (vips_php_call_array(operation_name, IM, 
		option_string, argc, argv, return_value)) {
		zval_dtor(&argv[0]);
		RETURN_LONG(-1);
	}

	zval_dtor(&argv[0]);
}

/**
 * Write an image to a string
 *
 * @param resource $image
 * @param string $suffix
 * @param array $options
 *
 * @return mixed
 **/
PHP_METHOD(Phalcon_Image_Vips, write_to_buffer)
{
	zval *IM;
	zval *options = NULL;
	char *suffix;
	size_t suffix_len;
	VipsImage *image;
	char filename[VIPS_PATH_MAX];
	char option_string[VIPS_PATH_MAX];
	const char *operation_name;
	zval argv[1];
	int argc;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rs|a!", 
		&IM, &suffix, &suffix_len, &options) == FAILURE) {
		RETURN_LONG(-1);
	}

	if ((image = (VipsImage *)zend_fetch_resource(Z_RES_P(IM), 
		phalcon_image_vips_type_name, le_vips_gobject)) == NULL) {
		RETURN_LONG(-1);
	}

	vips__filename_split8(suffix, filename, option_string);
	if (!(operation_name = vips_foreign_find_save_buffer(filename))) {
		RETURN_LONG(-1);
	}

	argc = 0;
	if (options) {
		ZVAL_ARR(&argv[0], Z_ARR_P(options));
		argc += 1;
	}

	if (vips_php_call_array(operation_name, IM, 
		option_string, argc, argv, return_value)) {
		RETURN_LONG(-1);
	}
}

/**
 * Copy an image to a memory image
 *
 * @param resource $image
 *
 * @return mixed
 **/
PHP_METHOD(Phalcon_Image_Vips, copy_memory)
{
	zval *IM;
	VipsImage *image;
	VipsImage *new_image;
	zend_resource *resource;
	zval zvalue;

	VIPS_DEBUG_MSG("vips_image_copy_memory:\n");

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &IM) == FAILURE) {
		RETURN_LONG(-1);
	}

	if ((image = (VipsImage *)zend_fetch_resource(Z_RES_P(IM), 
		phalcon_image_vips_type_name, le_vips_gobject)) == NULL) {
		RETURN_LONG(-1);
	}

	if (!(new_image = vips_image_copy_memory(image))) {
		RETURN_LONG(-1);
	}

	/* Return as an array for all OK, -1 for error.
	 */
	array_init(return_value);
	resource = zend_register_resource(new_image, le_vips_gobject);
	ZVAL_RES(&zvalue, resource);
	add_assoc_zval(return_value, "out", &zvalue);
}

/**
 * Wrap an image around a memory array
 *
 * @param string $data
 * @param int $width
 * @param int $height
 * @param int $bands
 * @param string $format
 *
 * @return mixed
 **/
PHP_METHOD(Phalcon_Image_Vips, new_from_memory)
{
	char *bstr;
	size_t bstr_len;
	long width;
	long height;
	long bands;
	char *format;
	size_t format_len;
	int format_value;
	VipsBandFormat band_format;
	VipsImage *image;
	zend_resource *resource;
	zval zvalue;

	VIPS_DEBUG_MSG("vips_image_new_from_memory:\n");

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "slllp",
		&bstr, &bstr_len, &width, &height, &bands, &format, &format_len) == FAILURE) {
		RETURN_LONG(-1);
	}

	if ((format_value = vips_enum_from_nick("phalcon-vips", VIPS_TYPE_BAND_FORMAT, format)) < 0) {
		RETURN_LONG(-1);
	}
	band_format = format_value;

	if (!(image = vips_image_new_from_memory_copy(bstr, bstr_len, width, height, bands,
		band_format))) {
		RETURN_LONG(-1);
	}

	/* Return as an array for all OK, -1 for error.
	 */
	array_init(return_value);
	resource = zend_register_resource(image, le_vips_gobject);
	ZVAL_RES(&zvalue, resource);
	add_assoc_zval(return_value, "out", &zvalue);
}

/**
 * Write an image to a memory array
 *
 * @param resource $image
 *
 * @return string
 **/
PHP_METHOD(Phalcon_Image_Vips, write_to_memory)
{
	zval *IM;
	VipsImage *image;
	size_t arr_len;
	uint8_t *arr;

	VIPS_DEBUG_MSG("vips_image_write_to_memory:\n");

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &IM) == FAILURE) {
		RETURN_LONG(-1);
	}

	if ((image = (VipsImage *)zend_fetch_resource(Z_RES_P(IM),
		phalcon_image_vips_type_name, le_vips_gobject)) == NULL) {
		RETURN_LONG(-1);
	}

	if (!(arr = vips_image_write_to_memory(image, &arr_len))) {
		RETURN_LONG(-1);
	}

	RETVAL_STRINGL((char *)arr, arr_len);

	g_free(arr);
}

/**
 * Write an image to a PHP array
 *
 * @param resource $image
 *
 * @return array
 **/
PHP_METHOD(Phalcon_Image_Vips, write_to_array)
{
	zval *IM;
	VipsImage *image;
	size_t arr_len;
	uint8_t *arr;
	size_t n;

	VIPS_DEBUG_MSG("vips_image_write_to_array:\n");

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &IM) == FAILURE) {
		RETURN_LONG(-1);
	}

	if ((image = (VipsImage *)zend_fetch_resource(Z_RES_P(IM),
		phalcon_image_vips_type_name, le_vips_gobject)) == NULL) {
		RETURN_LONG(-1);
	}

	if (!(arr = vips_image_write_to_memory(image, &arr_len))) {
		RETURN_LONG(-1);
	}

	array_init(return_value);
	n = arr_len / vips_format_sizeof(image->BandFmt);
	g_assert(arr_len % vips_format_sizeof(image->BandFmt) == 0);
	switch (image->BandFmt) {
	case VIPS_FORMAT_UCHAR:
		ADD_ELEMENTS (unsigned char, add_next_index_long, n);
		break;

	case VIPS_FORMAT_CHAR:
		ADD_ELEMENTS (signed char, add_next_index_long, n);
		break;

	case VIPS_FORMAT_USHORT:
		ADD_ELEMENTS (unsigned short, add_next_index_long, n);
		break;

	case VIPS_FORMAT_SHORT:
		ADD_ELEMENTS (signed short, add_next_index_long, n);
		break;

	case VIPS_FORMAT_UINT:
		ADD_ELEMENTS (unsigned int, add_next_index_long, n);
		break;

	case VIPS_FORMAT_INT:
		ADD_ELEMENTS (signed int, add_next_index_long, n);
		break;

	case VIPS_FORMAT_FLOAT:
		ADD_ELEMENTS (float, add_next_index_double, n);
		break;

	case VIPS_FORMAT_DOUBLE:
		ADD_ELEMENTS (double, add_next_index_double, n);
		break;

	case VIPS_FORMAT_COMPLEX:
		ADD_ELEMENTS (float, add_next_index_double, n * 2);
		break;

	case VIPS_FORMAT_DPCOMPLEX:
		ADD_ELEMENTS (double, add_next_index_double, n * 2);
		break;

	default:
		break;
	}

	g_free(arr);
}

/**
 * Find a loader for a file
 *
 * @param string $filename
 *
 * @return string|long
 **/
PHP_METHOD(Phalcon_Image_Vips, foreign_find_load)
{
	char *filename;
	size_t filename_len;
	const char *operation_name;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", 
		&filename, &filename_len) == FAILURE) {
		RETURN_LONG(-1);
	}

	if (!(operation_name = vips_foreign_find_load(filename))) {
		RETURN_LONG(-1);
	}

	RETVAL_STRING(strdup(operation_name));
}

/**
 * Find a loader for a buffer
 *
 * @param string $buffer
 *
 * @return string|long
 **/
PHP_METHOD(Phalcon_Image_Vips, foreign_find_load_buffer)
{
	char *buffer;
	size_t buffer_len;
	const char *operation_name;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", 
		&buffer, &buffer_len) == FAILURE) {
		RETURN_LONG(-1);
	}

	if (!(operation_name = vips_foreign_find_load_buffer(buffer, buffer_len))) {
		RETURN_LONG(-1);
	}

	RETVAL_STRING(strdup(operation_name));
}

/**
 * Fetch field from image
 *
 * @param resource $image
 * @param string $field
 *
 * @return array
 **/
PHP_METHOD(Phalcon_Image_Vips, get)
{
	zval *im;
	char *field_name;
	size_t field_name_len;
	VipsImage *image;
	GValue gvalue = { 0 };
	zval zvalue;
	GParamSpec *pspec;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rs", 
		&im, &field_name, &field_name_len) == FAILURE) {
		RETURN_LONG(-1);
	}

	if ((image = (VipsImage *)zend_fetch_resource(Z_RES_P(im), 
		phalcon_image_vips_type_name, le_vips_gobject)) == NULL) {
		RETURN_LONG(-1);
	}

	/* Ugly: older libvipses would return enums from the true header fields 
	 * (eg. ->interpretation) as ints, but we want to send a string back
	 * for things like this.
	 *
	 * Test if field_name exists as a regular glib property and if it does, use
	 * g_object_get(). Otherwise use vips_image_get(), since it can read extra
	 * image metadata.
	 */
	if ((pspec = g_object_class_find_property(G_OBJECT_GET_CLASS(image), 
		field_name))) {
		g_value_init(&gvalue, G_PARAM_SPEC_VALUE_TYPE(pspec));
		g_object_get_property(G_OBJECT(image), field_name, &gvalue);
	} 
	else if (vips_image_get(image, field_name, &gvalue)) {
		RETURN_LONG(-1);
	}

	if (vips_php_gval_to_zval(&gvalue, &zvalue)) {
		g_value_unset(&gvalue);
		RETURN_LONG(-1);
	}
	g_value_unset(&gvalue);

	/* Return as an array for all OK, -1 for error.
	 */
	array_init(return_value);
	add_assoc_zval(return_value, "out", &zvalue);
}

/**
 * Fetch type of field from image
 *
 * @param resource $image
 * @param string $field
 *
 * @return long
 **/
PHP_METHOD(Phalcon_Image_Vips, get_typeof)
{
	zval *im;
	char *field_name;
	size_t field_name_len;
	VipsImage *image;
	GType type;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rs", 
		&im, &field_name, &field_name_len) == FAILURE) {
		RETURN_LONG(-1);
	}

	if ((image = (VipsImage *)zend_fetch_resource(Z_RES_P(im), 
		phalcon_image_vips_type_name, le_vips_gobject)) == NULL) {
		RETURN_LONG(-1);
	}

	type = vips_image_get_typeof(image, field_name); 
	
	RETURN_LONG(type);
}

/**
 * Set field on image
 *
 * @param resource $image
 * @param string $field
 * @param mixed $value
 *
 * @return long
 **/
PHP_METHOD(Phalcon_Image_Vips, set)
{
	zval *im;
	char *field_name;
	size_t field_name_len;
	zval *zvalue;
	VipsImage *image;
	GType type;
	GValue gvalue = { 0 };

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rsz", 
		&im, &field_name, &field_name_len, &zvalue) == FAILURE) {
		RETURN_LONG(-1);
	}

	if ((image = (VipsImage *)zend_fetch_resource(Z_RES_P(im), 
		phalcon_image_vips_type_name, le_vips_gobject)) == NULL) {
		RETURN_LONG(-1);
	}

	type = vips_image_get_typeof(image, field_name); 

	/* If the type is not set, guess a default.
	 */
	if (type == 0) {
		zval *ele;

		type = 0;

		zvalue = zval_get_nonref(zvalue); 

		switch (Z_TYPE_P(zvalue)) {
			case IS_ARRAY:
				if ((ele = zend_hash_index_find(Z_ARRVAL_P(zvalue), 
					0)) != NULL) {
					ele = zval_get_nonref(ele); 

					switch (Z_TYPE_P(ele)) {
						case IS_RESOURCE:
							type = VIPS_TYPE_ARRAY_IMAGE;
							break;

						case IS_LONG:
							type = VIPS_TYPE_ARRAY_INT;
							break;

						case IS_DOUBLE:
							type = VIPS_TYPE_ARRAY_DOUBLE;
							break;

						default:
							break;
					}
				}
				break;

			case IS_RESOURCE:
				type = VIPS_TYPE_IMAGE;
				break;

			case IS_LONG:
				type = G_TYPE_INT;
				break;

			case IS_DOUBLE:
				type = G_TYPE_DOUBLE;
				break;

			case IS_STRING:
				type = VIPS_TYPE_REF_STRING;
				break;

			default:
				break;
		}
	}

	g_value_init(&gvalue, type);

	if (vips_php_zval_to_gval(NULL, zvalue, &gvalue)) {
		RETURN_LONG(-1);
	}

	vips_image_set(image, field_name, &gvalue);

	g_value_unset(&gvalue);

	RETURN_LONG(0);
}

/**
 * Remove field from image
 *
 * @param resource $image
 * @param string $field
 *
 * @return long
 **/
PHP_METHOD(Phalcon_Image_Vips, remove)
{
	zval *im;
	char *field_name;
	size_t field_name_len;
	VipsImage *image;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rs", 
		&im, &field_name, &field_name_len) == FAILURE) {
		RETURN_LONG(-1);
	}

	if ((image = (VipsImage *)zend_fetch_resource(Z_RES_P(im), 
		phalcon_image_vips_type_name, le_vips_gobject)) == NULL) {
		RETURN_LONG(-1);
	}

	if (!vips_image_remove(image, field_name)) {
		RETURN_LONG(-1);
	}
	
	RETURN_LONG(0);
}

/**
 * Fetch and clear the vips error buffer
 *
 * @return string
 **/
PHP_METHOD(Phalcon_Image_Vips, error_buffer)
{
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "") == FAILURE) {
		return;
	}

	RETVAL_STRING(strdup(vips_error_buffer()));
	vips_error_clear();
}

/**
 * Set max number of operations to cache
 *
 * @param integer value
 **/
PHP_METHOD(Phalcon_Image_Vips, cache_set_max)
{
	long value;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &value) == FAILURE) {
		return;
	}

	vips_cache_set_max(value); 
}

/**
 * Get max number of operations to cache
 *
 * @return integer
 **/
PHP_METHOD(Phalcon_Image_Vips, cache_get_max)
{
	RETURN_LONG(vips_cache_get_max());
}

/**
 * Get max number of operations to cache
 *
 * @param integer $value
 **/
PHP_METHOD(Phalcon_Image_Vips, cache_set_max_mem)
{
	long value;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &value) == FAILURE) {
		return;
	}

	vips_cache_set_max_mem(value); 
}

/**
 * Get max memory to use for operation cache
 *
 * @return integer
 **/
PHP_METHOD(Phalcon_Image_Vips, cache_get_max_mem)
{
	RETURN_LONG(vips_cache_get_max_mem());
}

/**
 * Set max number of open files for operation cache
 *
 * @param integer $value
 *
 **/
PHP_METHOD(Phalcon_Image_Vips, cache_set_max_files)
{
	long value;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &value) == FAILURE) {
		return;
	}

	vips_cache_set_max_files(value); 
}

/**
 * Get max number of open files for operation cache
 *
 * @return integer
 **/
PHP_METHOD(Phalcon_Image_Vips, cache_get_max_files)
{
	RETURN_LONG(vips_cache_get_max_files());
}

/**
 * Get current cached operations
 *
 * @return integer
 **/
PHP_METHOD(Phalcon_Image_Vips, cache_get_size)
{
	RETURN_LONG(vips_cache_get_size());
}

/**
 * Set number of workers per threadpool
 *
 * @param integer $value
 *
 **/
PHP_METHOD(Phalcon_Image_Vips, concurrency_set)
{
	long value;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &value) == FAILURE) {
		return;
	}

	vips_concurrency_set(value); 
}

/**
 * Get number of workers per threadpool
 *
 * @return integer
 **/
PHP_METHOD(Phalcon_Image_Vips, concurrency_get)
{
	RETURN_LONG(vips_concurrency_get());
}

/**
 * Returns the version number of the vips library
 *
 * @return string
 **/
PHP_METHOD(Phalcon_Image_Vips, version)
{
	char digits[256];

	vips_snprintf(digits, 256, "%d.%d.%d", vips_version(0), vips_version(1), vips_version(2));

	RETVAL_STRING(digits);
}
