/*
 * pg_page_util.h
 *
 *  Created on: 2019-2-21
 *      Author: rengaohe
 */

#ifndef PG_PAGE_UTIL_H_
#define PG_PAGE_UTIL_H_

int pg_get_text_line (char **, char *, int);

int pg_get_text_in_span(char **, char *, int);

int pg_get_line (char **, char *, int);

int pg_url_relative_to_absolute (const char *, char *, int , int );

int pg_extract_url(char **, char *, int);

int pg_next_url(char **, char *, int);

#endif /* PG_PAGE_UTIL_H_ */
