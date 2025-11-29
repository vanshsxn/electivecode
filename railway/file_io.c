// file_io.c
// Implementation of file saving and loading logic

#include "backend.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* These functions are assumed to be implemented in data_structs.c */
extern Passenger* get_head();
extern void set_head(Passenger* new_head);
extern void cleanup_passengers();

/* Save all passenger records to file (from backend.c) */
int save_passengers_to_file(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) return 0; // Failure

    // 1. Write the current PNR counter first
    fprintf(fp, "%d\r\n", pnr_counter);

    // 2. Write each passenger record
    Passenger *t = get_head();
    while (t) {
        fprintf(fp, "%d|%s|%d|%s\r\n", t->pnr, t->name, t->age, t->category);
        t = t->next;
    }
    
    fclose(fp);
    return 1; // Success
}

/* Load all passenger records from file (from backend.c) */
int load_passengers_from_file(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) return 0; // File may not exist yet, treat as success/empty

    cleanup_passengers(); // Clear any existing list before loading

    int pnr;
    char name[64];
    int age;
    char category[32];
    char line[256];
    
    // 1. Read the PNR counter from the first line
    if (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "%d", &pnr_counter) != 1) {
            pnr_counter = 1001; // Default if first line is corrupt
        }
    } else {
        fclose(fp);
        return 1; // Empty file is fine
    }

    Passenger *current_head = NULL;
    Passenger *last = NULL;

    // 2. Read remaining lines, one passenger per line
    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "%d|%63[^|]|%d|%31[^ \r\n]", 
                   &pnr, name, &age, category) == 4) 
        {
            Passenger *p = (Passenger*)malloc(sizeof(Passenger));
            if (!p) { fclose(fp); return 0; } // Memory allocation failure
            
            p->pnr = pnr;
            strncpy(p->name, name, sizeof(p->name)-1);
            p->name[sizeof(p->name)-1] = '\0';
            p->age = age;
            strncpy(p->category, category, sizeof(p->category)-1);
            p->category[sizeof(p->category)-1] = '\0';
            p->next = NULL;

            // Link the new node
            if (current_head == NULL) {
                current_head = p;
                last = p;
            } else {
                last->next = p;
                last = p;
            }
        }
    }
    
    set_head(current_head); // Update the global head pointer
    fclose(fp);
    return 1; // Success
}