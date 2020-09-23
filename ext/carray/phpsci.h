/*
  +----------------------------------------------------------------------+
  | PHPSci CArray                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 2018 PHPSci Team                                       |
  +----------------------------------------------------------------------+
  | Licensed under the Apache License, Version 2.0 (the "License");      |
  | you may not use this file except in compliance with the License.     |
  | You may obtain a copy of the License at                              |
  |                                                                      |
  |     http://www.apache.org/licenses/LICENSE-2.0                       |
  |                                                                      |
  | Unless required by applicable law or agreed to in writing, software  |
  | distributed under the License is distributed on an "AS IS" BASIS,    |
  | WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or      |
  | implied.                                                             |
  | See the License for the specific language governing permissions and  |
  | limitations under the License.                                       |
  +----------------------------------------------------------------------+
  | Authors: Henrique Borba <henrique.borba.dev@gmail.com>               |
  +----------------------------------------------------------------------+
*/

#ifndef CARRAY_EXT_PHPSCI_H
#define CARRAY_EXT_PHPSCI_H

#define PHP_CARRAY_EXTNAME "CArray"
#define PHP_CARRAY_VERSION "0.0.1"


#include "php_phalcon.h"
#include "carray/kernel/carray.h"


extern zend_class_entry *carray_sc_entry;

void RETURN_MEMORYPOINTER(zval * return_value, MemoryPointer * ptr);
int * ZVAL_TO_TUPLE(zval * obj, int * size);
void ZVAL_TO_MEMORYPOINTER(zval * obj, MemoryPointer * ptr, char * type);
void RETURN_RUBIX_MEMORYPOINTER(zval * return_value, MemoryPointer * ptr);
void FREE_FROM_MEMORYPOINTER(MemoryPointer * ptr);
void FREE_TUPLE(int * tuple);
zval * MEMORYPOINTER_TO_ZVAL(MemoryPointer * ptr);

void carray_ce_register();

#endif //PHPSCI_EXT_PHPSCI_H
