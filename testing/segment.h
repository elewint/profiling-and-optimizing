/**************************************************************
 *
 *                         segment.h
 *
 *     Assignment: UM
 *     Authors:  Eli Intriligator (eintri01), Max Behrendt (mbehre01)
 *     Date:     Nov 23, 2021
 *
 *     Summary
 *     This class allows the user to manage segmented memory. It offers
 *     functions to allocate new segments of memory, free memory segments,
 *     and access the elements within segments. Users should know that in
 *     this implementation, each segment holds a Hanson UArray of integers.
 *     
 **************************************************************/
#ifndef SEGMENT_INCLUDED
#define SEGMENT_INCLUDED
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <seq.h>
#include <uarray.h>
#include <bitpack.h>

/* init_segment
 * Purpose: initializes our sequences of segments and available indicies,
            and places m0 into our segments sequence
 * Parameters: A UArray_T
 * Returns: Nothing
 *
 * Expected input: A UArray_T of instructions read in from a file
 * Success output: none 
 * Failure output: none
 */
void init_segment(UArray_T m0);

/* new_segment
 * Purpose: maps a new segment by adding it to our sequence of segments
 * Parameters: An integer
 * Returns: Nothing
 *
 * Expected input: An integer denoting the size of the segment to be
                    mapped
 * Success output: The index that the new segment was mapped to
 * Failure output: none
 */
uint32_t new_segment(int size);

/* free_segment
 * Purpose: frees the UArray at the given index
 * Parameters: A uint32_t
 * Returns: Nothing
 *
 * Expected input: A valid segment index
 * Success output: none
 * Failure output: exits the program if the supplied index is out of bounds
 */
void free_segment(uint32_t segment_index);

/* free_all_segments
 * Purpose: frees both the segments and available_indicies sequences and
            everything in them
 * Parameters: none
 * Returns: Nothing
 *
 * Expected input: none
 * Success output: none
 * Failure output: none
 */
void free_all_segments();

/* get_word
 * Purpose: gets the word at the given segment and index
 * Parameters: two uint32_ts
 * Returns: A uint32_t
 *
 * Expected input: A valid segment index and a valid word index
 * Success output: The uint32_t stored at the given indicies
 * Failure output: exits the program if either of the indicies is out of
                    bounds
 */
uint32_t get_word(uint32_t segment_index, uint32_t word_index);

/* set_word
 * Purpose: sets the given word at the given segment and index
 * Parameters: 3 uint32_ts
 * Returns: Nothing
 *
 * Expected input: A valid segment index, a valid word index, and a
                    32 bit word
 * Success output: none
 * Failure output: exits the program if either of the indicies is out of
                    bounds
 */
void set_word(uint32_t segment_index, uint32_t word_index,
                                             uint32_t word);

/* replace_segment_zero
 * Purpose: replaces m[0] with the segment at the supplied index, if the
            supplied index is 0, the function just returns
 * Parameters: a uint32_t
 * Returns: Nothing
 *
 * Expected input: A valid segment index
 * Success output: none
 * Failure output: exits the program if the index is out of bounds
 */
void replace_segment_zero(uint32_t new_segment_index);

/* seg_zero_length
 * Purpose: returns the length of the 0th memory segment
 * Parameters: none
 * Returns: a uint32_t representing the length of the segment
 *
 * Expected input: none
 * Success output: the length is returned
 * Failure output: none
 */
int seg_zero_length();

#endif