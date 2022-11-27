/**
 * @author Artem Chornyi. 11922295
 * @brief Implements default algebraic operations (+, -, *)
 *			for rai_numbers (values with real and imaginary floats)
 *
 * @date 11.12.21
 */

#include "rai_number.h"

rai_number rai_mult(rai_number n1, rai_number n2){
	rai_number out = {
		.real = n1.real * n2.real - n1.imag * n2.imag,
		.imag = n1.real * n2.imag + n1.imag * n2.real 
	};

	return out;
}

rai_number rai_add(rai_number n1, rai_number n2){
	rai_number out = {.real=n1.real+n2.real, .imag=n1.imag+n2.imag};
	
	return out;
}

rai_number rai_sub(rai_number n1, rai_number n2){
	rai_number out = {.real=n1.real-n2.real, .imag=n1.imag-n2.imag};

	return out;
}



