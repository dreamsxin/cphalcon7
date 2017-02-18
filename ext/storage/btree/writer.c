#include "storage/btree/bplus.h"
#include "storage/btree/private/writer.h"
#include "storage/btree/private/compressor.h"

#include "kernel/main.h"

#include <fcntl.h> /* open */
#include <unistd.h> /* close, write, read */
#include <sys/stat.h> /* S_IWUSR, S_IRUSR */
#include <stdio.h> /* sprintf */
#include <string.h> /* memset */
#include <errno.h> /* errno */

int _phalcon_storage_btree_writer_create(_phalcon_storage_btree_writer_t *w, const char *filename)
{
    off_t filesize;
    size_t filename_length;

    /* copy filename + '\0' char */
    filename_length = strlen(filename) + 1;
    w->filename = emalloc(filename_length);
    if (w->filename == NULL) return PHALCON_STORAGE_BTREE_EALLOC;
    memcpy(w->filename, filename, filename_length);

    w->fd = open(filename,
                 O_RDWR | O_APPEND | O_CREAT,
                 S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
    if (w->fd == -1) goto error;

    /* Determine filesize */
    filesize = lseek(w->fd, 0, SEEK_END);
    if (filesize == -1) goto error;

    w->filesize = (uint64_t) filesize;

    /* Nullify padding to shut up valgrind */
    memset(&w->padding, 0, sizeof(w->padding));

    return PHALCON_STORAGE_BTREE_OK;

error:
    efree(w->filename);
    return PHALCON_STORAGE_BTREE_EFILE;
}

int _phalcon_storage_btree_writer_destroy(_phalcon_storage_btree_writer_t *w)
{
    efree(w->filename);
    w->filename = NULL;
    if (close(w->fd)) return PHALCON_STORAGE_BTREE_EFILE;
    return PHALCON_STORAGE_BTREE_OK;
}

int _phalcon_storage_btree_writer_fsync(_phalcon_storage_btree_writer_t *w)
{
#ifdef F_FULLFSYNC
    /* OSX support */
    return fcntl(w->fd, F_FULLFSYNC);
#else
    return fdatasync(w->fd) == 0 ? PHALCON_STORAGE_BTREE_OK : PHALCON_STORAGE_BTREE_EFILEFLUSH;
#endif
}

int _phalcon_storage_btree_writer_compact_name(_phalcon_storage_btree_writer_t *w, char **compact_name)
{
    char *filename = emalloc(strlen(w->filename) + sizeof(".compact") + 1);
    if (filename == NULL) return PHALCON_STORAGE_BTREE_EALLOC;

    sprintf(filename, "%s.compact", w->filename);
    if (access(filename, F_OK) != -1 || errno != ENOENT) {
        efree(filename);
        return PHALCON_STORAGE_BTREE_ECOMPACT_EXISTS;
    }

    *compact_name = filename;
    return PHALCON_STORAGE_BTREE_OK;
}

int _phalcon_storage_btree_writer_compact_finalize(_phalcon_storage_btree_writer_t *s, _phalcon_storage_btree_writer_t *t)
{
    int ret;
    char *name, *compacted_name;

    /* save filename and prevent efreeing it */
    name = s->filename;
    compacted_name = t->filename;
    s->filename = NULL;
    t->filename = NULL;

    /* close both trees */
    _phalcon_storage_btree_destroy((phalcon_storage_btree_db_t *) s);
    ret = phalcon_storage_btree_close((phalcon_storage_btree_db_t *) t);
    if (ret != PHALCON_STORAGE_BTREE_OK) goto fatal;

    if (rename(compacted_name, name) != 0) return PHALCON_STORAGE_BTREE_EFILERENAME;

    /* reopen source tree */
    ret = _phalcon_storage_btree_writer_create(s, name);
    if (ret != PHALCON_STORAGE_BTREE_OK) goto fatal;
    ret = _phalcon_storage_btree_init((phalcon_storage_btree_db_t *) s);

fatal:
    efree(compacted_name);
    efree(name);

    return ret;
}

int _phalcon_storage_btree_writer_read(_phalcon_storage_btree_writer_t *w,
                    const enum comp_type comp,
                    const uint64_t offset,
                    uint64_t *size,
                    void **data)
{
    ssize_t bytes_read;
    char *cdata;

    if (w->filesize < offset + *size) return PHALCON_STORAGE_BTREE_EFILEREAD_OOB;

    /* Ignore empty reads */
    if (*size == 0) {
        *data = NULL;
        return PHALCON_STORAGE_BTREE_OK;
    }

    cdata = emalloc(*size);
    if (cdata == NULL) return PHALCON_STORAGE_BTREE_EALLOC;

    bytes_read = pread(w->fd, cdata, (size_t) *size, (off_t) offset);
    if ((uint64_t) bytes_read != *size) {
        efree(cdata);
        return PHALCON_STORAGE_BTREE_EFILEREAD;
    }

    /* no compression for head */
    if (comp == kNotCompressed) {
        *data = cdata;
    } else {
        int ret = 0;

        char *uncompressed = NULL;
        size_t usize;

        if (_phalcon_storage_btree_uncompressed_length(cdata, *size, &usize) != PHALCON_STORAGE_BTREE_OK) {
            ret = PHALCON_STORAGE_BTREE_EDECOMP;
        } else {
            uncompressed = emalloc(usize);
            if (uncompressed == NULL) {
                ret = PHALCON_STORAGE_BTREE_EALLOC;
            } else if (_phalcon_storage_btree_uncompress(cdata, *size, uncompressed, &usize) != PHALCON_STORAGE_BTREE_OK) {
                ret = PHALCON_STORAGE_BTREE_EDECOMP;
            } else {
                *data = uncompressed;
                *size = usize;
            }
        }

        efree(cdata);

        if (ret != PHALCON_STORAGE_BTREE_OK) {
            efree(uncompressed);
            return ret;
        }
    }

    return PHALCON_STORAGE_BTREE_OK;
}

int _phalcon_storage_btree_writer_write(_phalcon_storage_btree_writer_t *w,
                     const enum comp_type comp,
                     const void *data,
                     uint64_t *offset,
                     uint64_t *size)
{
    ssize_t written;
    uint32_t padding = sizeof(w->padding) - (w->filesize % sizeof(w->padding));

    /* Write padding */
    if (padding != sizeof(w->padding)) {
        written = write(w->fd, &w->padding, (size_t) padding);
        if ((uint32_t) written != padding) return PHALCON_STORAGE_BTREE_EFILEWRITE;
        w->filesize += padding;
    }

    /* Ignore empty writes */
    if (size == NULL || *size == 0) {
        if (offset != NULL) *offset = w->filesize;
        return PHALCON_STORAGE_BTREE_OK;
    }

    /* head shouldn't be compressed */
    if (comp == kNotCompressed) {
        written = write(w->fd, data, *size);
    } else {
        int ret;
        size_t max_csize = _phalcon_storage_btree_max_compressed_size(*size);
        size_t result_size;
        char *compressed = emalloc(max_csize);
        if (compressed == NULL) return PHALCON_STORAGE_BTREE_EALLOC;

        result_size = max_csize;
        ret = _phalcon_storage_btree_compress(data, *size, compressed, &result_size);
        if (ret != PHALCON_STORAGE_BTREE_OK) {
            efree(compressed);
            return PHALCON_STORAGE_BTREE_ECOMP;
        }

        *size = result_size;
        written = write(w->fd, compressed, result_size);
        efree(compressed);
    }

    if ((uint64_t) written != *size) return PHALCON_STORAGE_BTREE_EFILEWRITE;

    /* change offset */
    *offset = w->filesize;
    w->filesize += written;

    return PHALCON_STORAGE_BTREE_OK;
}

int _phalcon_storage_btree_writer_find(_phalcon_storage_btree_writer_t*w,
                    const enum comp_type comp,
                    const uint64_t size,
                    void *data,
                    _phalcon_storage_btree_writer_cb seek,
                    _phalcon_storage_btree_writer_cb miss)
{
    int ret = 0;
    int match = 0;
    uint64_t offset, size_tmp;

    /* Write padding first */
    ret = _phalcon_storage_btree_writer_write(w, kNotCompressed, NULL, NULL, NULL);
    if (ret != PHALCON_STORAGE_BTREE_OK) return ret;

    offset = w->filesize;
    size_tmp = size;

    /* Start seeking from bottom of file */
    while (offset >= size) {
        ret = _phalcon_storage_btree_writer_read(w, comp, offset - size, &size_tmp, &data);
        if (ret != PHALCON_STORAGE_BTREE_OK) break;

        /* Break if matched */
        if (seek(w, data) == 0) {
            match = 1;
            break;
        }

        offset -= size;
    }

    /* Not found - invoke miss */
    if (!match)
        ret = miss(w, data);

    return ret;
}
