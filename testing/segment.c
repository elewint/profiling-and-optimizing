/**************************************************************
 *
 *                         segment.c
 *
 *     Assignment: UM
 *     Authors:  Eli Intriligator (eintri01), Max Behrendt (mbehre01)
 *     Date:     Nov 23, 2021
 *
 *     Summary
 *     Implementation of the segment class.
 *     
 **************************************************************/
#include "segment.h"


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
void init_segment(UArray_T m0)
{
    segments = Seq_new(0);
    available_indices = Seq_new(0);
    Seq_addhi(segments, m0);
}

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
uint32_t new_segment(int size)
{
    UArray_T segment = UArray_new(size, sizeof(uint32_t));

    if (Seq_length(available_indices) == 0) {
        Seq_addhi(segments, segment);
        return (Seq_length(segments) - 1);
    } else {
        uint32_t *available_index_p = (uint32_t *)Seq_remlo(available_indices);
        uint32_t available_index = *available_index_p;
        free(available_index_p);

        Seq_put(segments, available_index, segment);

        return available_index;
    }
}

/* free_segment
 * Purpose: frees the UArray at the given index
 * Parameters: A uint32_t
 * Returns: Nothing
 *
 * Expected input: A valid segment index
 * Success output: none
 * Failure output: exits the program if the supplied index is out of bounds
 */
void free_segment(uint32_t segment_index)
{
    if ((int)segment_index >= Seq_length(segments)) {
        exit(1);
    }

    UArray_T seg = (UArray_T)Seq_put(segments, segment_index, NULL);
    UArray_free(&seg);

    uint32_t *available_index = malloc(sizeof(uint32_t));
    assert (available_index != NULL);

    *available_index = segment_index;
    Seq_addhi(available_indices, available_index);
}

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
void free_all_segments()
{
    int length = Seq_length(segments);

    for (int i = 0; i < length; i++) {
        UArray_T seg = (UArray_T)Seq_remlo(segments); 

        if (seg != NULL) {
            UArray_free(&seg);
        }
    }

    Seq_free(&segments);

    length = Seq_length(available_indices);

    for (int i = 0; i < length; i++) {
        uint32_t *available_index = (uint32_t *)Seq_remlo(available_indices); 
        free(available_index);
    }

    Seq_free(&available_indices);
}

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
uint32_t get_word(uint32_t segment_index, uint32_t word_index)
{
    if ((int)segment_index >= Seq_length(segments)) {
        exit(1);
    }

    UArray_T seg = (UArray_T)Seq_get(segments, segment_index);

    if ((int)word_index >= UArray_length(seg)) {
        exit(1);
    }

    uint32_t word = *(uint32_t *)UArray_at(seg, word_index);
   
    return word;
}

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
void set_word(uint32_t segment_index, uint32_t word_index, uint32_t word)
{
    if ((int)segment_index >= Seq_length(segments)) {
        exit(1);
    }

    UArray_T seg = (UArray_T)Seq_get(segments, segment_index);
    
    if ((int)word_index >= UArray_length(seg)) {
        exit(1);
    }

    *(uint32_t *)UArray_at(seg, word_index) = word;
}

/* replace_segment_zero
 * Purpose: replaces m[0] with the segment at the supplied index, if the
 *          supplied index is 0, the function just returns
 * Parameters: a uint32_t
 * Returns: Nothing
 *
 * Expected input: A valid segment index
 * Success output: none
 * Failure output: exits the program if the index is out of bounds
 */
void replace_segment_zero(uint32_t new_segment_index)
{
    if ((int)new_segment_index >= Seq_length(segments)) {
        exit(1);
    }

    if (new_segment_index == 0) {
        return;
    }

    UArray_T seg = Seq_get(segments, new_segment_index);
    UArray_T new_seg_zero = UArray_copy(seg, UArray_length(seg));

    UArray_T orig_seg_zero = (UArray_T)Seq_put(segments, 0, new_seg_zero);
    UArray_free(&orig_seg_zero);
}

/* seg_zero_length
 * Purpose: returns the length of the 0th memory segment
 * Parameters: none
 * Returns: a uint32_t representing the length of the segment
 *
 * Expected input: none
 * Success output: the length is returned
 * Failure output: none
 */
int seg_zero_length()
{
    UArray_T seg_zero = (UArray_T)Seq_get(segments, 0);
    return UArray_length(seg_zero);
}
