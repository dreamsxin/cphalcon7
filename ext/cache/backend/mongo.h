
/*
  +------------------------------------------------------------------------+
  | Phalcon Framework                                                      |
  +------------------------------------------------------------------------+
  | Copyright (c) 2011-2014 Phalcon Team (http://www.phalconphp.com)       |
  +------------------------------------------------------------------------+
  | This source file is subject to the New BSD License that is bundled     |
  | with this package in the file docs/LICENSE.txt.                        |
  |                                                                        |
  | If you did not receive a copy of the license and are unable to         |
  | obtain it through the world-wide-web, please send an email             |
  | to license@phalconphp.com so we can send you a copy immediately.       |
  +------------------------------------------------------------------------+
  | Authors: Andres Gutierrez <andres@phalconphp.com>                      |
  |          Eduar Carvajal <eduar@phalconphp.com>                         |
  +------------------------------------------------------------------------+
*/

#ifndef PHALCON_CACHE_BACKEND_MONGO_H
#define PHALCON_CACHE_BACKEND_MONGO_H

#include "php_phalcon.h"

#ifdef PHALCON_USE_MONGOC
#include <bson.h>
#include <bcon.h>
#include <mongoc.h>

typedef struct {
	mongoc_client_t *client;
	mongoc_collection_t  *collection;
	zend_object std;
} phalcon_cache_backend_mongo_object;

static inline phalcon_cache_backend_mongo_object *phalcon_cache_backend_mongo_object_from_obj(zend_object *obj) {
	return (phalcon_cache_backend_mongo_object*)((char*)(obj) - XtOffsetOf(phalcon_cache_backend_mongo_object, std));
}

#endif

extern zend_class_entry *phalcon_cache_backend_mongo_ce;

PHALCON_INIT_CLASS(Phalcon_Cache_Backend_Mongo);

#endif /* PHALCON_CACHE_BACKEND_MONGO_H */
