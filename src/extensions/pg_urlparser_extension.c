/*
 * pg_urlparser_extension.c
 *
 *  Created on: 2019-2-23
 *      Author: rengaohe
 */

#include <stdio.h>

#include "pg_types.h"
#include "util/at_hashset.h"
#include "util/at_util.h"
#include "http/at_httpclient_pool.h"
#include "http/at_httpclient.h"
#include "pg_page_util.h"

#define INIT_URL_SET_SIZE 10240
#define MAX_DEPTH 3

static at_hashset_t *VISITED_URL_SET;

static at_boolean_t pg_urlparser_init (at_hashmap_t *ctx) {
    VISITED_URL_SET = hashset_new(INIT_URL_SET_SIZE, STRINGTYPE);
    return AT_TRUE;
}

static at_boolean_t pg_parse_url (at_hashmap_t *ctx, pg_page_t *page) {
    at_boolean_t ret = AT_TRUE;
    if (page->depth >= MAX_DEPTH) {
        return ret;
    }
    char *html = string_cstr(page->body);
    char url[MAX_URL_LEN] = {0}, *visited_url;
    int url_len = pg_extract_url(&html, url, MAX_URL_LEN - 1);
    at_stack_t *pageset = (at_stack_t *)hashmap_get(ctx, "pageset");
    pthread_mutex_t *lock = (pthread_mutex_t *)hashmap_get(ctx, "thread_lock"); 
    while (url_len > 0) {
        if (!strstr(url, "javascript") && !strstr(url, "mailto:")) {
            url_len = pg_url_relative_to_absolute(page->url, url, url_len, MAX_URL_LEN - 1);
            if (!hashset_iscontain(VISITED_URL_SET, url)) {
                pg_page_t *another_page = (pg_page_t *)calloc(1, sizeof(pg_page_t));
                another_page->ctx = ctx;
                another_page->depth = page->depth + 1;
                memcpy(another_page->url, url, url_len);
                pthread_mutex_lock(lock);
                visited_url = (char *)calloc(url_len + 1, sizeof(char));
                memcpy(visited_url, url, url_len);
                hashset_insert(VISITED_URL_SET, visited_url);
                stack_push(pageset, another_page);
                pthread_mutex_unlock(lock);
            }
        }
        url_len = pg_next_url(url, MAX_URL_LEN);
    }
    return ret;
}

void pg_urlparser_exit(at_hashmap_t *ctx) {
    void **data_array = hashset_array(VISITED_URL_SET), **head;
    void *temp = NULL;
    if (data_array) {
        head = data_array;
        while ((temp = *data_array) != NULL) {
            free(temp);
            data_array++;
        }
        free(head);
    }
    hashset_free(&VISITED_URL_SET);
}

pg_extension_t pg_urlparser_extension = {
        "urlparser_extension",
        pg_urlparser_init,
        pg_parse_url,
        pg_urlparser_exit
};
