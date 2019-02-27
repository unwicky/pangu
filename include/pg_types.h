/*
 * pg_types.h
 *
 *  Created on: 2019-2-21
 *      Author: rengaohe
 */


#ifndef PG_TYPES_H_
#define PG_TYPES_H_

#include <zlog.h>

#include "util/at_hashmap.h"
#include "util/at_string.h"

#define MAX_URL_LEN 128
#define SPIDER_NUM 4
#define IS_NOT_EMPTY(s) *s != ' ' && *s != '\t' && *s != '\r' && *s != '\n'
#define IS_EMPTY(s) *s == ' ' || *s == '\t' || *s == '\r' || *s == '\n'

typedef struct {
    int tag;
    char url[MAX_URL_LEN];
    int depth;
    at_hashmap_t *heads;
    at_string_t *body;
    at_hashmap_t *ctx;
} pg_page_t;

typedef at_boolean_t (*pg_extension_init)(at_hashmap_t *);
typedef at_boolean_t (*pg_extension_handle)(at_hashmap_t *, pg_page_t *);
typedef void (*pg_extension_exit)(at_hashmap_t *);

typedef struct {
    char name[64];
    pg_extension_init init;
    pg_extension_handle handle;
    pg_extension_exit exit;
} pg_extension_t;

#endif
