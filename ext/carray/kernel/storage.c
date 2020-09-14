#include "carray.h"
#include "storage.h"
#include "buffer.h"

/**
 * Save CArrray to binary file
 *
 * @param filename
 * @param target
 * @return 1 - Success or 0 error
 */
int
CArrayStorage_SaveBin(char * filename, CArray *target)
{
    char * fullname = emalloc(sizeof(char) * (strlen(filename) + 4));

    strcpy(fullname, filename);
    strcat(fullname, ".carray");

    FILE* fp = fopen(fullname, "wb");

    if (fp == NULL || fp == 0) {
        return 0;
    }

    assert(fp != 0);

    fwrite(&(target->ndim), sizeof(int), 1, fp);
    fwrite(target->dimensions, sizeof(int) * CArray_NDIM(target), 1, fp);
    fwrite(target->strides, sizeof(int), CArray_NDIM(target), fp);
    fwrite(&(target->flags), sizeof(int), 1, fp);
    fwrite(&(target->descriptor->type_num), sizeof(int), 1, fp);
    fwrite(target->data, CArray_DESCR(target)->elsize, CArray_MultiplyList(CArray_DIMS(target), CArray_NDIM(target)), fp);

    fclose(fp);
    efree(fullname);

    return 1;
}

int
CArrayStorage_LoadBin(char *filename, MemoryPointer *out)
{
    CArrayDescriptor *descr;
    CArray *array;
    char * fullname = emalloc(sizeof(char) * (strlen(filename) + 4));
    int type_num;

    strcpy(fullname, filename);
    strcat(fullname, ".carray");

    FILE* fp = fopen(fullname, "r");

    if (fp == NULL || fp == 0) {
        return 0;
    }

    assert(fp != 0);

    array = emalloc(sizeof(CArray));
    fread(&(array->ndim), sizeof(int), 1, fp);

    array->dimensions = emalloc(sizeof(int) * array->ndim);
    fread(array->dimensions, sizeof(int) * array->ndim, 1, fp);

    array->strides = emalloc(sizeof(int) * array->ndim);
    fread(array->strides, sizeof(int), array->ndim, fp);
    fread(&(array->flags), sizeof(int), 1, fp);
    fread(&type_num, sizeof(int), 1, fp);

    descr = CArray_DescrFromType(type_num);
    array->descriptor = descr;

    array->data = emalloc(array->descriptor->elsize * CArray_MultiplyList(CArray_DIMS(array), CArray_NDIM(array)));
    fread(array->data, array->descriptor->elsize, CArray_MultiplyList(CArray_DIMS(array), CArray_NDIM(array)), fp);

    array->descriptor->numElements =  CArray_MultiplyList(CArray_DIMS(array), CArray_NDIM(array));
    array->refcount = 0;


    array->flags |= CARRAY_ARRAY_OWNDATA;

    add_to_buffer(out, array, sizeof(CArray));
    efree(fullname);
    return 1;
}