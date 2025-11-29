#ifndef BACKEND_H
#define BACKEND_H

#include <stddef.h>

typedef struct Passenger {
    int pnr;
    char name[64];
    int age;
    char category[32];
    struct Passenger *next;
} Passenger;
extern int pnr_counter;

void add_passenger_node(const char *name, int age, const char *category, int *out_pnr);

int delete_passenger_by_pnr(int pnr);
Passenger* search_passenger_by_pnr(int pnr);
void build_passenger_list_string(char *buf, size_t bufsize);
int save_passengers_to_file(const char *filename);
int load_passengers_from_file(const char *filename);
void cleanup_passengers();
#endif