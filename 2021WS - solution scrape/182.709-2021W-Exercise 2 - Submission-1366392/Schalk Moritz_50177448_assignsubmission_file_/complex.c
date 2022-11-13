#include <math.h>

#include "complex.h"

complex_t complex_add(complex_t a, complex_t b) {
    if (a.real == NAN || a.complex == NAN) return COMPLEX_ERR;
    if (b.real == NAN || b.complex == NAN) return COMPLEX_ERR;
    
    return (complex_t) {.real = a.real + b.real, .complex = a.complex + b.complex};
}

complex_t complex_mult(complex_t a, complex_t b) {
    if (a.real == NAN || a.complex == NAN) return COMPLEX_ERR;
    if (b.real == NAN || b.complex == NAN) return COMPLEX_ERR;
    
    return (complex_t) {
        .real    = a.real * b.real    - a.complex * b.complex, 
        .complex = a.real * b.complex + a.complex * b.real
    };
}