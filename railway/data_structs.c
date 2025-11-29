// data_structs.c
// Implementation of core linked list logic

#include "backend.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Internal head pointer (private to data_structs.c) */
static Passenger *head = NULL;
int pnr_counter = 1001; // Global PNR counter

/* Helper function to get the current list head */
Passenger* get_head() {
    return head;
}

/* Helper function to set the current list head (used by file_io) */
void set_head(Passenger* new_head) {
    head = new_head;
}

/* Add a passenger node (from backend.c) */
void add_passenger_node(const char *name, int age, const char *category, int *out_pnr) {
    Passenger *p = (Passenger*)malloc(sizeof(Passenger));
    if (!p) return;
    p->pnr = pnr_counter++;
    
    strncpy(p->name, name, sizeof(p->name)-1);
    p->name[sizeof(p->name)-1] = '\0';
    p->age = age;
    
    // Set category
    if (category && category[0] != '\0') {
        strncpy(p->category, category, sizeof(p->category)-1);
        p->category[sizeof(p->category)-1] = '\0';
    } else {
        strncpy(p->category, "General", sizeof(p->category)-1);
        p->category[sizeof(p->category)-1] = '\0';
    }
    p->next = NULL;

    // Append to list
    if (head == NULL) head = p;
    else {
        Passenger *t = head;
        while (t->next) t = t->next;
        t->next = p;
    }
    if (out_pnr) *out_pnr = p->pnr;
}

/* Delete passenger by PNR (from backend.c) */
int delete_passenger_by_pnr(int pnr) {
    Passenger *curr = head;
    Passenger *prev = NULL;

    while (curr) {
        if (curr->pnr == pnr) {
            if (prev == NULL) head = curr->next;
            else prev->next = curr->next;
            
            free(curr);
            return 1; // Success
        }
        prev = curr;
        curr = curr->next;
    }
    return 0; // Not found
}

/* Search passenger by PNR (from backend.c) */
Passenger* search_passenger_by_pnr(int pnr) {
    Passenger *t = head;
    while (t) {
        if (t->pnr == pnr) return t;
        t = t->next;
    }
    return NULL;
}

/* Build a human-readable passenger list string (from backend.c) */
void build_passenger_list_string(char *buf, size_t bufsize) {
    Passenger *t = head;
    int offset = 0;
    
    // Print Header
    offset += snprintf(buf + offset, bufsize - offset, 
                       "PNR | Name | Age | Category\r\n");
    offset += snprintf(buf + offset, bufsize - offset, 
                       "-----------------------------------\r\n");

    while (t && offset < bufsize) {
        offset += snprintf(buf + offset, bufsize - offset, 
                           "%d | %s | %d | %s\r\n", 
                           t->pnr, t->name, t->age, t->category);
        t = t->next;
    }
    // Null-terminate the buffer if it was truncated
    if (offset >= bufsize) {
        buf[bufsize - 1] = '\0';
    }
}

/* Free all nodes in the list (from backend.c) */
void cleanup_passengers() {
    Passenger *curr = head;
    Passenger *next;
    while (curr) {
        next = curr->next;
        free(curr);
        curr = next;
    }
    head = NULL;
}