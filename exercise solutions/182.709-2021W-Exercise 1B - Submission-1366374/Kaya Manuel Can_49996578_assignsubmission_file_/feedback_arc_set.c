/**
 * @file feedback_arc_set.c
 * @author Manuel Can Kaya 12020629
 * @brief Implements 'error_msg()'.
 * @details Implements the 'error_msg()' function to be used by both supervisor and generator program.
 * @version 0.1
 * @date 2021-11-13
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "feedback_arc_set.h"

void error_msg(const char *program_name, const char *brief_msg, const char *detail_msg)
{
    fprintf(stderr, "[%s]: %s: %s\n", program_name, brief_msg, detail_msg);
}
