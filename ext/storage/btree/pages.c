#include <assert.h> /* assert */

#include "storage/btree/bplus.h"
#include "storage/btree/private/pages.h"
#include "storage/btree/private/utils.h"

#include "kernel/main.h"

int _phalcon_storage_btree_page_create(phalcon_storage_btree_db_t *t,
                    const enum page_type type,
                    const uint64_t offset,
                    const uint64_t config,
                    _phalcon_storage_btree_page_t **page)
{
    /* Allocate space for page + keys */
    _phalcon_storage_btree_page_t *p;

    p = emalloc(sizeof(*p) + sizeof(p->keys[0]) * (t->head.page_size - 1));
    if (p == NULL) return PHALCON_STORAGE_BTREE_EALLOC;

    p->type = type;
    if (type == kLeaf) {
        p->length = 0;
        p->byte_size = 0;
    } else {
        /* non-leaf pages always have left element */
        p->length = 1;
        p->keys[0].value = NULL;
        p->keys[0].length = 0;
        p->keys[0].offset = 0;
        p->keys[0].config = 0;
        p->keys[0].allocated = 0;
        p->byte_size = PHALCON_STORAGE_BTREE__KV_SIZE(p->keys[0]);
    }

    /* this two fields will be changed on page_write */
    p->offset = offset;
    p->config = config;

    p->buff_ = NULL;
    p->is_head = 0;

    *page = p;
    return PHALCON_STORAGE_BTREE_OK;
}

void _phalcon_storage_btree_page_destroy(phalcon_storage_btree_db_t *t, _phalcon_storage_btree_page_t *page)
{
    uint64_t i;
    /* Free all keys */
    for (i = 0; i < page->length; i++) {
        if (page->keys[i].allocated) {
            efree(page->keys[i].value);
            page->keys[i].value = NULL;
        }
    }

    if (page->buff_ != NULL) {
        efree(page->buff_);
        page->buff_ = NULL;
    }

    /* Free page itself */
    efree(page);
}

int _phalcon_storage_btree_page_clone(phalcon_storage_btree_db_t *t, _phalcon_storage_btree_page_t *page, _phalcon_storage_btree_page_t **clone)
{
    uint64_t i;
    int ret = PHALCON_STORAGE_BTREE_OK;
    ret = _phalcon_storage_btree_page_create(t, page->type, page->offset, page->config, clone);
    if (ret != PHALCON_STORAGE_BTREE_OK) return ret;

    (*clone)->is_head = page->is_head;

    (*clone)->length = 0;
    for (i = 0; i < page->length; i++) {
        ret = _phalcon_storage_btree_kv_copy(&page->keys[i], &(*clone)->keys[i], 1);
        (*clone)->length++;
        if (ret != PHALCON_STORAGE_BTREE_OK) break;
    }
    (*clone)->byte_size = page->byte_size;

    /* if failed - efree memory for all allocated keys */
    if (ret != PHALCON_STORAGE_BTREE_OK) _phalcon_storage_btree_page_destroy(t, *clone);

    return ret;
}

int _phalcon_storage_btree_page_read(phalcon_storage_btree_db_t *t, _phalcon_storage_btree_page_t *page)
{
    int ret;
    uint64_t size, o;
    uint64_t i;
    _phalcon_storage_btree_writer_t *w = (_phalcon_storage_btree_writer_t *) t;

    char *buff = NULL;

    /* Read page size and leaf flag */
    size = page->config >> 1;
    page->type = page->config & 1 ? kLeaf : kPage;

    /* Read page data */
    ret = _phalcon_storage_btree_writer_read(w, kCompressed, page->offset, &size, (void**) &buff);
    if (ret != PHALCON_STORAGE_BTREE_OK) return ret;

    /* Parse data */
    i = 0;
    o = 0;
    while (o < size) {
        page->keys[i].length = _phalcon_ntohll(*(uint64_t *) (buff + o));
        page->keys[i].offset = _phalcon_ntohll(*(uint64_t *) (buff + o + 8));
        page->keys[i].config = _phalcon_ntohll(*(uint64_t *) (buff + o + 16));
        page->keys[i].value = buff + o + 24;
        page->keys[i].allocated = 0;

        o += PHALCON_STORAGE_BTREE__KV_SIZE(page->keys[i]);
        i++;
    }
    page->length = i;
    page->byte_size = size;

    if (page->buff_ != NULL) {
        efree(page->buff_);
    }
    page->buff_ = buff;

    return PHALCON_STORAGE_BTREE_OK;
}

int _phalcon_storage_btree_page_load(phalcon_storage_btree_db_t *t,
                  const uint64_t offset,
                  const uint64_t config,
                  _phalcon_storage_btree_page_t **page)
{
    int ret;

    _phalcon_storage_btree_page_t *new_page;
    ret = _phalcon_storage_btree_page_create(t, 0, offset, config, &new_page);
    if (ret != PHALCON_STORAGE_BTREE_OK) return ret;

    ret = _phalcon_storage_btree_page_read(t, new_page);
    if (ret != PHALCON_STORAGE_BTREE_OK) {
        _phalcon_storage_btree_page_destroy(t, new_page);
        return ret;
    }

    /* _phalcon_storage_btree_page_load should be atomic */
    *page = new_page;

    return PHALCON_STORAGE_BTREE_OK;
}

int _phalcon_storage_btree_page_save(phalcon_storage_btree_db_t *t, _phalcon_storage_btree_page_t *page)
{
    int ret;
    _phalcon_storage_btree_writer_t *w = (_phalcon_storage_btree_writer_t *) t;
    uint64_t i;
    uint64_t o;
    char *buff;

    assert(page->type == kLeaf || page->length != 0);

    /* Allocate space for serialization (header + keys); */
    buff = emalloc(page->byte_size);
    if (buff == NULL) return PHALCON_STORAGE_BTREE_EALLOC;

    o = 0;
    for (i = 0; i < page->length; i++) {
        assert(o + PHALCON_STORAGE_BTREE__KV_SIZE(page->keys[i]) <= page->byte_size);

        *(uint64_t *) (buff + o) = _phalcon_htonll(page->keys[i].length);
        *(uint64_t *) (buff + o + 8) = _phalcon_htonll(page->keys[i].offset);
        *(uint64_t *) (buff + o + 16) = _phalcon_htonll(page->keys[i].config);

        memcpy(buff + o + 24, page->keys[i].value, page->keys[i].length);

        o += PHALCON_STORAGE_BTREE__KV_SIZE(page->keys[i]);
    }
    assert(o == page->byte_size);

    page->config = page->byte_size;
    ret = _phalcon_storage_btree_writer_write(w,
                           kCompressed,
                           buff,
                           &page->offset,
                           &page->config);
    page->config = (page->config << 1) | (page->type == kLeaf);

    efree(buff);
    return ret;
}


int _phalcon_storage_btree_page_load_value(phalcon_storage_btree_db_t *t,
                        _phalcon_storage_btree_page_t *page,
                        const uint64_t index,
                        phalcon_storage_btree_value_t *value)
{
    return _phalcon_storage_btree_value_load(t,
                          page->keys[index].offset,
                          page->keys[index].config,
                          value);
}


int _phalcon_storage_btree_page_save_value(phalcon_storage_btree_db_t *t,
                        _phalcon_storage_btree_page_t *page,
                        const uint64_t index,
                        const int cmp,
                        const phalcon_storage_btree_key_t *key,
                        const phalcon_storage_btree_value_t *value,
                        phalcon_storage_btree_update_cb update_cb,
                        void *arg)
{
    int ret;
    _phalcon_storage_btree_kv_t previous, tmp;

    /* replace item with same key from page */
    if (cmp == 0) {
        /* solve conflicts if callback was provided */
        if (update_cb != NULL) {
            phalcon_storage_btree_value_t prev_value;

            ret = _phalcon_storage_btree_page_load_value(t, page, index, &prev_value);
            if (ret != PHALCON_STORAGE_BTREE_OK) return ret;

            ret = update_cb(arg, &prev_value, value);
            efree(prev_value.value);

            if (!ret) return PHALCON_STORAGE_BTREE_EUPDATECONFLICT;
        }
        previous.offset = page->keys[index].offset;
        previous.length = page->keys[index].config;
        _phalcon_storage_btree_page_remove_idx(t, page, index);
    }

    /* store key */
    tmp.value = key->value;
    tmp.length = key->length;

    /* store value */
    ret = _phalcon_storage_btree_value_save(t,
                         value,
                         cmp == 0 ? &previous : NULL,
                         &tmp.offset,
                         &tmp.config);
    if (ret != PHALCON_STORAGE_BTREE_OK) return ret;

    /* Shift all keys right */
    _phalcon_storage_btree_page_shiftr(t, page, index);

    /* Insert key in the middle */
    ret = _phalcon_storage_btree_kv_copy(&tmp, &page->keys[index], 1);
    if (ret != PHALCON_STORAGE_BTREE_OK) {
        /* shift keys back */
        _phalcon_storage_btree_page_shiftl(t, page, index);
        return ret;
    }

    page->byte_size += PHALCON_STORAGE_BTREE__KV_SIZE(tmp);
    page->length++;

    return PHALCON_STORAGE_BTREE_OK;
}

int _phalcon_storage_btree_page_search(phalcon_storage_btree_db_t *t,
                    _phalcon_storage_btree_page_t *page,
                    const phalcon_storage_btree_key_t *key,
                    const enum search_type type,
                    _phalcon_storage_btree_page_search_res_t *result)
{
    int ret;
    uint64_t i = page->type == kPage;
    int cmp = -1;
    _phalcon_storage_btree_page_t *child;

    /* assert infinite recursion */
    assert(page->type == kLeaf || page->length > 0);

    while (i < page->length) {
        /* left key is always lower in non-leaf nodes */
        cmp = t->compare_cb((phalcon_storage_btree_key_t*) &page->keys[i], key);

        if (cmp >= 0) break;
        i++;
    }

    result->cmp = cmp;

    if (page->type == kLeaf) {
        result->index = i;
        result->child = NULL;

        return PHALCON_STORAGE_BTREE_OK;
    } else {
        assert(i > 0);
        if (cmp != 0) i--;

        if (type == kLoad) {
            ret = _phalcon_storage_btree_page_load(t,
                                page->keys[i].offset,
                                page->keys[i].config,
                                &child);
            if (ret != PHALCON_STORAGE_BTREE_OK) return ret;

            result->child = child;
        } else {
            result->child = NULL;
        }

        result->index = i;

        return PHALCON_STORAGE_BTREE_OK;
    }
}

int _phalcon_storage_btree_page_get(phalcon_storage_btree_db_t *t,
                 _phalcon_storage_btree_page_t *page,
                 const phalcon_storage_btree_key_t *key,
                 phalcon_storage_btree_value_t *value)
{
    int ret;
    _phalcon_storage_btree_page_search_res_t res;
    ret = _phalcon_storage_btree_page_search(t, page, key, kLoad, &res);
    if (ret != PHALCON_STORAGE_BTREE_OK) return ret;

    if (res.child == NULL) {
        if (res.cmp != 0) return PHALCON_STORAGE_BTREE_ENOTFOUND;

        return _phalcon_storage_btree_page_load_value(t, page, res.index, value);
    } else {
        ret = _phalcon_storage_btree_page_get(t, res.child, key, value);
        _phalcon_storage_btree_page_destroy(t, res.child);
        res.child = NULL;
        return ret;
    }
}

int _phalcon_storage_btree_page_get_range(phalcon_storage_btree_db_t *t,
                       _phalcon_storage_btree_page_t *page,
                       const phalcon_storage_btree_key_t *start,
                       const phalcon_storage_btree_key_t *end,
                       phalcon_storage_btree_filter_cb filter,
                       phalcon_storage_btree_range_cb cb,
                       void *arg)
{
    int ret;
    uint64_t i;
    _phalcon_storage_btree_page_search_res_t start_res, end_res;

    /* find start and end indexes */
    ret = _phalcon_storage_btree_page_search(t, page, start, kNotLoad, &start_res);
    if (ret != PHALCON_STORAGE_BTREE_OK) return ret;
    ret = _phalcon_storage_btree_page_search(t, page, end, kNotLoad, &end_res);
    if (ret != PHALCON_STORAGE_BTREE_OK) return ret;

    if (page->type == kLeaf) {
        /* on leaf pages end-key should always be greater or equal than first key */
        if (end_res.cmp > 0 && end_res.index == 0) return PHALCON_STORAGE_BTREE_OK;

        if (end_res.cmp < 0) end_res.index--;
    }

    /* go through each page item */
    for (i = start_res.index; i <= end_res.index; i++) {
        /* run filter */
        if (!filter(arg, (phalcon_storage_btree_key_t *) &page->keys[i])) continue;

        if (page->type == kPage) {
            /* load child page and apply range get to it */
            _phalcon_storage_btree_page_t* child;

            ret = _phalcon_storage_btree_page_load(t,
                                page->keys[i].offset,
                                page->keys[i].config,
                                &child);
            if (ret != PHALCON_STORAGE_BTREE_OK) return ret;

            ret = _phalcon_storage_btree_page_get_range(t, child, start, end, filter, cb, arg);

            /* destroy child regardless of error */
            _phalcon_storage_btree_page_destroy(t, child);

            if (ret != PHALCON_STORAGE_BTREE_OK) return ret;
        } else {
            /* load value and pass it to callback */
            phalcon_storage_btree_value_t value;
            ret = _phalcon_storage_btree_page_load_value(t, page, i, &value);
            if (ret != PHALCON_STORAGE_BTREE_OK) return ret;

            cb(arg, (phalcon_storage_btree_key_t *) &page->keys[i], &value);

            efree(value.value);
        }
    }

    return PHALCON_STORAGE_BTREE_OK;
}

int _phalcon_storage_btree_page_insert(phalcon_storage_btree_db_t *t,
                    _phalcon_storage_btree_page_t *page,
                    const phalcon_storage_btree_key_t *key,
                    const phalcon_storage_btree_value_t *value,
                    phalcon_storage_btree_update_cb update_cb,
                    void *arg)
{
    int ret;
    _phalcon_storage_btree_page_search_res_t res;
    ret = _phalcon_storage_btree_page_search(t, page, key, kLoad, &res);
    if (ret != PHALCON_STORAGE_BTREE_OK) return ret;

    if (res.child == NULL) {
        /* store value in db file to get offset and config */
        ret = _phalcon_storage_btree_page_save_value(t,
                                  page,
                                  res.index,
                                  res.cmp,
                                  key,
                                  value,
                                  update_cb,
                                  arg);
        if (ret != PHALCON_STORAGE_BTREE_OK) return ret;
    } else {
        /* Insert kv in child page */
        ret = _phalcon_storage_btree_page_insert(t, res.child, key, value, update_cb, arg);

        /* kv was inserted but page is full now */
        if (ret == PHALCON_STORAGE_BTREE_ESPLITPAGE) {
            ret = _phalcon_storage_btree_page_split(t, page, res.index, res.child);
        } else if (ret == PHALCON_STORAGE_BTREE_OK) {
            /* Update offsets in page */
            page->keys[res.index].offset = res.child->offset;
            page->keys[res.index].config = res.child->config;
        }

        _phalcon_storage_btree_page_destroy(t, res.child);
        res.child = NULL;

        if (ret != PHALCON_STORAGE_BTREE_OK) {
            return ret;
        }
    }

    if (page->length == t->head.page_size) {
        if (page->is_head) {
            ret = _phalcon_storage_btree_page_split_head(t, &page);
            if (ret != PHALCON_STORAGE_BTREE_OK) return ret;
        } else {
            /* Notify caller that it should split page */
            return PHALCON_STORAGE_BTREE_ESPLITPAGE;
        }
    }

    assert(page->length < t->head.page_size);

    ret = _phalcon_storage_btree_page_save(t, page);
    if (ret != PHALCON_STORAGE_BTREE_OK) return ret;

    return PHALCON_STORAGE_BTREE_OK;
}

int _phalcon_storage_btree_page_bulk_insert(phalcon_storage_btree_db_t *t,
                         _phalcon_storage_btree_page_t *page,
                         const phalcon_storage_btree_key_t *limit,
                         uint64_t *count,
                         phalcon_storage_btree_key_t **keys,
                         phalcon_storage_btree_value_t **values,
                         phalcon_storage_btree_update_cb update_cb,
                         void *arg)
{
    int ret;
    _phalcon_storage_btree_page_search_res_t res;

    while (*count > 0 &&
            (limit == NULL || t->compare_cb(limit, *keys) > 0)) {

        ret = _phalcon_storage_btree_page_search(t, page, *keys, kLoad, &res);
        if (ret != PHALCON_STORAGE_BTREE_OK) return ret;

        if (res.child == NULL) {
            /* store value in db file to get offset and config */
            ret = _phalcon_storage_btree_page_save_value(t,
                                      page,
                                      res.index,
                                      res.cmp,
                                      *keys,
                                      *values,
                                      update_cb,
                                      arg);
            /*
             * ignore update conflicts, to handle situations where
             * only one kv failed in a bulk
             */
            if (ret != PHALCON_STORAGE_BTREE_OK && ret != PHALCON_STORAGE_BTREE_EUPDATECONFLICT) return ret;

            *keys = *keys + 1;
            *values = *values + 1;
            *count = *count - 1;
        } else {
            /* we're in regular page */
            phalcon_storage_btree_key_t* new_limit = NULL;

            if (res.index + 1 < page->length) {
                new_limit = (phalcon_storage_btree_key_t*) &page->keys[res.index + 1];
            }

            ret = _phalcon_storage_btree_page_bulk_insert(t,
                                       res.child,
                                       new_limit,
                                       count,
                                       keys,
                                       values,
                                       update_cb,
                                       arg);
            if (ret == PHALCON_STORAGE_BTREE_ESPLITPAGE) {
                ret = _phalcon_storage_btree_page_split(t, page, res.index, res.child);
            } else if (ret == PHALCON_STORAGE_BTREE_OK) {
                /* Update offsets in page */
                page->keys[res.index].offset = res.child->offset;
                page->keys[res.index].config = res.child->config;
            }

            _phalcon_storage_btree_page_destroy(t, res.child);
            res.child = NULL;

            if (ret != PHALCON_STORAGE_BTREE_OK) return ret;
        }

        if (page->length == t->head.page_size) {
            if (page->is_head) {
                ret = _phalcon_storage_btree_page_split_head(t, &page);
                if (ret != PHALCON_STORAGE_BTREE_OK) return ret;
            } else {
                /* Notify caller that it should split page */
                return PHALCON_STORAGE_BTREE_ESPLITPAGE;
            }
        }

        assert(page->length < t->head.page_size);
    }

    return _phalcon_storage_btree_page_save(t, page);
}

int _phalcon_storage_btree_page_remove(phalcon_storage_btree_db_t *t,
                    _phalcon_storage_btree_page_t *page,
                    const phalcon_storage_btree_key_t *key,
                    phalcon_storage_btree_remove_cb remove_cb,
                    void *arg)
{
    int ret;
    _phalcon_storage_btree_page_search_res_t res;
    ret = _phalcon_storage_btree_page_search(t, page, key, kLoad, &res);
    if (ret != PHALCON_STORAGE_BTREE_OK) return ret;

    if (res.child == NULL) {
        if (res.cmp != 0) return PHALCON_STORAGE_BTREE_ENOTFOUND;

        /* remove only if remove_cb returns PHALCON_STORAGE_BTREE_OK */
        if (remove_cb != NULL) {
            phalcon_storage_btree_value_t prev_val;

            ret = _phalcon_storage_btree_page_load_value(t, page, res.index, &prev_val);
            if (ret != PHALCON_STORAGE_BTREE_OK) return ret;

            ret = remove_cb(arg, &prev_val);
            efree(prev_val.value);

            if (!ret) return PHALCON_STORAGE_BTREE_EREMOVECONFLICT;
        }
        _phalcon_storage_btree_page_remove_idx(t, page, res.index);

        if (page->length == 0 && !page->is_head) return PHALCON_STORAGE_BTREE_EEMPTYPAGE;
    } else {
        /* Insert kv in child page */
        ret = _phalcon_storage_btree_page_remove(t, res.child, key, remove_cb, arg);

        if (ret != PHALCON_STORAGE_BTREE_OK && ret != PHALCON_STORAGE_BTREE_EEMPTYPAGE) {
            return ret;
        }

        /* kv was inserted but page is full now */
        if (ret == PHALCON_STORAGE_BTREE_EEMPTYPAGE) {
            _phalcon_storage_btree_page_remove_idx(t, page, res.index);

            /* we don't need child now */
            _phalcon_storage_btree_page_destroy(t, res.child);
            res.child = NULL;

            /* only one item left - lift kv from last child to current page */
            if (page->length == 1) {
                page->offset = page->keys[0].offset;
                page->config = page->keys[0].config;

                /* remove child to efree memory */
                _phalcon_storage_btree_page_remove_idx(t, page, 0);

                /* and load child as current page */
                ret = _phalcon_storage_btree_page_read(t, page);
                if (ret != PHALCON_STORAGE_BTREE_OK) return ret;
            }
        } else {
            /* Update offsets in page */
            page->keys[res.index].offset = res.child->offset;
            page->keys[res.index].config = res.child->config;

            /* we don't need child now */
            _phalcon_storage_btree_page_destroy(t, res.child);
            res.child = NULL;
        }
    }

    return _phalcon_storage_btree_page_save(t, page);
}

int _phalcon_storage_btree_page_copy(phalcon_storage_btree_db_t *source, phalcon_storage_btree_db_t *target, _phalcon_storage_btree_page_t *page)
{
    uint64_t i;
    int ret;
    for (i = 0; i < page->length; i++) {
        if (page->type == kPage) {
            /* copy child page */
            _phalcon_storage_btree_page_t *child;
            ret = _phalcon_storage_btree_page_load(source,
                                page->keys[i].offset,
                                page->keys[i].config,
                                &child);
            if (ret != PHALCON_STORAGE_BTREE_OK) return ret;

            ret = _phalcon_storage_btree_page_copy(source, target, child);
            if (ret != PHALCON_STORAGE_BTREE_OK) return ret;

            /* update child position */
            page->keys[i].offset = child->offset;
            page->keys[i].config = child->config;

            _phalcon_storage_btree_page_destroy(source, child);
        } else {
            /* copy value */
            phalcon_storage_btree_value_t value;

            ret = _phalcon_storage_btree_page_load_value(source, page, i, &value);
            if (ret != PHALCON_STORAGE_BTREE_OK) return ret;

            page->keys[i].config = value.length;
            ret = _phalcon_storage_btree_value_save(target,
                                 &value,
                                 NULL,
                                 &page->keys[i].offset,
                                 &page->keys[i].config);

            /* value is not needed anymore */
            efree(value.value);
            if (ret != PHALCON_STORAGE_BTREE_OK) return ret;
        }
    }

    return _phalcon_storage_btree_page_save(target, page);
}


int _phalcon_storage_btree_page_remove_idx(phalcon_storage_btree_db_t *t, _phalcon_storage_btree_page_t *page, const uint64_t index)
{
    assert(index < page->length);

    /* Free memory allocated for kv and reduce byte_size of page */
    page->byte_size -= PHALCON_STORAGE_BTREE__KV_SIZE(page->keys[index]);
    if (page->keys[index].allocated) {
        efree(page->keys[index].value);
        page->keys[index].value = NULL;
    }

    /* Shift all keys left */
    _phalcon_storage_btree_page_shiftl(t, page, index);

    page->length--;

    return PHALCON_STORAGE_BTREE_OK;
}


int _phalcon_storage_btree_page_split(phalcon_storage_btree_db_t *t,
                   _phalcon_storage_btree_page_t *parent,
                   const uint64_t index,
                   _phalcon_storage_btree_page_t *child)
{
    int ret;
    uint64_t i, middle;
    _phalcon_storage_btree_page_t *left = NULL, *right = NULL;
    _phalcon_storage_btree_kv_t middle_key;

    _phalcon_storage_btree_page_create(t, child->type, 0, 0, &left);
    _phalcon_storage_btree_page_create(t, child->type, 0, 0, &right);

    middle = t->head.page_size >> 1;
    ret = _phalcon_storage_btree_kv_copy(&child->keys[middle], &middle_key, 1);
    if (ret != PHALCON_STORAGE_BTREE_OK) goto fatal;

    /* non-leaf nodes has byte_size > 0 nullify it first */
    left->byte_size = 0;
    left->length = 0;
    for (i = 0; i < middle; i++) {
        ret = _phalcon_storage_btree_kv_copy(&child->keys[i], &left->keys[left->length++], 1);
        if (ret != PHALCON_STORAGE_BTREE_OK) goto fatal;
        left->byte_size += PHALCON_STORAGE_BTREE__KV_SIZE(child->keys[i]);
    }

    right->byte_size = 0;
    right->length = 0;
    for (; i < t->head.page_size; i++) {
        ret = _phalcon_storage_btree_kv_copy(&child->keys[i], &right->keys[right->length++], 1);
        if (ret != PHALCON_STORAGE_BTREE_OK) goto fatal;
        right->byte_size += PHALCON_STORAGE_BTREE__KV_SIZE(child->keys[i]);
    }

    /* save left and right parts to get offsets */
    ret = _phalcon_storage_btree_page_save(t, left);
    if (ret != PHALCON_STORAGE_BTREE_OK) goto fatal;

    ret = _phalcon_storage_btree_page_save(t, right);
    if (ret != PHALCON_STORAGE_BTREE_OK) goto fatal;

    /* store offsets with middle key */
    middle_key.offset = right->offset;
    middle_key.config = right->config;

    /* insert middle key into parent page */
    _phalcon_storage_btree_page_shiftr(t, parent, index + 1);
    _phalcon_storage_btree_kv_copy(&middle_key, &parent->keys[index + 1], 0);

    parent->byte_size += PHALCON_STORAGE_BTREE__KV_SIZE(middle_key);
    parent->length++;

    /* change left element */
    parent->keys[index].offset = left->offset;
    parent->keys[index].config = left->config;

    ret = PHALCON_STORAGE_BTREE_OK;
fatal:
    /* cleanup */
    _phalcon_storage_btree_page_destroy(t, left);
    _phalcon_storage_btree_page_destroy(t, right);
    return ret;
}

int _phalcon_storage_btree_page_split_head(phalcon_storage_btree_db_t *t, _phalcon_storage_btree_page_t **page)
{
    int ret;
    _phalcon_storage_btree_page_t *new_head = NULL;
    _phalcon_storage_btree_page_create(t, 0, 0, 0, &new_head);
    new_head->is_head = 1;

    ret = _phalcon_storage_btree_page_split(t, new_head, 0, *page);
    if (ret != PHALCON_STORAGE_BTREE_OK) {
        _phalcon_storage_btree_page_destroy(t, new_head);
        return ret;
    }

    t->head.page = new_head;
    _phalcon_storage_btree_page_destroy(t, *page);
    *page = new_head;

    return PHALCON_STORAGE_BTREE_OK;
}

void _phalcon_storage_btree_page_shiftr(phalcon_storage_btree_db_t *t, _phalcon_storage_btree_page_t *p, const uint64_t index)
{
    uint64_t i;
    if (p->length != 0) {
        for (i = p->length - 1; i >= index; i--) {
            _phalcon_storage_btree_kv_copy(&p->keys[i], &p->keys[i + 1], 0);

            if (i == 0) break;
        }
    }
}

void _phalcon_storage_btree_page_shiftl(phalcon_storage_btree_db_t *t, _phalcon_storage_btree_page_t *p, const uint64_t index)
{
    uint64_t i;
    for (i = index + 1; i < p->length; i++) {
        _phalcon_storage_btree_kv_copy(&p->keys[i], &p->keys[i - 1], 0);
    }
}
