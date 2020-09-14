//
// Created by Henrique Borba on 19/11/2018.
//

#ifndef PHPSCI_EXT_ALLOC_H
#define PHPSCI_EXT_ALLOC_H

#include "carray.h"

void CArray_Data_alloc(CArray * ca);
void * carray_data_alloc_zeros(int num_elements, int size_element, char type);
void * carray_data_alloc(int num_elements, int size_element);

void CArray_INCREF(CArray * target);
void CArray_DECREF(CArray * target);
void CArrayDescriptor_INCREF(CArrayDescriptor * descriptor);
void CArrayDescriptor_DECREF(CArrayDescriptor * descriptor);
void CArray_Alloc_FreeFromMemoryPointer(MemoryPointer * ptr);
CArray * CArray_Alloc(CArrayDescriptor *descr, int nd, int* dims, int is_fortran, void *interfaceData);
void CArray_Free(CArray * self);
void CArrayDescriptor_FREE(CArrayDescriptor * descr);
#endif //PHPSCI_EXT_ALLOC_H
