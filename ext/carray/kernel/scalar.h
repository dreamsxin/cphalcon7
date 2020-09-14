#ifndef PHPSCI_EXT_SCALAR_H
#define PHPSCI_EXT_SCALAR_H

#include "carray.h"

/**
 * SCALARS STRUCTURES
 */ 
typedef struct {
        int  type;
        int  obval;
} CArrayIntegerScalarObject;

typedef struct {
        int  type;
        float obval;
} CArrayFloatScalarObject;

typedef struct {
        int  type;
        double obval;
} CArrayDoubleScalarObject;

typedef struct {
        int  type;
        double obval;
} CArrayLongScalarObject;

typedef struct {
    int  type;
    char * obval;    
} CArrayScalar;

CArrayDescriptor * CArray_DescrFromScalar(CArrayScalar *sc);
CArrayScalar * CArrayScalar_FromZval(PHPObject * obj, int is_double, int is_long);
void CArrayScalar_FREE(CArrayScalar * sc);
CArrayScalar * CArrayScalar_NewInt(int sc);
CArrayScalar * CArrayScalar_NewDouble(double sc);
CArrayScalar * CArrayScalar_NewFloat(float sc);
CArrayScalar * CArrayScalar_NewLong(long sc);
void * scalar_value(CArrayScalar *scalar, CArrayDescriptor *descr);
#endif //PHPSCI_EXT_SCALAR_H
