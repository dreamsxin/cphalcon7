/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2018 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Authors: Martin Schröder <m.schroeder2007@gmail.com>                 |
  +----------------------------------------------------------------------+
*/

#include "async/core.h"


#include <ext/standard/file.h>
#include <ext/standard/flock_compat.h>
#include <ext/standard/php_filestat.h>

#if HAVE_SYS_FILE_H
#include <sys/file.h>
#endif

#define ASYNC_STRIP_FILE_SCHEME(url) do { \
	if (strncasecmp(url, "file://", sizeof("file://") - 1) == 0) { \
		url += sizeof("file://") - 1; \
	} else if (strncasecmp(url, "async-file://", sizeof("async-file://") - 1) == 0) { \
		url += sizeof("async-file://") - 1; \
	} \
} while (0)

#define ASYNC_FS_CALL(data, req, func, ...) do { \
	async_uv_op *op; \
	int code; \
	op = NULL; \
	if (UNEXPECTED((data)->scheduler->flags & ASYNC_TASK_SCHEDULER_FLAG_DISPOSED)) { \
		(data)->async = 0; \
	} \
	if (EXPECTED((data)->async)) { \
		ASYNC_ALLOC_CUSTOM_OP(op, sizeof(async_uv_op)); \
		(req)->data = op; \
	} \
	code = (func)(&(data)->scheduler->loop, req, __VA_ARGS__, (data)->async ? dummy_cb : NULL); \
	if (EXPECTED(code >= 0)) { \
		if (EXPECTED((data)->async)) { \
			if (UNEXPECTED(async_await_op((async_op *) op) == FAILURE)) { \
				ASYNC_FORWARD_OP_ERROR(op); \
				(req)->result = -1; \
				if (0 == uv_cancel((uv_req_t *) req)) { \
					ASYNC_FREE_OP(op); \
				} else { \
					((async_op *) op)->status = ASYNC_STATUS_FAILED; \
				} \
			} else { \
				code = op->code; \
				ASYNC_FREE_OP(op); \
			} \
		} \
	} else { \
		(req)->result = code; \
		if ((data)->async) { \
			ASYNC_FREE_OP(op); \
		} \
	} \
} while (0)

#define ASYNC_FS_CALLW(async, req, func, ...) do { \
	async_uv_op *op; \
	int code; \
	zend_bool disposed; \
	disposed = (async_task_scheduler_get()->flags & (ASYNC_TASK_SCHEDULER_FLAG_DISPOSED | ASYNC_TASK_SCHEDULER_FLAG_ERROR)) ? 1 : 0; \
	op = NULL; \
	if (EXPECTED(async && !disposed)) { \
		ASYNC_ALLOC_CUSTOM_OP(op, sizeof(async_uv_op)); \
		(req)->data = op; \
	} \
	code = (func)(async_loop_get(), req, __VA_ARGS__, (async && !disposed) ? dummy_cb : NULL); \
	if (EXPECTED(code >= 0)) { \
		if (EXPECTED(async && !disposed)) { \
			if (UNEXPECTED(async_await_op((async_op *) op) == FAILURE)) { \
				ASYNC_FORWARD_OP_ERROR(op); \
				(req)->result = -1; \
				if (0 == uv_cancel((uv_req_t *) req)) { \
					ASYNC_FREE_OP(op); \
				} else { \
					((async_op *) op)->status = ASYNC_STATUS_FAILED; \
				} \
			} else { \
				code = op->code; \
				ASYNC_FREE_OP(op); \
			} \
		} \
	} else { \
		(req)->result = code; \
		if (async && !disposed) { \
			ASYNC_FREE_OP(op); \
		} \
	} \
} while (0)

static php_stream_wrapper orig_file_wrapper;

typedef struct _async_dirstream_entry async_dirstream_entry;

struct _async_dirstream_entry {
	async_dirstream_entry *prev;
	async_dirstream_entry *next;

	/* Uses the struct hack to avoid additional memory allocations. */
	char name[1];
};

typedef struct _async_dirstream_data {
	async_dirstream_entry *first;
	async_dirstream_entry *last;
	async_dirstream_entry *entry;
	zend_off_t offset;
	zend_off_t size;
	async_task_scheduler *scheduler;
} async_dirstream_data;

typedef struct _async_filestream_data {
	uv_file file;
	char fmode[8];
	int mode;
	int lock_flag;
	zend_bool async;
	zend_bool finished;
	int64_t rpos;
	int64_t wpos;
	async_task_scheduler *scheduler;
} async_filestream_data;


ASYNC_CALLBACK dummy_cb(uv_fs_t* req)
{
	async_uv_op *op;
	
	op = (async_uv_op *) req->data;
	
	ZEND_ASSERT(op != NULL);
	
	if (UNEXPECTED(req->result == UV_ECANCELED)) {
		ASYNC_FREE_OP(op);
	} else {
		op->code = 0;

		ASYNC_FINISH_OP(op);
	}
}

static zend_always_inline void map_stat(uv_stat_t *stat, php_stream_statbuf *ssb)
{
	memset(ssb, 0, sizeof(php_stream_statbuf));

#ifdef PHP_WIN32
	ssb->sb.st_mode = (unsigned short) stat->st_mode;
	ssb->sb.st_size = stat->st_size;
	ssb->sb.st_nlink = (short) stat->st_nlink;

	// TODO: Check how PHP computes these values.
//	ssb->sb.st_dev = (short) stat->st_dev;
//	ssb->sb.st_rdev = (short) stat->st_rdev;

	ssb->sb.st_atime = (time_t) stat->st_atim.tv_sec;
	ssb->sb.st_mtime = (time_t) stat->st_mtim.tv_sec;
	ssb->sb.st_ctime = (time_t) stat->st_ctim.tv_sec;
#else
	ssb->sb.st_dev = stat->st_dev;
	ssb->sb.st_mode = stat->st_mode;
	ssb->sb.st_size = stat->st_size;
	ssb->sb.st_nlink = stat->st_nlink;
	ssb->sb.st_rdev = stat->st_rdev;

#if defined(__APPLE__)
    ssb->sb.st_atimespec.tv_sec = (time_t) stat->st_atim.tv_sec;
	ssb->sb.st_atimespec.tv_nsec = stat->st_atim.tv_nsec;
	
	ssb->sb.st_mtimespec.tv_sec = (time_t) stat->st_mtim.tv_sec;
	ssb->sb.st_mtimespec.tv_nsec = stat->st_mtim.tv_nsec;
	
	ssb->sb.st_ctimespec.tv_sec = (time_t) stat->st_ctim.tv_sec;
	ssb->sb.st_ctimespec.tv_nsec = stat->st_ctim.tv_nsec;
#else
	ssb->sb.st_atim.tv_sec = (time_t) stat->st_atim.tv_sec;
	ssb->sb.st_atim.tv_nsec = stat->st_atim.tv_nsec;
	
	ssb->sb.st_mtim.tv_sec = (time_t) stat->st_mtim.tv_sec;
	ssb->sb.st_mtim.tv_nsec = stat->st_mtim.tv_nsec;
	
	ssb->sb.st_ctim.tv_sec = (time_t) stat->st_ctim.tv_sec;
	ssb->sb.st_ctim.tv_nsec = stat->st_ctim.tv_nsec;
#endif
	
	ssb->sb.st_ino = stat->st_ino;
	ssb->sb.st_uid = stat->st_uid;
	ssb->sb.st_gid = stat->st_gid;
	ssb->sb.st_blksize = stat->st_blksize;
	ssb->sb.st_blocks = stat->st_blocks;
#endif
}

static zend_always_inline int parse_open_mode(const char *mode, int *mods)
{
	int flags;
	
	switch (mode[0]) {
	case 'r':
		flags = 0;
		break;
	case 'w':
		flags = UV_FS_O_TRUNC | UV_FS_O_CREAT;
		break;
	case 'a':
		flags = UV_FS_O_CREAT | UV_FS_O_APPEND;
		break;
	case 'x':
		flags = UV_FS_O_CREAT | UV_FS_O_EXCL;
		break;
	case 'c':
		flags = UV_FS_O_CREAT;
		break;
	default:
		return FAILURE;
	}
	
	if (strchr(mode, '+')) {
        flags |= UV_FS_O_RDWR;
    } else if (flags) {
        flags |= UV_FS_O_WRONLY;
    } else {
        flags |= UV_FS_O_RDONLY;
    }

#ifdef O_CLOEXEC
    if (strchr(mode, 'e')) {
        flags |= O_CLOEXEC;
    }
#endif

#ifdef O_NONBLOCK
    if (strchr(mode, 'n')) {
        flags |= O_NONBLOCK;
    }
#endif

#if defined(_O_TEXT) && defined(O_BINARY)
    if (strchr(mode, 't')) {
        flags |= _O_TEXT;
    } else {
        flags |= O_BINARY;
    }
#endif
	
	*mods = flags;
	
	return SUCCESS;
}

#if PHP_VERSION_ID >= 70400
static ssize_t async_dirstream_read(php_stream *stream, char *buf, size_t count)
#else
static size_t async_dirstream_read(php_stream *stream, char *buf, size_t count)
#endif
{
	async_dirstream_data *data;
	php_stream_dirent *ent;

	data = (async_dirstream_data *) stream->abstract;
	ent = (php_stream_dirent *) buf;
	
	if (UNEXPECTED(data->entry == NULL)) {
		return 0;
	}

	strcpy(ent->d_name, data->entry->name);

	data->entry = data->entry->next;
	data->offset++;

	return sizeof(php_stream_dirent);
}

static int async_dirstream_rewind(php_stream *stream, zend_off_t offset, int whence, zend_off_t *newoffs)
{
	async_dirstream_data *data;
	async_dirstream_entry *entry;

	zend_off_t i;

	data = (async_dirstream_data *) stream->abstract;

	switch (whence) {
	case SEEK_SET:
		entry = data->first;

		for (i = 0; i < offset; i++) {
			if (entry != NULL) {
				entry = entry->next;
			}
		}

		data->entry = entry;
		data->offset = offset;

		break;
	case SEEK_CUR:
		entry = data->first;

		for (i = 0; i < offset; i++) {
			if (entry != NULL) {
				entry = entry->next;
			}
		}

		data->entry = entry;
		data->offset += offset;

		break;
	case SEEK_END:
		entry = data->last;

		for (i = 0; i < offset; i++) {
			if (entry != NULL) {
				entry = entry->prev;
			}
		}

		data->entry = entry;
		data->offset = data->size - offset;

		break;
	default:
		return FAILURE;
	}

	*newoffs = data->offset;

	return SUCCESS;
}

static int async_dirstream_close(php_stream *stream, int close_handle)
{
	async_dirstream_data *data;
	async_dirstream_entry *entry;
	async_dirstream_entry *prev;
	
	data = (async_dirstream_data *) stream->abstract;
	entry = data->first;
	
	while (entry != NULL) {
		prev = entry;
		entry = entry->next;

		efree(prev);
	}
	
	async_task_scheduler_unref(data->scheduler);
	
	efree(data);
	
	return 0;
}

static php_stream_ops async_dirstream_ops = {
	NULL,
	async_dirstream_read,
	async_dirstream_close,
	NULL,
	"dir/async",
	async_dirstream_rewind,
	NULL,
	NULL,
	NULL
};

#if PHP_VERSION_ID >= 70400
static ssize_t async_filestream_write(php_stream *stream, const char *buf, size_t count)
#else
static size_t async_filestream_write(php_stream *stream, const char *buf, size_t count)
#endif
{
	async_filestream_data *data;
	uv_fs_t req;
	uv_buf_t bufs[1];
	
	data = (async_filestream_data *) stream->abstract;
	
	bufs[0] = uv_buf_init((char *) buf, (unsigned int) count);
	
	ASYNC_FS_CALL(data, &req, uv_fs_write, data->file, bufs, 1, data->wpos);
	
	uv_fs_req_cleanup(&req);
	
	if (UNEXPECTED(req.result < 0)) {
		return 0;
	}
	
	data->wpos += req.result;

	return (size_t) req.result;
}

#if PHP_VERSION_ID >= 70400
static ssize_t async_filestream_read(php_stream *stream, char *buf, size_t count)
#else
static size_t async_filestream_read(php_stream *stream, char *buf, size_t count)
#endif
{
	async_filestream_data *data;
	uv_fs_t req;
	uv_buf_t bufs[1];

	data = (async_filestream_data *) stream->abstract;
	
	if (UNEXPECTED(data->finished || count < 8192)) {
		return 0;
	}
	
	bufs[0] = uv_buf_init(buf, (unsigned int) count);
	
	ASYNC_FS_CALL(data, &req, uv_fs_read, data->file, bufs, 1, data->rpos);
	
	uv_fs_req_cleanup(&req);
	
	if (UNEXPECTED(req.result < 0)) {
		return 0;
	}
	
	if (UNEXPECTED((size_t) req.result < count)) {
		data->finished = 1;
		stream->eof = 1;
	}
	
	data->rpos += req.result;

	return (size_t) req.result;
}

static int async_filestream_close(php_stream *stream, int close_handle)
{
	async_filestream_data *data;
	uv_fs_t req;
	
	data = (async_filestream_data *) stream->abstract;
	
	if (EXPECTED(close_handle)) {
		ASYNC_FS_CALL(data, &req, uv_fs_close, data->file);
		
		uv_fs_req_cleanup(&req);
	}
	
	async_task_scheduler_unref(data->scheduler);
	
	efree(data);
	
	return (req.result < 1) ? 1 : 0;
}

static int async_filestream_flush(php_stream *stream)
{
	return 0;
}

static int async_filestream_cast(php_stream *stream, int castas, void **ret)
{
	async_filestream_data *data;
	php_socket_t fd;
	FILE *file;
	
	data = (async_filestream_data *) stream->abstract;
	
	switch (castas) {
	case PHP_STREAM_AS_STDIO:
		if (ret) {
			file = fdopen((int) data->file, data->fmode);
			
			if (file == NULL) {
				return FAILURE;
			}
			
			*(FILE **)ret = file;
		}
		return SUCCESS;
	case PHP_STREAM_AS_FD_FOR_SELECT:
	case PHP_STREAM_AS_FD:
		fd = (php_socket_t) uv_get_osfhandle((int) data->file);
		
		if (ret) {
			*(php_socket_t *)ret = fd;
		}
	
		return SUCCESS;
	}

	return FAILURE;
}

static int async_filestream_stat(php_stream *stream, php_stream_statbuf *ssb)
{
	async_filestream_data *data;
	uv_fs_t req;

	data = (async_filestream_data *) stream->abstract;
	
	ASYNC_FS_CALL(data, &req, uv_fs_fstat, data->file);
	
	uv_fs_req_cleanup(&req);

	if (UNEXPECTED(req.result < 0)) {
		return 1;
	}

	map_stat(&req.statbuf, ssb);
	
	return 0;
}

static int async_filestream_seek(php_stream *stream, zend_off_t offset, int whence, zend_off_t *newoffs)
{
	async_filestream_data *data;
	php_stream_statbuf ssb;
	
	int64_t pos;

	data = (async_filestream_data *) stream->abstract;
	
	if (UNEXPECTED(0 != async_filestream_stat(stream, &ssb))) {
		return FAILURE;
	}
	
	pos = data->rpos;

	switch (whence) {
		case SEEK_SET:
			data->rpos = offset;
			break;
		case SEEK_CUR:
			data->rpos = data->rpos + offset;
			break;
		case SEEK_END:
			data->rpos = ssb.sb.st_size + offset;
			break;
		default:
			return FAILURE;
	}
	
	if (UNEXPECTED(data->rpos < 0)) {
		data->rpos = pos;
		
		return FAILURE;
	}

	*newoffs = data->rpos;
	
	if (0 == (data->mode & UV_FS_O_APPEND)) {
		data->wpos = data->rpos;
	}
	
	if (data->rpos >= 0 && data->rpos < ssb.sb.st_size) {
		data->finished = 0;
		stream->eof = 0;
	}
	
	return SUCCESS;
}

static zend_always_inline int async_truncate(async_filestream_data *data, int64_t nsize)
{
	uv_fs_t req;

	ASYNC_FS_CALL(data, &req, uv_fs_ftruncate, data->file, nsize);

	uv_fs_req_cleanup(&req);

	return (UNEXPECTED(req.result < 0)) ? FAILURE : SUCCESS;
}

static int async_filestream_set_option(php_stream *stream, int option, int value, void *ptrparam)
{
	async_filestream_data *data;
	ptrdiff_t nsize;
	
	data = (async_filestream_data *) stream->abstract;
	
	switch (option) {
	case PHP_STREAM_OPTION_META_DATA_API:
		add_assoc_bool((zval *) ptrparam, "timed_out", 0);
		add_assoc_bool((zval *) ptrparam, "blocked", 1);
		add_assoc_bool((zval *) ptrparam, "eof", stream->eof);
		
		return PHP_STREAM_OPTION_RETURN_OK;
	case PHP_STREAM_OPTION_READ_BUFFER:
	 	if (value == PHP_STREAM_BUFFER_NONE) {
	 		stream->readbuf = perealloc(stream->readbuf, 0, stream->is_persistent);
	 		stream->flags |= PHP_STREAM_FLAG_NO_BUFFER;
	 	} else {
		 	stream->readbuflen = MAX(*((size_t *) ptrparam), 0x8000);
		 	stream->readbuf = perealloc(stream->readbuf, stream->readbuflen, stream->is_persistent);
		 	stream->flags &= ~PHP_STREAM_FLAG_NO_BUFFER;
	 	}
	 	
	 	return PHP_STREAM_OPTION_RETURN_OK;
	case PHP_STREAM_OPTION_LOCKING:
		if ((zend_uintptr_t) ptrparam == PHP_STREAM_LOCK_SUPPORTED) {
			return PHP_STREAM_OPTION_RETURN_OK;
		}
		
		// TODO: Check if non-blocking locks can be implemented...
		if (!flock((int) data->file, value)) {
			data->lock_flag = value;
			return PHP_STREAM_OPTION_RETURN_OK;
		}
		
		return PHP_STREAM_OPTION_RETURN_ERR;
	case PHP_STREAM_OPTION_MMAP_API:
		// TODO: Investigate support for mmap() combined with libuv.
		switch (value) {
		case PHP_STREAM_MMAP_SUPPORTED:
		case PHP_STREAM_MMAP_MAP_RANGE:
		case PHP_STREAM_MMAP_UNMAP:
			return PHP_STREAM_OPTION_RETURN_ERR;
		}
		return PHP_STREAM_OPTION_RETURN_ERR;
	case PHP_STREAM_OPTION_TRUNCATE_API:
		switch (value) {
		case PHP_STREAM_TRUNCATE_SUPPORTED:
			return PHP_STREAM_OPTION_RETURN_OK;
		case PHP_STREAM_TRUNCATE_SET_SIZE:
			nsize = *(ptrdiff_t *) ptrparam;

			if (nsize < 0) {
				return PHP_STREAM_OPTION_RETURN_ERR;
			}

			return (async_truncate(data, nsize) == SUCCESS) ? PHP_STREAM_OPTION_RETURN_OK : PHP_STREAM_OPTION_RETURN_ERR;
		}
	}

	return PHP_STREAM_OPTION_RETURN_NOTIMPL;
}

ASYNC_API php_stream_ops async_filestream_ops = {
	async_filestream_write,
	async_filestream_read,
	async_filestream_close,
	async_filestream_flush,
	"STDIO/async",
	async_filestream_seek,
	async_filestream_cast,
	async_filestream_stat,
	async_filestream_set_option
};


static php_stream *async_filestream_wrapper_open(php_stream_wrapper *wrapper, const char *path, const char *mode,
int options, zend_string **opened_path, php_stream_context *context STREAMS_DC)
{
	async_filestream_data *data;
	zend_bool async;
	
	uv_fs_t req;

	php_stream *stream;
	char realpath[MAXPATHLEN];
	int flags;
	
	if (UNEXPECTED(FAILURE == parse_open_mode(mode, &flags))) {
		if (options & REPORT_ERRORS) {
			php_error_docref(NULL, E_WARNING, "'%s' is not a valid mode for fopen", mode);
        }
        
		return NULL;
	}

	if (UNEXPECTED(!(options & STREAM_DISABLE_OPEN_BASEDIR) && php_check_open_basedir(path))) {
		return NULL;
	}

	async = ((options & STREAM_OPEN_FOR_INCLUDE) == 0) && ASYNC_G(cli);

	if (options & STREAM_ASSUME_REALPATH) {
		strlcpy(realpath, path, MAXPATHLEN);
	} else {
		if (expand_filepath(path, realpath) == NULL) {
			return NULL;
		}
	}

	ASYNC_FS_CALLW(async, &req, uv_fs_open, realpath, flags, 0666);

	uv_fs_req_cleanup(&req);
	
	if (UNEXPECTED(req.result < 0)) {
		if (options & REPORT_ERRORS) {
			php_error_docref(NULL, E_WARNING, "Failed to open file: %s", realpath);
		}
	
		return NULL;
	}

	data = ecalloc(1, sizeof(async_filestream_data));

	stream = php_stream_alloc_rel(&async_filestream_ops, data, 0, mode);

	if (UNEXPECTED(stream == NULL)) {
		efree(data);
		
		return NULL;
	}
	
	data->file = (uv_file) req.result;
	data->mode = flags;
	data->lock_flag = LOCK_UN;
	data->async = async;
	data->scheduler = async_task_scheduler_ref();
	
	strcpy(data->fmode, mode);
	
	if (opened_path != NULL) {
		*opened_path = zend_string_init(realpath, strlen(realpath), 0);
	}
	
	return stream;
}

static zend_always_inline async_dirstream_entry *create_dir_entry(async_dirstream_entry *prev, const char *name)
{
	async_dirstream_entry *entry;

	entry = emalloc(sizeof(async_dirstream_entry) + strlen(name));
	entry->prev = prev;
	entry->next = NULL;

	strcpy(entry->name, name);

	if (EXPECTED(prev != NULL)) {
		prev->next = entry;
	}

	return entry;
}

static php_stream *async_filestream_wrapper_opendir(php_stream_wrapper *wrapper, const char *path, const char *mode,
int options, zend_string **opened_path, php_stream_context *context STREAMS_DC)
{
	async_dirstream_data *data;
	async_dirstream_entry *prev;

	uv_fs_t req;
	uv_dirent_t tmp;

	php_stream *stream;
	char realpath[MAXPATHLEN];
	int code;

	if (UNEXPECTED(((options & STREAM_DISABLE_OPEN_BASEDIR) == 0) && php_check_open_basedir(path))) {
		return NULL;
	}
	
	if (options & STREAM_ASSUME_REALPATH) {
		strlcpy(realpath, path, MAXPATHLEN);
	} else {
		if (expand_filepath(path, realpath) == NULL) {
			return NULL;
		}
	}

	ASYNC_FS_CALLW(ASYNC_G(cli), &req, uv_fs_scandir, realpath, 0);
	
	if (UNEXPECTED(req.result < 0)) {
		if (options & REPORT_ERRORS) {
			php_error_docref(NULL, E_WARNING, "Failed to open dir %s: %s", realpath, uv_strerror((int) req.result));
		}
		
		uv_fs_req_cleanup(&req);
	
		return NULL;
	}
	
	data = ecalloc(1, sizeof(async_dirstream_data));
	
	stream = php_stream_alloc_rel(&async_dirstream_ops, data, 0, mode);
	
	if (UNEXPECTED(stream == NULL)) {
		efree(data);
		uv_fs_req_cleanup(&req);
		
		return NULL;
	}

	data->offset = 0;
	data->size = 2;

	prev = create_dir_entry(NULL, ".");

	data->first = prev;
	data->entry = prev;

	prev = create_dir_entry(prev, "..");

	while (1) {
		code = uv_fs_scandir_next(&req, &tmp);

		if (code == UV_EOF) {
			data->last = prev;

			break;
		}

		prev = create_dir_entry(prev, tmp.name);

		data->size++;
	}

	uv_fs_req_cleanup(&req);

	data->scheduler = async_task_scheduler_ref();
	
	if (opened_path != NULL) {
		*opened_path = zend_string_init(realpath, strlen(realpath), 0);
	}

	return stream;
}

static int async_filestream_wrapper_url_stat(php_stream_wrapper *wrapper, const char *url, int flags, php_stream_statbuf *ssb, php_stream_context *context)
{
	uv_fs_t req;
	
	char realpath[MAXPATHLEN];
	
	ASYNC_STRIP_FILE_SCHEME(url);
	
	if (UNEXPECTED(php_check_open_basedir(url))) {
		return 1;
	}
	
	if (UNEXPECTED(expand_filepath(url, realpath) == NULL)) {
		return 1;
	}
	
	if (flags & PHP_STREAM_URL_STAT_LINK) {
		ASYNC_FS_CALLW(ASYNC_G(cli), &req, uv_fs_lstat, realpath);
	} else {
		ASYNC_FS_CALLW(ASYNC_G(cli), &req, uv_fs_stat, realpath);
	}
	
	uv_fs_req_cleanup(&req);
	
	if (UNEXPECTED(req.result < 0)) {
		if (flags & REPORT_ERRORS) {
			php_error_docref(NULL, E_WARNING, "Failed to stat file %s: %s", realpath, uv_strerror((int) req.result));
		}
		
		return FAILURE;
	}
	
	map_stat(&req.statbuf, ssb);
	
	return SUCCESS;
}

static int async_filestream_wrapper_unlink(php_stream_wrapper *wrapper, const char *url, int options, php_stream_context *context)
{
	uv_fs_t req;
	
	char realpath[MAXPATHLEN];
	
	ASYNC_STRIP_FILE_SCHEME(url);
	
	if (UNEXPECTED(((options & STREAM_DISABLE_OPEN_BASEDIR) == 0) && php_check_open_basedir(url))) {
		return 0;
	}
	
	if (UNEXPECTED(expand_filepath(url, realpath) == NULL)) {
		return 0;
	}
	
	ASYNC_FS_CALLW(ASYNC_G(cli), &req, uv_fs_unlink, realpath);
	
	uv_fs_req_cleanup(&req);
	
	if (UNEXPECTED(req.result < 0)) {
		if (options & REPORT_ERRORS) {
			php_error_docref(NULL, E_WARNING, "Failed to unlink file %s: %s", realpath, uv_strerror((int) req.result));
		}
		
		return 0;
	}
	
	php_clear_stat_cache(1, NULL, 0);
	
	return 1;
}

static int async_filestream_wrapper_rename(php_stream_wrapper *wrapper, const char *url_from, const char *url_to, int options, php_stream_context *context)
{
	uv_fs_t req;
	
	if (UNEXPECTED(!url_from || !url_to)) {
		return 0;
	}
	
	ASYNC_STRIP_FILE_SCHEME(url_from);
	ASYNC_STRIP_FILE_SCHEME(url_to);
	
	if (UNEXPECTED(php_check_open_basedir(url_from) || php_check_open_basedir(url_to))) {
		return 0;
	}
	
	ASYNC_FS_CALLW(ASYNC_G(cli), &req, uv_fs_rename, url_from, url_to);
	
	uv_fs_req_cleanup(&req);
	
	if (UNEXPECTED(req.result < 0)) {
		if (options & REPORT_ERRORS) {
			php_error_docref(NULL, E_WARNING, "Failed to rename %s: %s", url_from, uv_strerror((int) req.result));
		}
	
		return 0;
	}

	php_clear_stat_cache(1, NULL, 0);
	
	return 1;
}

static zend_always_inline int async_mkdir(const char *url, int mode)
{
	uv_fs_t req;
	
	ASYNC_FS_CALLW(ASYNC_G(cli), &req, uv_fs_mkdir, url, mode);
	
	uv_fs_req_cleanup(&req);
	
	return (int) req.result;
}

static int async_filestream_wrapper_mkdir(php_stream_wrapper *wrapper, const char *dir, int mode, int options, php_stream_context *context)
{
	int ret, recursive = options & PHP_STREAM_MKDIR_RECURSIVE;
	char *p;

	ASYNC_STRIP_FILE_SCHEME(dir);

	if (!recursive) {
		ret = async_mkdir(dir, mode);
	} else {
		/* we look for directory separator from the end of string, thus hopefuly reducing our work load */
		char *e;
		zend_stat_t sb;
		size_t dir_len = strlen(dir), offset = 0;
		char buf[MAXPATHLEN];

		if (!expand_filepath_with_mode(dir, buf, NULL, 0, CWD_EXPAND )) {
			php_error_docref(NULL, E_WARNING, "Invalid path");
			return 0;
		}

		e = buf +  strlen(buf);

		if ((p = memchr(buf, DEFAULT_SLASH, dir_len))) {
			offset = p - buf + 1;
		}

		if (p && dir_len == 1) {
			/* buf == "DEFAULT_SLASH" */
		}
		else {
			/* find a top level directory we need to create */
			while ( (p = strrchr(buf + offset, DEFAULT_SLASH)) || (offset != 1 && (p = strrchr(buf, DEFAULT_SLASH))) ) {
				int n = 0;

				*p = '\0';
				while (p > buf && *(p-1) == DEFAULT_SLASH) {
					++n;
					--p;
					*p = '\0';
				}
				if (VCWD_STAT(buf, &sb) == 0) {
					while (1) {
						*p = DEFAULT_SLASH;
						if (!n) break;
						--n;
						++p;
					}
					break;
				}
			}
		}

		if (p == buf) {
			ret = async_mkdir(dir, mode);
		} else if (!(ret = async_mkdir(buf, mode))) {
			if (!p) {
				p = buf;
			}
			/* create any needed directories if the creation of the 1st directory worked */
			while (++p != e) {
				if (*p == '\0') {
					*p = DEFAULT_SLASH;
					if ((*(p+1) != '\0') && (ret = async_mkdir(buf, mode)) < 0) {
						break;
					}
				}
			}
		}
	}
	
	if (UNEXPECTED(ret < 0 && options & REPORT_ERRORS)) {
		php_error_docref(NULL, E_WARNING, "%s", uv_strerror(ret));
	}
	
	return (UNEXPECTED(ret < 0)) ? 0 : 1;
}

static int async_filestream_wrapper_rmdir(php_stream_wrapper *wrapper, const char *url, int options, php_stream_context *context)
{
	uv_fs_t req;
	
	char realpath[MAXPATHLEN];
	
	ASYNC_STRIP_FILE_SCHEME(url);
	
	if (UNEXPECTED(((options & STREAM_DISABLE_OPEN_BASEDIR) == 0) && php_check_open_basedir(url))) {
		return 0;
	}
	
	if (UNEXPECTED(expand_filepath(url, realpath) == NULL)) {
		return 0;
	}
	
	ASYNC_FS_CALLW(ASYNC_G(cli), &req, uv_fs_rmdir, realpath);
	
	uv_fs_req_cleanup(&req);
	
	if (UNEXPECTED(req.result < 0)) {
		if (options & REPORT_ERRORS) {
			php_error_docref(NULL, E_WARNING, "Failed to delete %s: %s", realpath, uv_strerror((int) req.result));
		}
	
		return 0;
	}
	
	php_clear_stat_cache(1, NULL, 0);
	
	return 1;
}

static zend_always_inline int async_chmod(const char *url, int mode)
{
	uv_fs_t req;
	
	ASYNC_FS_CALLW(ASYNC_G(cli), &req, uv_fs_chmod, url, mode);
	
	if (UNEXPECTED(req.result < 0)) {
		php_error_docref1(NULL, url, E_WARNING, "Operation failed: %s", uv_strerror((int) req.result));
	
		return 0;
	}

	return 1;
}

static zend_always_inline int async_touch(const char *url, struct utimbuf *time)
{
	uv_fs_t req;
	
	ASYNC_FS_CALLW(ASYNC_G(cli), &req, uv_fs_utime, url, (double) time->actime, (double) time->modtime);
	
	uv_fs_req_cleanup(&req);
	
	if (req.result == UV_ENOENT) {
		return UV_ENOENT;
	}
	
	if (UNEXPECTED(req.result < 0)) {
		php_error_docref1(NULL, url, E_WARNING, "Operation failed: %s", uv_strerror((int) req.result));
		
		return 0;
	}
	
	return 1;
}

static zend_always_inline int async_touch_create(const char *url)
{
	uv_fs_t req;
	uv_fs_t close;
	uv_file file;
	
	ASYNC_FS_CALLW(ASYNC_G(cli), &req, uv_fs_open, url, UV_FS_O_CREAT | UV_FS_O_APPEND, 0);
	
	uv_fs_req_cleanup(&req);
	
	if (UNEXPECTED(req.result < 0)) {
		php_error_docref1(NULL, url, E_WARNING, "Operation failed: %s", uv_strerror((int) req.result));
		
		return 0;
	}
	
	file = (uv_file) req.result;

	ASYNC_FS_CALLW(ASYNC_G(cli), &close, uv_fs_close, file);
	
	uv_fs_req_cleanup(&close);
	
	return 1;
}

static int async_filestream_wrapper_metadata(php_stream_wrapper *wrapper, const char *url, int option, void *value, php_stream_context *context)
{
	int ret;
	
	ASYNC_STRIP_FILE_SCHEME(url);

	if (UNEXPECTED(php_check_open_basedir(url))) {
		return 0;
	}
	
	switch (option) {
	case PHP_STREAM_META_ACCESS:
		ret = async_chmod(url, (int) *(zend_long *) value);
		break;
	case PHP_STREAM_META_TOUCH:
		ret = async_touch(url, (struct utimbuf *) value);
		
		if (ret == UV_ENOENT) {
			ret = async_touch_create(url);
			
			if (ret) {
				ret = async_touch(url, (struct utimbuf *) value);
				
				if (UNEXPECTED(ret == UV_ENOENT)) {
					ret = 0;
				}
			}
		}
		break;
	default:
		// TODO: Add non-blocking chown() and chgrp() implementations...
	
		return orig_file_wrapper.wops->stream_metadata(wrapper, url, option, value, context);
	}

	if (EXPECTED(ret == 1)) {
		php_clear_stat_cache(0, url, strlen(url));
	}

	return ret;
}

static php_stream_wrapper_ops async_filestream_wrapper_ops = {
	async_filestream_wrapper_open,
	NULL,
	NULL,
	async_filestream_wrapper_url_stat,
	async_filestream_wrapper_opendir,
	"plainfile/async",
	async_filestream_wrapper_unlink,
	async_filestream_wrapper_rename,
	async_filestream_wrapper_mkdir,
	async_filestream_wrapper_rmdir,
	async_filestream_wrapper_metadata
};

static php_stream_wrapper async_filestream_wrapper = {
	&async_filestream_wrapper_ops,
	NULL,
	0
};


void async_filesystem_init()
{
	if (ASYNC_G(cli)) {
		php_register_url_stream_wrapper("async-file", &async_filestream_wrapper);
	
		if (ASYNC_G(fs_enabled)) {
			orig_file_wrapper = php_plain_files_wrapper;
			php_plain_files_wrapper = async_filestream_wrapper;
		}
	}
}

void async_filesystem_shutdown()
{
	if (ASYNC_G(cli)) {
		if (ASYNC_G(fs_enabled)) {
			php_plain_files_wrapper = orig_file_wrapper;
		}
		
		php_unregister_url_stream_wrapper("async-file");
	}
}

