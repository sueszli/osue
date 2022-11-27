#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>

// ADAPT FOR SUBMISSION! COPY .h + .c, rem "../memMngmt/"
#include "memMngmt.h"
#include "datatypes.h"
#include "PointList.h"
#include "main.h"

/**
 * @file main.c for cpair
 * @author: Niki Herl, MatrNr. 01634238
 * @date: 4.12.2019
 * @brief
 * @details
 *  usage: cpair
 *  reads in n "points" (pairs of floating point numbers) via stdin (separated via newlines, terminated via EOF)
 *  cpair computes the closest pair of all points given
 */

/*
 Program outline
    - create pipes
    - fork twice, saving children's pids (+low/high info)
    - wire up pipes stdin+stdout in parent+child (mui complicado)
    - in children: execlp (to reset progress in main)
    - close write-to-children-pipe
      (closing write pipe triggers EOF)
    - wait()
    - get exit status + low_or_high_part?.
    - wait()
    - get exit status + low_or_high_part?.
    - read from children
    - close read-from-children pipe
    - determine closest pair among result1, result2, any combination of res1+res2 points
    - close write-to-parent pipe
*/

const int max_str_len = 420;
// these can be used for ALL read (getline) operations, right?
int n_read_chars;
bool cur_line_malloced = false;
char *cur_line_P = NULL;
size_t n_allocd_bytes = 0;
char *end_of_parsed_stuff_P = NULL;

PointList input;
float mean;
PointList low_part;
PointList high_part;

struct EdgecaseInfo edgecase_info;

int to_low_child[2];
int from_low_child[2];
int to_high_child[2];
int from_high_child[2];

FILE *t_l_FILE = NULL;
FILE *f_l_FILE = NULL;
FILE *t_h_FILE = NULL;
FILE *f_h_FILE = NULL;

void cleanup2(void) {
    if (cur_line_malloced) {
        free(cur_line_P);
        cur_line_malloced = false;
    }
    
    fclose(stdin);
    // not actually "stdout" after rewiring, but the write-to-parent pipe
    not_0_guard(fclose(stdout), "closing stdout failed");
        
    free_nodes_Point(&input);
    free_nodes_Point(&low_part);
    free_nodes_Point(&high_part);
}

// ought to detect (un)successful strtof parse results.
// (sometimes) resets errno!
bool parsed_ok(float parse_result) {
    if (isnan(parse_result) || parse_result == INFINITY) {
        return false;
    }
    if (errno == ERANGE) {
        errno = 0;
        return false;
    }
    return true;
}

void parsed_ok_guard(float parse_res, char *error_msg) {
    if (!parsed_ok(parse_res)) {
        char parse_result_Str[max_str_len];
        snprintf(parse_result_Str, max_str_len, "%f", parse_res);
        abortW2Msgs(error_msg, parse_result_Str, false);
    }
}

void* not_NULL = (void *) 1;

Point parse_point(char* parse_string) {
    Point result;
    char *endptr = not_NULL;
    result.x = strtof(parse_string, &endptr);
    parsed_ok_guard(result.x, "parsing the x part of a Point failed. result was:");
    result.y = strtof(endptr, NULL);
    parsed_ok_guard(result.y, "parsing the y part of a Point failed. result was:");

    return result;
}


void print_point2(Point p, int index) {
    print_point(p);
}

float dist_1D(float x1, float x2) {
    if (x1 > x2) {
        return (x1 - x2);
    } else
        return (x2 - x1);
}

float dist_2D(Point p1, Point p2) {
    return sqrtf(powf(dist_1D(p1.x, p2.x), 2) + powf(dist_1D(p1.y, p2.y), 2));
}

// precondition: size is the actual size of the array (=^= size-1 is last valid index)
// post: -1 if array had size 0 or all values were __FLT_MAX__
int minimum_index_f(float array[], int size){
    int cur_min_ind = -1;
    float cur_min_val = __FLT_MAX__;
    for (int i = 0; i < size; i++) {
        if(array[i] < cur_min_val) {
            cur_min_ind = i;
            cur_min_val = array[i];
        }
    }
    return cur_min_ind;
}

void add_to_low_or_high(Point p, int index){
    if (p.x <= mean) {
        add_Point(&low_part, p);
    } else {
        add_Point(&high_part, p);
    }
}

void count_low_or_high(Point p, int index) {
    if (p.x <= mean) {
        edgecase_info.n_low++;
        edgecase_info.ind_last_low = index;
    } else {
        edgecase_info.n_high++;
        edgecase_info.ind_last_high = index;
    }
}

void print_low(Point p, int index) {
    print_point_to(p, t_l_FILE);
}
void print_high(Point p, int index) {
    print_point_to(p, t_h_FILE);
}

typedef struct ComboInfo_Str {
    Point cur_low_point;
    float cur_best_dist; // init to -1
    Point cur_best_p1;
    Point cur_best_p2;
} ComboInfo;

ComboInfo comboinfo;

void combo_check_inner(Point p, int index){
    float cur_dist = dist_2D(comboinfo.cur_low_point, p);
    if (comboinfo.cur_best_dist == -1 || cur_dist < comboinfo.cur_best_dist){
        comboinfo.cur_best_p1 = comboinfo.cur_low_point;
        comboinfo.cur_best_p2 = p;
        comboinfo.cur_best_dist = cur_dist;
    }
}

void combo_check_outer(Point p, int index){
    comboinfo.cur_low_point = p;
    for_each_Point(&high_part, &combo_check_inner);
}


int main(int argc, char *argv[], char **envp) {

    // SECTION: READ IN UP TO 3 LINES (or already EOF)
    input = empty_Point();
    for (int i = 0; i < 3; i++) {
        n_read_chars = getline(&cur_line_P, &n_allocd_bytes, stdin);
        cur_line_malloced = true;

        if (n_read_chars < 0 && errno == 0) {
            // no more points to read
            break;
        }
        if (n_read_chars < 0 && errno != 0) {
            abortWMsg("reading one of the inital 2 lines went wrong", true);
        }

        Point cur_point = parse_point(cur_line_P);
        add_Point(&input, cur_point);
    }

    // SECTION: Base Cases
    switch (input.length) {
        case 0:
            assert(feof(stdin));
            // abortWMsg("no points found on stdin", false);
            cleanup2();
            exit(EXIT_SUCCESS);
            break;
        case 1:
            // just 1 point -> no pair possible -> silence
            assert(feof(stdin));
            cleanup2();
            exit(EXIT_SUCCESS);
            break;
        case 2:
            // that pair -> stdout
            assert(feof(stdin));
            for_each_Point(&input, &print_point2);
            cleanup2();
            exit(EXIT_SUCCESS);
            break;
        case 3:
            // continue outside the switch
            break;
        default:
            assert(false);
    }

    // SECTION: READ IN REST OF INPUT
    // and calc mean on the fly
    // on-line mean algo: see <https://stackoverflow.com/questions/28820904/> 

    // init mean with first 3 .x values
    Point p1 = Point_at_index(&input, 0);
    Point p2 = Point_at_index(&input, 1);
    Point p3 = Point_at_index(&input, 2);
    mean = (p1.x + p2.x + p3.x) / input.length;

    // for each line on stdin
    while ((n_read_chars = getline(&cur_line_P, &n_allocd_bytes, stdin)) > 0) {
        Point cur_point = parse_point(cur_line_P);
        add_Point(&input, cur_point);
        mean = mean + (cur_point.x - mean) / input.length;
    }
    if (n_read_chars < 0 && errno != 0) {
        abortWMsg("during the big read-in, something went wrong:", true);
    }
    assert(feof(stdin));

    // catch EDGE CASE w/ all x equal <-> splitting would degenerate, no rec advance.

    // for each point in input
    low_part = empty_Point();
    high_part = empty_Point();
    for_each_Point(&input, &add_to_low_or_high);
    free_nodes_Point(&input);

    assert (low_part.length != 1 || high_part.length != 1);
    assert (low_part.length != 0);

    if (high_part.length == 0) {
        assert(low_part.length >= 3);
        Point move_for_rec_advancement = remove_Point_at(&low_part, 0);
        add_Point(&high_part, move_for_rec_advancement);
    }

    /* SECTION
        create pipes to/from low/high children
        make those children by forking twice
        wire up (detangle) pipes in parent to stdin / stdout in child
        execlp in children (to reset progress in main)
    */
    // pipe[0] becomes fd of the read  end of the pipe.
    // pipe[1] becomes fd of the write end of the pipe.
    // todo: do each part only if .length >= ...2? 3?
    not_0_guard(pipe(to_low_child), "creating pipe to_low_child failed");
    not_0_guard(pipe(from_low_child), "creating pipe from_low_child failed");

    // fork, the first
    pid_t temp_pid = fork();
    switch (temp_pid) {
        case -1:
            abortWMsg("Cannot fork (low)!", true);
        case 0: // child
            // wire up ("detangle") pipes
            close(to_low_child[1]);
            close(from_low_child[0]);

            // "to self", i.e. input pipe
            minus_1_guard(dup2(to_low_child[0], STDIN_FILENO),
                            "dup'ing to_low_child[0] failed");
            minus_1_guard(close(to_low_child[0]),
                            "closing to_low_child[0] failed");
            null_guard(stdin = fdopen(STDIN_FILENO, "r"), "fdopen failed");

            // "from self", i.e. output pipe
            minus_1_guard(dup2(from_low_child[1], STDOUT_FILENO),
                            "dup'ing from_low_child[1] failed");
            minus_1_guard(close(from_low_child[1]),
                            "closing from_low_child[1] failed");
            null_guard(stdout = fdopen(STDOUT_FILENO, "w"), "fdopen failed");

            // exec self
            execlp("./cpair", "cpair", (char *) NULL);
            abortWMsg("execlp for low failed", true);
            break;
        default: // parent
            // wire up ("detangle") pipes
            close(to_low_child[0]);
            close(from_low_child[1]);
            t_l_FILE = null_guard(fdopen(to_low_child[1], "w"),
                                     "fdopen failed");
            f_l_FILE = null_guard(fdopen(from_low_child[0], "r"),
                                     "fdopen failed");
            break;
    }

    not_0_guard(pipe(to_high_child), "creating pipe to_high_child failed");
    not_0_guard(pipe(from_high_child), "creating pipe from_high_child failed");

    // fork, the second
    temp_pid = fork();
    switch (temp_pid) {
        case -1:
            abortWMsg("Cannot fork (high)!", true);
        case 0: // child
            // wire up ("detangle") pipes
            close(to_high_child[1]);
            close(from_high_child[0]);

            // "to self", i.e. input pipe
            minus_1_guard(dup2(to_high_child[0], STDIN_FILENO),
                            "dup'ing to_high_child[0] failed");
            minus_1_guard(close(to_high_child[0]),
                            "closing to_high_child[0] failed");
            null_guard(stdin = fdopen(STDIN_FILENO, "r"), "fdopen failed");

            // "from self", i.e. output pipe
            minus_1_guard(dup2(from_high_child[1], STDOUT_FILENO),
                            "dup'ing from_high_child[1] failed");
            minus_1_guard(close(from_high_child[1]),
                            "closing from_high_child[1] failed");
            null_guard(stdout = fdopen(STDOUT_FILENO, "w"), "fdopen failed");

            // exec self
            execlp("./cpair", "cpair", (char *) NULL);
            abortWMsg("execlp for high failed", true);
            break;
        default: // parent
            // wire up ("detangle") pipes
            close(to_high_child[0]);
            close(from_high_child[1]);
            t_h_FILE = null_guard(fdopen(to_high_child[1], "w"),
                                     "fdopen failed");
            f_h_FILE = null_guard(fdopen(from_high_child[0], "r"),
                                     "fdopen failed");
            break;
    }

    // assert: we're in a parent with 2 direct children. to have _some_ actual assertion:
    assert (t_l_FILE != NULL && f_l_FILE != NULL && t_h_FILE != NULL && f_h_FILE != NULL);


    // SECTION: DISTRIBUTE THE INPUT
    for_each_Point(&low_part, &print_low);
    for_each_Point(&high_part, &print_high);


    // close both write-to-child pipes
    // implicitly sends EOF (fclose closes fd, closing pipe fd sends EOF)
    not_0_guard(fclose(t_l_FILE), NULL);
    not_0_guard(fclose(t_h_FILE), NULL);

    /*
    SECTION: WAIT FOR BOTH CHILDREN TO FINISH
             get exit status.
             if exit status == failure: terminate with failure
    */
    int return_status;
    minus_1_guard(wait(&return_status),
                    "first wait failed");
    if (return_status != EXIT_SUCCESS) {
        abortWMsg("first-finishing child didn't succeed", false);
    }

    minus_1_guard(wait(&return_status),
                    "second wait failed");
    if (return_status != EXIT_SUCCESS) {
        abortWMsg("second-finishing child didn't succeed", false);
    }
    // assert: both children were successful



    // SECTION: GET CHILD RESULTS
    Point low_res[2], high_res[2];
    if (low_part.length >= 2) {
        negative_guard(getline(&cur_line_P, &n_allocd_bytes, f_l_FILE), "reading p1 from (low) result pipe failed");
        low_res[0] = parse_point(cur_line_P);
        negative_guard(getline(&cur_line_P, &n_allocd_bytes, f_l_FILE), "reading p2 from (low) result pipe failed");
        low_res[1] = parse_point(cur_line_P);
    }
    if (high_part.length >= 2) {
        negative_guard(getline(&cur_line_P, &n_allocd_bytes, f_h_FILE), "reading p1 from (high) result pipe failed");
        high_res[0] = parse_point(cur_line_P);
        negative_guard(getline(&cur_line_P, &n_allocd_bytes, f_h_FILE), "reading p2 from (high) result pipe failed");
        high_res[1] = parse_point(cur_line_P);
    }
    // read-from-children pipe no longer needed
    not_0_guard(fclose(f_l_FILE), NULL);
    not_0_guard(fclose(f_h_FILE), NULL);



    // SECTION: MERGE RESULTS
    // i.e. pick the best of a) 2 sub-result pairs or b) combination of low_part X high_part
    // (w/ several edge cases)

    Point final_pair[2];
    
    comboinfo.cur_best_dist = -1;
    for_each_Point(&low_part, &combo_check_outer);
    Point best_cross_combo[2] = {comboinfo.cur_best_p1, comboinfo.cur_best_p2};

    // "STANDARD" CASE, i.e. both children returned a pair
    if (low_part.length >= 2 && high_part.length >= 2) {
        // compare low_res,
        // high_res,
        // {low_res[0]<->high_res[0], low_res[0]<->high_res[1], low_res[1]<->high_res[0], low_res[1]<->high_res[1]}
        float dist_low  = dist_2D(low_res[0] , low_res[1] );
        float dist_high = dist_2D(high_res[0], high_res[1]);

        float dists[] = {dist_low, dist_high, comboinfo.cur_best_dist};
        int min_ind = minimum_index_f(dists, 3);

        switch (min_ind) {
        case 0:
            memcpy(final_pair, low_res, sizeof(Point) * 2);
            break;
        case 1:
            memcpy(final_pair, high_res, sizeof(Point) * 2);
            break;
        case 2:
            memcpy(final_pair, best_cross_combo, sizeof(Point) * 2);
            break;
        default:
            assert(false);
        }
    } else if (low_part.length == 1 && high_part.length == 1) {
        assert(false);
    } else if (low_part.length == 1 || high_part.length == 1) {
        // CASE w/ 1 child having gotten just 1 point (other has >=2)

        // determine the "good" result
        Point good_res[2];
        if (low_part.length == 1) {
            memcpy(good_res, high_res, sizeof(Point) * 2);
        } else {
            memcpy(good_res, low_res, sizeof(Point) * 2);
        }

        float dist_good_res = dist_2D(good_res[0], good_res[1]);

        if(dist_good_res < comboinfo.cur_best_dist) {
            memcpy(final_pair, good_res, sizeof(Point) * 2);
        } else {
            memcpy(final_pair, best_cross_combo, sizeof(Point) * 2);
        }
    } else {
        assert(false);
    }

    // PRINT RESULTING PAIR
    print_point(final_pair[0]);
    print_point(final_pair[1]);

    cleanup2();
    exit(EXIT_SUCCESS);
}
