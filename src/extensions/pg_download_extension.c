/*
 * pg_download_extension.c
 *
 *  Created on: 2019-2-23
 *      Author: rengaohe
 */

#include <stdio.h>

#include "pg_types.h"
#include "util/at_util.h"
#include "http/at_httpclient_pool.h"
#include "http/at_httpclient.h"

static at_boolean_t pg_download_init (at_hashmap_t *ctx) {
    at_httpclient_pool_t *client_pool = httpclient_pool_create(SPIDER_NUM);
    hashmap_insert(ctx, "httpclient_pool", client_pool);
    if (zlog_init("conf/zlog.conf") != 0) {
        printf("init zlog failed");
        return AT_FALSE;
    }
    zlog_category_t *zc = zlog_get_category("run");
    if (!zc) {
        printf("get zlog category failed");
        return AT_FALSE;
    }
    hashmap_insert(ctx, "zlog_category", zc);
    return AT_TRUE;
}

static at_boolean_t pg_download (at_hashmap_t *ctx, pg_page_t *page) {
    at_boolean_t ret = AT_TRUE;
    zlog_category_t *zc = (zlog_category_t *)hashmap_get(ctx, "zlog_category");
    at_httpclient_pool_t *client_pool = (at_httpclient_pool_t *)hashmap_get(ctx, "httpclient_pool");
    at_httpclient_t *client = (at_httpclient_t *)httpclient_pool_get(client_pool);
    if (!client) {
	zlog_error(zc, "can't get httpclient");
	return AT_FALSE;
    }
    at_httpresponse_t *resp = httpclient_get(client, page->url);
    if (resp->op_code != 0) {
        zlog_error(zc, "download %s failed: code=%d", page->url, resp->op_code);
	ret = AT_FALSE;
    } else {
	if (resp->http_status != HTTP_OK) {
            zlog_error (zc, "download %s failed: respcode=%d", page->url, resp->http_status);
            ret = AT_FALSE;
	} else {
            page->body = string_new(string_cstr(resp->ret_str));
	}
    }
    httpclient_pool_return(client_pool, client);
    return ret;
}

void pg_download_exit(at_hashmap_t *ctx) {
    at_httpclient_pool_t *client_pool = (at_httpclient_pool_t *)hashmap_get(ctx, "httpclient_pool");
    httpclient_pool_destroy(&client_pool);
    zlog_fini();
}

pg_extension_t pg_download_extension = {
        "download_extension",
        pg_download_init,
        pg_download,
        pg_download_exit
};
