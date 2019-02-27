/*
 * pg_page_util.c
 *
 *  Created on: 2019-2-21
 *      Author: rengaohe
 */

#include <stdlib.h>
#include <string.h>

#include "pg_page_util.h"
#include "pg_types.h"

static char *html;

int pg_get_text_line (char **html_ptr, char *text_line, int max_len) {
    int len = 0;
    char *html = *html_ptr;
    while (*html != '\0') {
        if (*html == '<') {
            for (; *html != '\0' && *html != '>'; html++, (*html_ptr)++);
            if (*html == '\0') {
                return -1;
            }
        } else {
            if (*html == '\n') {
                (*html_ptr)++;
                text_line[len] = '\0';
                return len;
            } else {
                if (len < max_len) {
                    text_line[len++] = *html;
                }
            }
        }
        html++;
        (*html_ptr)++;
    }
    return -1;
}

int pg_get_text_in_span (char **html_ptr, char *text, int max_len) {
    char *html = *html_ptr;
    int count = 0;
    for ( ; *html != '<' || strncmp(html, "<span", 5) != 0; html++);
    html += 5;
    for ( ; *html != '>'; html++);
    html++;
    while (*html != '<' || strncmp(html, "</span>", 7) != 0) {
        if (IS_NOT_EMPTY(html)) {
            text[count++] = *html;
            if (count >= max_len) {
                break;
            }
        }
        html++;
    }
    (*html_ptr) += (html - *html_ptr);
    return count;
}

int pg_get_line (char **html_ptr, char *text_line, int max_len) {
    int len = 0;
    char *html = *html_ptr;
    while (*html != '\0') {
        if (*html == '\n') {
            (*html_ptr)++;
            text_line[len] = '\0';
            return len;
        } else {
            if (len < max_len) {
                text_line[len++] = *html;
            }
        }
        html++;
        (*html_ptr)++;
    }
    return -1;
}

int pg_url_relative_to_absolute (const char *parent_url, char *url,
        int url_len, int len_limit) {
    if (strncmp(url, "http://", 7) == 0 || strncmp(url, "https://", 8) == 0) {
        return url_len;
    }
    int terminal_indexes[100] = {0}, seg_num = 0, real_level = 0, domain_len = 0, new_url_len = 0;
    const char *parent_url_cursor = parent_url, *url_cursor = url;
    char tmp[MAX_URL_LEN] = {0};
    if (strncmp(parent_url_cursor, "http://", 7) == 0) {
        parent_url_cursor += 7;
    } else if (strncmp(parent_url_cursor, "https://", 8) == 0) {
        parent_url_cursor += 8;
    }
    while (*parent_url_cursor != '\0') {
        if (*parent_url_cursor == '/') {
                terminal_indexes[seg_num++] = parent_url_cursor - parent_url; 
        }
        parent_url_cursor++;
    }
    if (*url_cursor == '/') {
        url_cursor++;
        domain_len = terminal_indexes[0];
    } else {
        while (*url_cursor == '.') {
            if (strncmp(url_cursor, "./", 2) == 0) {
                url_cursor += 2;
                url_len -= 2;
            } else if (strncmp(url_cursor, "../", 3) == 0) {
                url_cursor += 3;
                url_len -= 3;
                real_level++;
            } else {
                break;
            }
        }
        if (seg_num <= real_level) {
            domain_len = terminal_indexes[0];
        } else {
            domain_len = terminal_indexes[seg_num - real_level - 1];
        }
    }
    if (domain_len == 0) {
        domain_len = strlen(parent_url);
        memcpy(tmp, parent_url, domain_len);
        tmp[domain_len] = '/';
    } else {
        memcpy(tmp, parent_url, domain_len + 1);
    }
    memcpy(tmp + domain_len + 1, url_cursor, url_len);
    new_url_len = (domain_len + url_len + 1) > len_limit ? len_limit : (domain_len + url_len + 1);
    memcpy(url, tmp, new_url_len);
    url[new_url_len++] = '\0';
    return new_url_len;
}

int pg_extract_url(char **html_ptr, char *url, int len) {
    if (!html_ptr || !(*html_ptr) || !url) {
        return -1;
    }
    html = *html_ptr;
    return pg_next_url(url, len);
}

int pg_next_url(char *url, int len) {
    if (!url) {
        return -1;
    }
    int exact_len = -1;
    while (*html != '\0') {
        for (; *html != '<' && *html != '\0'; html++);
        if (*html == '\0') {
            break;
        }
        html++;
        if (*html == 'a') {
            html++;
            for (; *html != 'h' && *html != '\0'; html++);
            if (*html == '\0') {
                break;
            }
            if (strncmp(html, "href", 4) == 0) {
                html += 4;
                for (; *html != '='; html++);
                html++;
                for (; *html == ' ' || *html == '\t' || *html == '\r'
                        || *html == '\n' || *html == '\"'
                        || *html == '\''; html++);
                exact_len = 0;
                for (; *html != ' ' && *html != '\t' && *html != '\"'
                        && *html != '\'' && *html != '>'; html++) {
                    if (exact_len >= len) {
                        break;
                    }
                    url[exact_len++] = *html;
                }
                url[exact_len] = '\0';
                if (exact_len > 0) {
                    break;
                }
            }
        }
    }
    return exact_len;
}
