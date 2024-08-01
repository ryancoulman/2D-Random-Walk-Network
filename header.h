#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>

// structure to store the position of the particle 
typedef struct {
    int pos[2];
} pos_info;

// structure to track the position and number of particles within the network 
typedef struct {
    int **pos_tracker;
    int num_particles;
    int num_components;
    int num_subcomponents;
} tracker_info;

// functions to handle the random walk
void random_int(int *comp_pos, int *subcomp_pos, int b);
void new_position(pos_info n[], tracker_info *t);
void reset_position(pos_info n[], tracker_info *t);
int iterations_to_vist_all(pos_info n[], tracker_info *t);
void random_walk(pos_info n[], tracker_info *t, float *average_iterations, int total_iterations);
// functions to handle memory allocation and deallocation
void allocate_struct(tracker_info *t, int a, int b, int np);
void reallocate_struct(tracker_info *t, int a, int b);
void free_struct(tracker_info *t);
// function to print positions 
void new_position_single(pos_info *n, tracker_info *t);
void print_M_positions(pos_info n[], tracker_info *t, int M);
// function to measure the average count of particles in each component 
void pos_distribution(pos_info n[], tracker_info *t, int total_iterations);
// function to handle file writing
FILE *open_file(const char *filename, const char *header);

#endif /* HEADER_H */
