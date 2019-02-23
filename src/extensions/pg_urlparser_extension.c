/*
 * pg_urlparser_extension.c
 *
 *  Created on: 2019-2-23
 *      Author: rengaohe
 */

#include <stdio.h>

#include "pg_types.h"
#include "util/at_util.h"
#include "http/at_httpclient_pool.h"
#include "http/at_httpclient.h"
#include "pg_page_util.h"

static at_boolean_t pg_urlparser_init (at_hashmap_t *ctx) {
    return AT_TRUE;
}

static at_boolean_t pg_parse_url (at_hashmap_t *ctx, pg_page_t *page) {
    at_boolean_t ret = AT_TRUE;
    char *html = string_cstr(page->body);
    char url[MAX_URL_LEN] = {0};
    int url_len = pg_extract_url(&html, url, MAX_URL_LEN);
    while (url_len > 0) {
        printf("%s\n", url);
        memset(url, 0, MAX_URL_LEN);
        url_len = pg_next_url(url, MAX_URL_LEN);
    }
    return ret;
}

void pg_urlparser_exit(at_hashmap_t *ctx) {
}

pg_extension_t pg_urlparser_extension = {
        "urlparser_extension",
        pg_urlparser_init,
        pg_parse_url,
        pg_urlparser_exit
};
