/*
 * pg_extensions.c
 *
 *  Created on: 2019-2-21
 *      Author: rengaohe
 */

#include <stdlib.h>

#include "pg_types.h"

extern pg_extension_t pg_download_extension;
extern pg_extension_t pg_urlparser_extension;

pg_extension_t *extensions[] = {
       &pg_download_extension,
       &pg_urlparser_extension,
       NULL
};
