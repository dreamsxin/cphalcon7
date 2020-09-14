#ifndef PHPSCI_CARRAY_RUBIX_H
#define PHPSCI_CARRAY_RUBIX_H

#include "../carray.h"

static zend_class_entry *crubix_sc_entry;
static zend_object_handlers crubix_object_handlers;
static zend_class_entry *crubix_exception_sc_entry;
static zend_class_entry *crubix_iterator_sc_entry;
extern zend_module_entry crubix_module_entry;

#define phpext_crubix_ptr &crubix_module_entry

/**
 * Rubix-ML CArray Interface Definition
 */
PHP_METHOD(CRubix, identity);               //OK    PASSED
PHP_METHOD(CRubix, zeros);                  //OK    PASSED
PHP_METHOD(CRubix, ones);                   //OK    PASSED
PHP_METHOD(CRubix, diagonal);               //OK    PASSED
PHP_METHOD(CRubix, fill);                   //OK    PASSED
PHP_METHOD(CRubix, rand);
PHP_METHOD(CRubix, gaussian);
PHP_METHOD(CRubix, uniform);
PHP_METHOD(CRubix, minimum);                //OK    PASSED
PHP_METHOD(CRubix, maximum);                //OK    PASSED
PHP_METHOD(CRubix, stack);
PHP_METHOD(CRubix, implodeRow);
PHP_METHOD(CRubix, shape);                  //OK    PASSED
PHP_METHOD(CRubix, size);                   //OK    PASSED
PHP_METHOD(CRubix, m);                      //OK    PASSED
PHP_METHOD(CRubix, n);                      //OK    PASSED
PHP_METHOD(CRubix, diagonalAsVector);       //OK    PASSED
PHP_METHOD(CRubix, flatten);                //OK    PASSED
PHP_METHOD(CRubix, argmin);                 //OK    PASSED
PHP_METHOD(CRubix, argmax);                 //OK    PASSED
PHP_METHOD(CRubix, map);
PHP_METHOD(CRubix, inverse);                //OK    PASSED
PHP_METHOD(CRubix, det);                    //OK    PASSED
PHP_METHOD(CRubix, trace);
PHP_METHOD(CRubix, rank);
PHP_METHOD(CRubix, fullRank);
PHP_METHOD(CRubix, symmetric);              //OK    PASSED
PHP_METHOD(CRubix, matmul);                 //OK    PASSED
PHP_METHOD(CRubix, dot);                    //OK    PASSED
PHP_METHOD(CRubix, ref);
PHP_METHOD(CRubix, rref);
PHP_METHOD(CRubix, lu);
PHP_METHOD(CRubix, eig);                    //OK    PASSED
PHP_METHOD(CRubix, solve);                  //OK    PASSED
PHP_METHOD(CRubix, l1Norm);
PHP_METHOD(CRubix, infinityNorm);
PHP_METHOD(CRubix, maxNorm);
PHP_METHOD(CRubix, multiply);               //OK     PASSED
PHP_METHOD(CRubix, divide);                 //OK     PASSED
PHP_METHOD(CRubix, add);                    //OK     PASSED
PHP_METHOD(CRubix, subtract);               //OK     PASSED
PHP_METHOD(CRubix, pow);                    //OK     PASSED
PHP_METHOD(CRubix, mod);                    //OK     PASSED
PHP_METHOD(CRubix, greaterEqual);           //OK     PASSED
PHP_METHOD(CRubix, reciprocal);         //OK        PASSED
PHP_METHOD(CRubix, abs);                //OK        PASSED
PHP_METHOD(CRubix, sqrt);               //OK        PASSED
PHP_METHOD(CRubix, exp);                //OK        PASSED
PHP_METHOD(CRubix, expm1);              //OK        PASSED
PHP_METHOD(CRubix, log);                //CAUTION   PASSED  MEMFAULT
PHP_METHOD(CRubix, log1p);              //OK        PASSED
PHP_METHOD(CRubix, sin);                //OK        PASSED
PHP_METHOD(CRubix, asin);               //OK        PASSED
PHP_METHOD(CRubix, cos);                //OK        PASSED
PHP_METHOD(CRubix, acos);               //OK        PASSED
PHP_METHOD(CRubix, tan);                //OK        PASSED
PHP_METHOD(CRubix, atan);               //OK        PASSED
PHP_METHOD(CRubix, rad2deg);
PHP_METHOD(CRubix, deg2rad);
PHP_METHOD(CRubix, sum);                //OK        PASSED
PHP_METHOD(CRubix, product);            //OK        PASSED
PHP_METHOD(CRubix, min);                //OK        PASSED
PHP_METHOD(CRubix, max);                //OK        PASSED
PHP_METHOD(CRubix, variance);
PHP_METHOD(CRubix, median);
PHP_METHOD(CRubix, quantile);
PHP_METHOD(CRubix, covariance);         //OK    PASSED
PHP_METHOD(CRubix, round);
PHP_METHOD(CRubix, floor);              //OK    PASSED
PHP_METHOD(CRubix, ceil);               //OK    PASSED
PHP_METHOD(CRubix, clip);
PHP_METHOD(CRubix, clipLower);
PHP_METHOD(CRubix, clipUpper);
PHP_METHOD(CRubix, sign);
PHP_METHOD(CRubix, negate);             //OK    PASSED
PHP_METHOD(CRubix, insert);
PHP_METHOD(CRubix, subMatrix);          //OK    PASSED
PHP_METHOD(CRubix, augmentAbove);
PHP_METHOD(CRubix, augmentBelow);
PHP_METHOD(CRubix, augmentLeft);
PHP_METHOD(CRubix, augmentRight);
PHP_METHOD(CRubix, repeat);
PHP_METHOD(CRubix, modMatrix);
PHP_METHOD(CRubix, equalMatrix);                    //OK    PASSED
PHP_METHOD(CRubix, notEqualMatrix);                 //OK    PASSED
PHP_METHOD(CRubix, greaterMatrix);                  //OK     PASSED
PHP_METHOD(CRubix, greaterEqualMatrix);             //OK     PASSED
PHP_METHOD(CRubix, lessMatrix);                     //OK    PASSED
PHP_METHOD(CRubix, lessEqualMatrix);                //OK    PASSED
PHP_METHOD(CRubix, modVector);
PHP_METHOD(CRubix, equalVector);                    //OK    PASSED
PHP_METHOD(CRubix, notEqualVector);                 //OK    PASSED
PHP_METHOD(CRubix, greaterVector);                  //OK     PASSED
PHP_METHOD(CRubix, greaterEqualVector);             //OK     PASSED
PHP_METHOD(CRubix, lessVector);                     //OK    PASSED
PHP_METHOD(CRubix, lessEqualVector);                //OK    PASSED
PHP_METHOD(CRubix, modColumnVector);                
PHP_METHOD(CRubix, equalColumnVector);              //OK    PASSED
PHP_METHOD(CRubix, notEqualColumnVector);           //OK    PASSED
PHP_METHOD(CRubix, greaterColumnVector);            //OK     PASSED
PHP_METHOD(CRubix, greaterEqualColumnVector);       //OK     PASSED
PHP_METHOD(CRubix, lessColumnVector);               //OK    PASSED
PHP_METHOD(CRubix, lessEqualColumnVector);          //OK    PASSED
PHP_METHOD(CRubix, modScalar);
PHP_METHOD(CRubix, equalScalar);                    //OK    PASSED
PHP_METHOD(CRubix, notEqualScalar);
PHP_METHOD(CRubix, greaterScalar);                  //OK     PASSED
PHP_METHOD(CRubix, greaterEqualScalar);             //OK     PASSED
PHP_METHOD(CRubix, lessScalar);                     //OK    PASSED
PHP_METHOD(CRubix, lessEqualScalar);                //OK    PASSED
PHP_METHOD(CRubix, count);
PHP_METHOD(CRubix, offsetSet);
PHP_METHOD(CRubix, offsetExists);
PHP_METHOD(CRubix, offsetUnset);
PHP_METHOD(CRubix, offsetGet);
PHP_METHOD(CRubix, getIterator);
PHP_METHOD(CRubix, transpose);              //OK    PASSED
PHP_METHOD(CRubix, reshape);                //OK    PASSED
#endif //PHPSCI_CARRAY_RUBIX_H
