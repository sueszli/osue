#include <stdlib.h>

#include "dynarray.h"

int dyn_new(dynarray **array) {
    dynarray *new = (dynarray*) malloc(sizeof (dynarray));
    
    if (new == NULL) return DYN_ERROR;
    
    new->array = (float *) malloc(sizeof (float));
    new->length = 0;
    new->max_length = 1;

    *array = new;
    return DYN_SUCCESS;
}

void dyn_free(dynarray *array) {
    if (array->array != NULL) {
        free(array->array);
    }

    free(array);
}

int dyn_add(dynarray *array, float a) {
    if (array->length + 1 > array->max_length) {
        // double the size of the array
        array->max_length *= 2;
        array->array = (float *) realloc(array->array, array->max_length * sizeof(float));
        
        if (array->array == NULL) return DYN_ERROR;
    }

    array->array[array->length] = a;
    array->length++;

    return DYN_SUCCESS;
}

int get_even(float **even, dynarray *array) {
    if (array->length % 2 != 0) return DYN_ERROR;

    float *ret = (float *) malloc(array->length / 2 * sizeof(float));

    int retindex = 0;
    for (int i = 0; i < array->length; i += 2) {
        ret[retindex] = array->array[i];
        retindex++;
    }

    *even = ret;
    return DYN_SUCCESS;
}

int get_odd(float **odd, dynarray *array) {
    if (array->length % 2 != 0) return DYN_ERROR;

    float *ret = (float *) malloc(array->length / 2 * sizeof(float));

    int retindex = 0;
    for (int i = 1; i < array->length; i += 2) {
        ret[retindex] = array->array[i];
        retindex++;
    }

    *odd = ret;
    return DYN_SUCCESS;
}
