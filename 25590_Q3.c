#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "rndwalknetwork_25590.h"


int main() {

    // seed the random number generator with the current time (typically in seconds)
    srandom(time(NULL));

    // vary paramaters and number of particles accordingly
    int A = 5, B = 4;
    int num_particles = 1;

    // declare an array of structures to store the position of each particle 
    pos_info particles[num_particles];
    tracker_info tracker;

    // allocate memory for the structure for A and B, and initialsie
    allocate_struct(&tracker, A, B, num_particles);
    reset_position(particles, &tracker);

    // print first M positions of the random walk
    int M = 20;
    print_M_positions(particles, &tracker, M);

    // vary total iterations accordingly
    int total_iterations = 1e4;
    float average_iterations;

    // average iterations to visit all points in network for A, B
    random_walk(particles, &tracker, &average_iterations, total_iterations);

    // average iterations for A = 2 to Amax, B = B
    int Amax = 100;
    FILE *fp1 = open_file("filename1.csv", "A,Avg_iterations");
    for (A = 2, B = B; A <= Amax; A+=2) {
        reallocate_struct(&tracker, A, B);
        random_walk(particles, &tracker, &average_iterations, total_iterations);
        fprintf(fp1, "%d,%f\n", A, average_iterations);
    }
    fclose(fp1);

    // average iterations for A = 5, B = 2 to Bmax
    int Bmax = 100;
    FILE *fp2 = open_file("filename2.csv", "B,Avg_iterations");
    for (A = 5, B = 2; B <= Bmax; B+=2) {
        reallocate_struct(&tracker, A, B);
        random_walk(particles, &tracker, &average_iterations, total_iterations);
        fprintf(fp2, "%d,%f\n", B, average_iterations);
    }
    fclose(fp2);

    // track the average count of particles in each component for A, B below; and store results in a file
    reallocate_struct(&tracker, A = 101, B = 4); // A and B specified for clarity 
    pos_distribution(particles, &tracker, total_iterations);

    free_struct(&tracker);
    return 0;

}


// functions to handle the random walk //

// random number generator for the network, please see the comments at the end of the file for a more detailed overview 
// acception - rejection method --> only ineficient at small B 
void random_int(int *comp_pos, int *subcomp_pos, int b) {

    int j = *subcomp_pos;
    // if particle is at nodes 1, B then can move to a different component or within the same component
    if (j == 1 || j == b) {
        while (j == *subcomp_pos) { // ensures self exclusion 
            *subcomp_pos = random() % (b+1); // random number between 0 and b
        }
        // if random number is 0 then move to another component and update sub component position (else subcomp_pos remains unchanged)
        if (*subcomp_pos == 0) { 
            if (j == 1) {
                (*comp_pos)--;
                *subcomp_pos = b;
            } else {
                (*comp_pos)++;
                *subcomp_pos = 1;
            }
        }
    } else { // if particle is not at nodes 1, B then it can only move to another node within the same component
        while (j == *subcomp_pos) {
            *subcomp_pos = (random() % b) + 1; // random number between 1 and b
        }
    }
} 


// generate a new position for each particle
void new_position(pos_info n[], tracker_info *t) {

    // loop through each particle
    for (int i = 0; i < t->num_particles; i++) {
        
        int component_position = n[i].pos[0], subcomponent_position = n[i].pos[1]; // variable assignement helps improve readability
        random_int(&component_position, &subcomponent_position, t->num_subcomponents); // generate random direction

        // implement periodic boundary conditions
        if (component_position < 1) {
            n[i].pos[0] = t->num_components;
        } else if (component_position > t->num_components) {
            n[i].pos[0] = 1;
        } else {
            n[i].pos[0] = component_position;
        }

        n[i].pos[1] = subcomponent_position;
    }

}


// reset the position of each particle, and the position tracker
void reset_position(pos_info n[], tracker_info *t) {

    int start_position[2] = {1, 1}; // A = 1, B = 1 start position

    // loop through each particle and set t same start position
    for (int i = 0; i < t->num_particles; i++) {

        for (int j = 0; j < 2; j++) {
            n[i].pos[j] = start_position[j];
        }
    }

    for (int i = 0; i < t->num_components; i++) {
        for (int j = 0; j < t->num_subcomponents; j++) {
            t->pos_tracker[i][j] = 0;
        }
    }


}


// count the number of iterations to visit all points in the network
int iterations_to_vist_all(pos_info n[], tracker_info *t) {

    int iterations = 0, count = 0;

    reset_position(n, t);

    // only increment count if the position has not been visited before
    while (count < (t->num_components * t->num_subcomponents)) {

        new_position(n, t);

        for (int i = 0; i < t->num_particles; i++) { 

            // if the position has not been visited before by any other particles then increment the count
            if (t->pos_tracker[n[i].pos[0]-1][n[i].pos[1]-1] == 0) {
                t->pos_tracker[n[i].pos[0]-1][n[i].pos[1]-1] = 1;
                count++;
            } else { 
                // if visited before then increment value so can track the number of times the position has been visited
                t->pos_tracker[n[i].pos[0]-1][n[i].pos[1]-1] += 1;
            }
        }

        iterations++;
    }

    return iterations;
}


// perform the random walk and calculate the average number of iterations to visit all points in the network
void random_walk(pos_info n[], tracker_info *t, float *average_iterations, int total_iterations) {

    long total = 0;
    *average_iterations = 0;

    for (int i = 0; i < total_iterations; i++) {
        total +=  iterations_to_vist_all(n, t);
    }

    *average_iterations = total / (float) total_iterations;
    printf("Average iterations = %f, for (A = %i, B = %i)\n", *average_iterations, t->num_components, t->num_subcomponents);

}

// functions to handle memory allocation and deallocation //

// allocate memory for the structure
void allocate_struct(tracker_info *t, int a, int b, int np) {

    t->num_components = a; // number of rows
    t->num_subcomponents = b; // number of columns
    t->num_particles = np;

    // allocate memory for an array of pointers for each component / row
    t->pos_tracker = malloc(t->num_components * sizeof(int *)); 
    if (t->pos_tracker == NULL) {
        printf("Memory allocation failed\n");
        exit(1);
    }

    // allocate memory for each column / sub component in each row
    for (int i = 0; i < t->num_components; i++) {
        t->pos_tracker[i] = malloc(t->num_subcomponents * sizeof(int));
        if (t->pos_tracker[i] == NULL) {
            printf("Memory allocation failed\n");
            exit(2);
        }
    }

}


// reallocate memory for the structure for varying A, B
void reallocate_struct(tracker_info *t, int a, int b) {

    int realloc_comp;

    // if the number of components is unchanged then so are the number of rows in the pointer array 
    if (t->num_components == a) {
        realloc_comp = 0;
    } else {
        realloc_comp = 1;
    }

    t->num_components = a;
    t->num_subcomponents = b;

    // reallocate memory for an array of pointers for each component / row
    if (realloc_comp) {
        t->pos_tracker = realloc(t->pos_tracker, t->num_components * sizeof(int *));
        if (t->pos_tracker == NULL) {
            printf("Memory reallocation failed\n");
            exit(3);
        }
    }

    // reallocate memory for each column / sub component in each row
    for (int i = 0; i < t->num_components; i++) {
        t->pos_tracker[i] = realloc(t->pos_tracker[i], t->num_subcomponents * sizeof(int));
        if (t->pos_tracker[i] == NULL) {
            printf("Memory reallocation failed\n");
            exit(4);
        } 
    }

}


// free memory allocated for the structure
void free_struct(tracker_info *t) {

    // free memory for each column / sub component in each row
    for (int i = 0; i < t->num_components; i++) {
        free(t->pos_tracker[i]);
    }

    // free memory for an array of pointers for each component / row
    free(t->pos_tracker);
}

// functions to print positions //

// generate a new position for a single particle at a time (somewhat degenerate but allows for neater printing)
void new_position_single(pos_info *n, tracker_info *t) {

    int component_position = n->pos[0], subcomponent_position = n->pos[1]; // variable assignement helps improve readability
    random_int(&component_position, &subcomponent_position, t->num_subcomponents);

    if (component_position < 1) {
        n->pos[0] = t->num_components;
    } else if (component_position > t->num_components) {
        n->pos[0] = 1;
    } else {
        n->pos[0] = component_position;
    }

    n->pos[1] = subcomponent_position;

}


// prints the first M positions of the random walk for each particle at a time
void print_M_positions(pos_info n[], tracker_info *t, int M) {

    for (int j = 0; j < t->num_particles; j++) {

        printf("Particle %i: [%d, %d]", j+1, n[j].pos[0], n[j].pos[1]); 
        for (int i = 0; i < M; i++) {
            new_position_single(&n[j], t);
            printf("[%d, %d]", n[j].pos[0], n[j].pos[1]);
        } 
        printf("\n");
    }
}

// function to measure the average count of particles in each component //

// track the average count of particles in each component 
void pos_distribution(pos_info n[], tracker_info *t, int total_iterations) {

    // array to store the cumulative count of particles in each component
    unsigned int culm_array[t->num_components];
    for (int i = 0; i < t->num_components; i++) {
        culm_array[i] = 0;
    }

    // after each call to iterations_to_vist_all, add the count of particles in each component to the cumulative array
    for (int i = 0; i < total_iterations; i++) {
        iterations_to_vist_all(n, t);
    
        for (int i = 0; i < t->num_components; i++) {
            for (int j = 0; j < t->num_subcomponents; j++) {
                culm_array[i] += t->pos_tracker[i][j];
            }
        }
    }


    // compute a normalised array of the cumulative count of particles in each component
    long total = 0;
    for (int i = 0; i < t->num_components; i++) {
        total += culm_array[i];
    }
    float norm_array[t->num_components];
    for (int i = 0; i < t->num_components; i++) {
        norm_array[i] = (culm_array[i] / (float) total);
    }

    // write the normalised array to a file
    FILE *fp3 = open_file("filename3.csv", "Component,Total");
    for (int i = 0; i < t->num_components; i++) {
        fprintf(fp3, "%d,%f\n", i+1, norm_array[i]);
    }
    fclose(fp3);
}

// function to handle file writing //

// returns a file pointer to the file with the specified filename and header
FILE *open_file(const char *filename, const char *header) {

    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        printf("Error opening file\n");
        exit(5);
    }
    fprintf(fp, "%s\n", header);
    return fp;
}


// Graphical overview of indexing within the network //

//   component i=1; subcomponent j=
//        2 *---* 3
//          | \ |
//    i=A --*---*-- i=2
//         1    B
// For B maximally connected sub components then equal probability of moving to any other node within 
//     |--> generate random number between 1 and B and move to that node
// If j = 1 then can move to another componet (i-1) or within sub component 
//     |--> generate random number x between 0 and B (if x=0 then move to i-1, j=B; else move to j=x)
// Similar for j = B but can move to i+1
// Implement PBCs if i < 1, or i > A 