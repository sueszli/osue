#ifndef LIST_H
#define LIST_H

struct listelem {
   char *val;
   struct listelem *next;
};

struct listelem *init_list(const char *const val);
void populate_list(struct listelem *const head);
void print_list(const struct listelem *const head);
void check_list(const struct listelem *const head);
void destroy_list(struct listelem *head);
#endif
