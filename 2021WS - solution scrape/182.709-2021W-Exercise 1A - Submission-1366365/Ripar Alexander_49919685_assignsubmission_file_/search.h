/**
 * @file search.h
 * @author Alexander Ripar <e12022494@student.tuwien.ac.at>
 * @date 9.11.2021
 *
 * @brief Header for logic responsible for searching files for the given keyword.
 *
 * @details Contains the declaration of the function run_search, which searches all
 * given input files for the given keyword.
**/

#ifndef SEARCH_INCLUDE_GUARD
#define SEARCH_INCLUDE_GUARD

#include <stdbool.h>

#include "init.h"

/**
 * @brief Searches all input files for the given keyword.
 *
 * @details Searches all files corresponding to ctx->input_filenames for ctx->keyword.
 * If ctx->input_filenames is empty (i.e. ctx->input_file_cnt == 0), stdin is used as input.
 * All lines that contain a match to ctx->keyword (respecting ctx->is_case_sensitive) 
 * are written to ctx->output_file.
 *
 * @param ctx Pointer to the proc_context struct holding the search configuration.
 *
 * @return true if all files have been successfully searched, false otherwise.
**/
bool run_search(proc_context* ctx);

#endif // SEARCH_INCLUDE_GUARD
