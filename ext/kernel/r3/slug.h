/*
 * slug.h
 * Copyright (C) 2014 c9s <yoanlin93@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */
#ifndef SLUG_H
#define SLUG_H

typedef struct {
    /**
     * source path
     */
    const char * path;

    int path_len;

    /**
     * slug start pointer
     */
    const char * begin;

    /**
     * slug end pointer
     */
    const char * end;

    /**
     * slug length
     */
    int len;

    // slug pattern pointer if we have one
    const char * pattern;

    // the length of custom pattern, if the pattern is found.
    int    pattern_len;

} r3_slug_t;


r3_slug_t * r3_slug_new(const char * path, int path_len);

int r3_slug_check(r3_slug_t *s);

int r3_slug_parse(r3_slug_t *s, const char *needle, int needle_len, const char *offset, char **errstr);

char * r3_slug_to_str(const r3_slug_t *s);

void r3_slug_free(r3_slug_t * s);

static inline int r3_path_contains_slug_char(const char *str, unsigned int len) {
    for (unsigned int i = 0; i < len; i++) {
        if (str[i] == '{') return 1;
    }
    return 0;
}

#endif /* !SLUG_H */
