/**
 * @file grep.h
 * @brief Header file containing definition of grep functions
 * @author Alexander Lampalzer <e12023145@student.tuwien.ac.at>
 * @date 31.10.2021
 *
 * @details
 *
 * This library provides a utility function to match a stream against a keyword.
 * The stream is read line-by-line, matched, and on success the matching line is written
 * to a second stream.
 */

#pragma once

#include <stdbool.h>
#include <stdio.h>

#ifndef _GREP_H
#define _GREP_H 1

/**
 * @brief Match a stream against a keyword.
 * @details
 *
 * A Stream is read line-by-line and matched against a keyword.
 * When a line contains the specified keyword, the whole line is written to the output (stream).
 *
 * @param case_sensitive Indicates case sensitivity. If false, the case of the keyword and stream is ignored.
 * @param keyword Keyword to match against
 * @param input Input Stream eg.: stdin, file stream, ...
 * @param output Output Stream eg.: stdout, file stream, ...
 *
 * @return
 *
 * Upon successful completion grep() returns a ERROR_SUCCESS status code.
 * Otherwise, a non-null status code is returned and errno is set to indicate the error.
 */
int grep(bool case_sensitive, const char* keyword, FILE* input, FILE* output);

#endif
