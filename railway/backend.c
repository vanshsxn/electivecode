// backend.c
// Implementation of railway booking backend

#include "backend.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Internal head pointer (private to backend.c) */
static Passenger *head = NULL;
int pnr_counter = 1001;

void add_passenger_node(const char *name, int age, const char *category, int *out_pnr) {
    Passenger *p = (Passenger*)malloc(sizeof(Passenger));
    if (!p) return;
    p->pnr = pnr_counter++;
    strncpy(p->name, name, sizeof(p->name)-1);
    p->name[sizeof(p->name)-1] = '\0';
    p->age = age;
    if (category && category[0] != '\0') {
        strncpy(p->category, category, sizeof(p->category)-1);
        p->category[sizeof(p->category)-1] = '\0';
    } else {
        strncpy(p->category, "General", sizeof(p->category)-1);
        p->category[sizeof(p->category)-1] = '\0';
    }
    p->next = NULL;

    if (head == NULL) head = p;
    else {
        Passenger *t = head;
        while (t->next) t = t->next;
        t->next = p;
    }
    if (out_pnr) *out_pnr = p->pnr;
}

int delete_passenger_by_pnr(int pnr) {
    Passenger *t = head, *prev = NULL;
    while (t && t->pnr != pnr) {
        prev = t;
        t = t->next;
    }
    if (!t) return 0; // not found
    if (!prev) head = t->next;
    else prev->next = t->next;
    free(t);
    return 1;
}

Passenger* search_passenger_by_pnr(int pnr) {
    Passenger *t = head;
    while (t) {
        if (t->pnr == pnr) return t;
        t = t->next;
        }
    return NULL;
}

void build_passenger_list_string(char *buf, size_t bufsize) {
    if (!buf || bufsize == 0) return;
    buf[0] = '\0';
    Passenger *t = head;
    char temp[256];
    if (!t) {
        strncat(buf, "No passengers available.\r\n", bufsize - strlen(buf) - 1);
        return;
    }
    while (t) {
        snprintf(temp, sizeof(temp), "PNR: %d  | Name: %s  | Age: %d  | Cat: %s\r\n",
                 t->pnr, t->name, t->age, t->category);
        strncat(buf, temp, bufsize - strlen(buf) - 1);
        t = t->next;
    }
}

void free_all_passengers(void) {
    Passenger *t = head;
    while (t) {
        Passenger *n = t->next;
        free(t);
        t = n;
    }
    head = NULL;
}

/* New: Save all passenger records to file. */
int save_passengers_to_file(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) return 0; // File opening failed

    Passenger *t = head;
    
    // Save the next available PNR counter value first
    fprintf(fp, "%d\n", pnr_counter); 

    // Iterate through the linked list and save each passenger
    while (t) {
        // Format: PNR|Name|Age|Category
        fprintf(fp, "%d|%s|%d|%s\n", t->pnr, t->name, t->age, t->category);
        t = t->next;
    }

    fclose(fp);
    return 1;
}

/* New: Load all passenger records from file. */
int load_passengers_from_file(const char *filename) {
    // Free existing data before loading new data
    free_all_passengers(); 
    head = NULL;
    int max_pnr = 1000;

    FILE *fp = fopen(filename, "r");
    if (fp == NULL) return 0; // File doesn't exist yet (normal on first run)

    // Load the saved PNR counter value first
    if (fscanf(fp, "%d\n", &pnr_counter) != 1) {
         // If we can't read the counter, close and reset it.
         pnr_counter = 1001; 
         fclose(fp);
         return 0;
    }

    // Temporary variables to hold data read from the file
    int pnr, age;
    char name[64] = {0}, category[32] = {0};
    char line[256];

    // Read remaining lines, one passenger per line
    while (fgets(line, sizeof(line), fp)) {
        // Parse the line using sscanf based on the saved format (PNR|Name|Age|Category)
        if (sscanf(line, "%d|%63[^|]|%d|%31[^\n]", 
                   &pnr, name, &age, category) == 4) 
        {
            // Dynamically allocate a new passenger node for the loaded data
            Passenger *p = (Passenger*)malloc(sizeof(Passenger));
            if (!p) { fclose(fp); return 0; }
            
            p->pnr = pnr;
            strncpy(p->name, name, sizeof(p->name)-1);
            p->name[sizeof(p->name)-1] = '\0';
            p->age = age;
            strncpy(p->category, category, sizeof(p->category)-1);
            p->category[sizeof(p->category)-1] = '\0';
            p->next = NULL;

            // Link the new node to the list (append)
            if (head == NULL) {
                head = p;
            } else {
                Passenger *t = head;
                while (t->next) t = t->next;
                t->next = p;
            }
            
            // Keep track of the highest PNR loaded to potentially reset pnr_counter
            if (pnr > max_pnr) max_pnr = pnr;
        }
    }
    
    // Safety check: ensure the global pnr_counter is at least 1 more than the highest PNR loaded
    if (pnr_counter <= max_pnr) {
        pnr_counter = max_pnr + 1;
    }

    fclose(fp);
    return 1;
}