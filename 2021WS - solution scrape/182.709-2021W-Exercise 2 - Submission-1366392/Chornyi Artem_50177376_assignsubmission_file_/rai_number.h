/**
 * @author Artem Chornyi. 11922295
 * @brief Signatures of basic algebraic operations (+, -, *) of
 * 			rai_numbers (values with real and imaginary parts)
 *
 * @date 11.12.2021
 */

#ifndef RAI_NUMBER
#define RAI_NUMBER

#define Pi (3.141592654)


typedef struct {
	float real,imag;
} rai_number;

rai_number rai_mult(rai_number n1, rai_number n2);
rai_number rai_add(rai_number n1, rai_number n2);
rai_number rai_sub(rai_number n1, rai_number n2);
#endif
