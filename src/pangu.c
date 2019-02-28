/*
 * pangu.c
 *
 *  Created on: 2019-2-21
 *      Author: rengaohe
 */

#include <stdio.h>

#include "pg_types.h"
#include "util/at_hashmap.h"
#include "util/at_stack.h"
#include "util/at_thread_pool.h"

#define INIT_CONTAINER_SIZE 16

pthread_mutex_t lock;
extern pg_extension_t *extensions[];

static void free_crawl_task(at_task_t **task_ptr) {
    at_task_t *task = *task_ptr;
    free(task);
    *task_ptr = NULL;
}

static void *do_crawl(void *ptr) {
    int i = 0;
    pg_page_t *page = (pg_page_t *)ptr;
    for (i = 0; extensions[i]; i++) {
        if (!extensions[i]->handle(page->ctx, page)) {
            break;
        }
    }
    if (!page->heads) {
        hashmap_free(&page->heads);
    }
    if (!page->body) {
        string_free(&page->body);
    }
    free(page);
    return NULL;
}

static at_task_t *new_crawl_task(at_hashmap_t *ctx, at_stack_t *pages) {
    at_task_t *task = (at_task_t *)malloc(sizeof(at_task_t));
    pthread_mutex_lock(&lock);
    pg_page_t *page = (pg_page_t *)stack_pop(pages);
    //printf("url=%s,depth=%d\n", page->url, page->depth);
    pthread_mutex_unlock(&lock);
    task->run = do_crawl;
    task->params = page;
    task->free_task = free_crawl_task;
    return task;
}

static at_boolean_t init_extensions(at_hashmap_t *ctx) {
    int i = 0;
    for (i = 0; extensions[i]; i++) {
        if (extensions[i]->init) {
            if (!extensions[i]->init(ctx)) {
                return AT_FALSE;
            }
        }
    }
    return AT_TRUE;
}

static void exit_extensions(at_hashmap_t *ctx) {
    int i = 0;
    for (i = 0; extensions[i]; i++) {
        if (extensions[i]->exit) {
            extensions[i]->exit(ctx);
        }
    }
}

void main() {
    at_hashmap_t *ctx = hashmap_new(INIT_CONTAINER_SIZE, STRINGTYPE, OBJECTTYPE);
    at_stack_t *pages = stack_new(INIT_CONTAINER_SIZE);
    pg_page_t *page = (pg_page_t *)malloc(sizeof(pg_page_t));
    snprintf(page->url, MAX_URL_LEN, "https://www.huawei.com");
    page->ctx = ctx;
    page->depth = 1;
    stack_push(pages, page);
    hashmap_insert(ctx, "pageset", pages);
    pthread_mutex_init(&lock, NULL);
    hashmap_insert(ctx, "thread_lock", &lock);
    if (!init_extensions(ctx)) {
        printf("initialize extensions failed");
        exit(1);
    }
    at_thread_pool_t *spiders = thread_pool_create(SPIDER_NUM);
    while (!stack_isempty(pages) || !thread_pool_is_all_idle(spiders)) {
        if (!stack_isempty(pages)) {
            at_task_t *task = new_crawl_task(ctx, pages);
            thread_pool_add_task(spiders, task);
        } else {
		    printf("hi,i'm a new committer\n");
            usleep(10000);
        }
    }   
    thread_pool_stop(&spiders);
    exit_extensions(ctx);
    pthread_mutex_destroy(&lock);
    stack_free(&pages); 
    hashmap_free(&ctx);
}
