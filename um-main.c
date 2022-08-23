/**************************************************************
 *
 *                         um-main.c
 *
 *     Assignment: UM
 *     Authors:  Eli Intriligator (eintri01), Max Behrendt (mbehre01)
 *     Date:     Nov 23, 2021
 *
 *     Summary
 *     This file takes in a UM file from the command line and executes
 *     its instructions, calling functions from the instructions.h
 *     and segment.h modules where necessary.
 *     
 *     Note
 *     A UM file must be supplied.
 *     
 **************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <assert.h>
#include <seq.h>
#include <uarray.h>

typedef enum Um_opcode {
        CMOV = 0, SLOAD, SSTORE, ADD, MUL, DIV,
        NAND, HALT, ACTIVATE, INACTIVATE, OUT, IN, LOADP, LV
} Um_opcode;
typedef uint32_t Um_instruction;
typedef enum Um_register { r0 = 0, r1, r2, r3, r4, r5, r6, r7 } Um_register;

uint32_t registers[8] = {0, 0, 0, 0, 0, 0, 0, 0};
UArray_T segments;
UArray_T segment_zero;
Seq_T available_indices;
int next_free_slot;

static inline UArray_T read_words(FILE *fp, int num_words);
static inline void execute_program();

static inline void cmov(Um_register a, Um_register b, Um_register c);
static inline void seg_load(Um_register a, Um_register b, Um_register c);
static inline void seg_store(Um_register a, Um_register b, Um_register c);
static inline void add(Um_register a, Um_register b, Um_register c);
static inline void multiply(Um_register a, Um_register b, Um_register c);
static inline void divide(Um_register a, Um_register b, Um_register c);
static inline void nand(Um_register a, Um_register b, Um_register c);
static inline void map_seg(Um_register b, Um_register c);
static inline void unmap_seg(Um_register c);      
static inline void output(Um_register c);                                    
static inline void input(Um_register c);                                              
static inline void loadval(Um_instruction instruction);
static inline void loadprog(Um_register b);

static inline uint32_t new_segment(int size);
static inline void free_segment(uint32_t segment_index);
static inline void free_all_segments();
static inline uint32_t get_word(uint32_t segment_index, uint32_t word_index);
static inline void set_word(uint32_t segment_index, uint32_t word_index,
                                             uint32_t word);
static inline void replace_segment_zero(uint32_t new_segment_index);

static inline uint32_t bitpack_get_c(Um_instruction instruction);
static inline uint32_t bitpack_get_b(Um_instruction instruction);
static inline uint32_t bitpack_get_a(Um_instruction instruction);
static inline uint64_t bitpack_new(uint64_t word, unsigned width, unsigned lsb,
                                    uint64_t value);
static inline uint64_t shl(uint64_t word, unsigned bits);
static inline uint64_t shr(uint64_t word, unsigned bits);

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Incorrect usage!\n");
        exit(EXIT_FAILURE);
    }

    struct stat buf;
    
    FILE *fp = fopen(argv[1], "r");
    assert(fp != NULL);

    stat(argv[1], &buf);
    int num_words = buf.st_size / 4;

    segment_zero = read_words(fp, num_words);

    segments = UArray_new(1000, sizeof(UArray_T));

    UArray_T *temp = (UArray_T *)UArray_at(segments, 0);
    *temp = segment_zero;

    next_free_slot = 1;

    available_indices = Seq_new(0);

    execute_program(num_words);

    free_all_segments();

    fclose(fp);
    
    return 0;
}

/* read_words
 * Purpose: Reads the instructions from a file into what will become
            segment 0
 * Parameters: a file pointer and an integer
 * Returns: A UArray
 *
 * Expected input: A file pointer pointing to a file that is filled with
                    valid um instructions, and the number of uint32_t words
                    in that fiile
 * Success output: A UArray_T that contains all of the instructions in the
                    supplied file in the proper order
 * Failure output: none
 */
static inline UArray_T read_words(FILE *fp, int num_words)
{
    UArray_T segment_zero = UArray_new(num_words, sizeof(uint32_t));

    for (int i = 0; i < num_words; i++) {
        uint32_t curr_word = 0;

        for (int j = 24; j >= 0; j -= 8) {
            uint32_t c = fgetc(fp);
            curr_word = bitpack_new(curr_word, 8, j, c);
        }

        *(uint32_t *)UArray_at(segment_zero, i) = curr_word;
    }

    return segment_zero;
}

/* execute_program
 * Purpose: loops through all of the words in m[0] and calls opcode_reader
            on them, updating the program pointer as needed
 * Parameters: none
 * Returns: Nothing
 *
 * Expected input: none
 * Success output: none
 * Failure output: none
 */
static inline void execute_program()
{
    bool continue_execution = true;
    int prog_counter = 0;

    while (continue_execution == true) {
        Um_instruction instruction = *(uint32_t *)UArray_at(segment_zero, prog_counter);
   
        prog_counter++;

        Um_opcode op = instruction >> 28;

        switch(op) {
            case CMOV:
                cmov(bitpack_get_a(instruction), 
                    bitpack_get_b(instruction),
                    bitpack_get_c(instruction));
                continue;
            case SLOAD:
                seg_load(bitpack_get_a(instruction), 
                    bitpack_get_b(instruction),
                    bitpack_get_c(instruction));
                continue;
            case SSTORE:
                seg_store(bitpack_get_a(instruction), 
                    bitpack_get_b(instruction),
                    bitpack_get_c(instruction));
                continue;
            case ADD:
                add(bitpack_get_a(instruction), 
                    bitpack_get_b(instruction),
                    bitpack_get_c(instruction));
                continue;
            case MUL:
                multiply(bitpack_get_a(instruction), 
                    bitpack_get_b(instruction),
                    bitpack_get_c(instruction));
                continue;
            case DIV:
                divide(bitpack_get_a(instruction), 
                    bitpack_get_b(instruction),
                    bitpack_get_c(instruction));
                continue;
            case NAND:
                nand(bitpack_get_a(instruction), 
                    bitpack_get_b(instruction),
                    bitpack_get_c(instruction));
                continue;
            case HALT:
                continue_execution = false;
                continue;
            case ACTIVATE:
                map_seg(bitpack_get_b(instruction),
                    bitpack_get_c(instruction));
                continue;
            case INACTIVATE:
                unmap_seg(bitpack_get_c(instruction));
                continue;
            case OUT:
                output(bitpack_get_c(instruction));
                continue;
            case IN:
                input(bitpack_get_c(instruction));
                continue;
            case LOADP:
                prog_counter = registers[bitpack_get_c(instruction)];
                loadprog(bitpack_get_b(instruction));
                continue;
            case LV:
                loadval(instruction);
                continue;
        }
    }
}

/* cmov
 * Purpose: moves the value in register a into register b if
            register c does not equal 0
 * Parameters: 3 Um_registers named a, b, and c
 * Returns: Nothing
 *
 * Expected input: 3 valid Um_registers, each being a uint32_t
                   between 0 and 7
 * Success output: none (register a is either changed or not)
 * Failure output: none
 */
static inline void cmov(Um_register a, Um_register b, Um_register c)
{
    if (registers[c] != 0) {
        registers[a] = registers[b];
    }
}

/* seg_load
 * Purpose: gets the word from segment m(registers[b]) at index registers[c]
 * Parameters: 3 Um_registers named a, b, and c
 * Returns: Nothing
 *
 * Expected input: 3 valid Um_registers, each being a uint32_t
                   between 0 and 7
 * Success output: none (register a is loaded with the value from the given
                    word in the supplied segment)
 * Failure output: none
 */
static inline void seg_load(Um_register a, Um_register b, Um_register c)
{
    registers[a] = get_word(registers[b], registers[c]);
}

/* seg_store
 * Purpose: stores the word in register c in segment m[register b] at
            index of register c
 * Parameters: 3 Um_registers named a, b, and c
 * Returns: Nothing
 *
 * Expected input: 3 valid Um_registers, each being a uint32_t
                   between 0 and 7
 * Success output: none (the word is properly stored in memory)
 * Failure output: none
 */
static inline void seg_store(Um_register a, Um_register b, Um_register c)
{
    set_word(registers[a], registers[b], registers[c]);
}

/* add
 * Purpose: adds the values in registers b and c, mods the result by 32, and
            places that value in register a
 * Parameters: 3 Um_registers named a, b, and c
 * Returns: Nothing
 *
 * Expected input: 3 valid Um_registers, each being a uint32_t
                   between 0 and 7
 * Success output: none (register a is updated with the sum value)
 * Failure output: none
 */
static inline void add(Um_register a, Um_register b, Um_register c)
{
    uint64_t mod_divisor = (uint64_t)1 << 32;
    registers[a] = (registers[b] + registers[c]) % mod_divisor;
}

/* multiply
 * Purpose: multiplies the values in registers b and c, mods the result by 32, 
            and places that value in register a
 * Parameters: 3 Um_registers named a, b, and c
 * Returns: Nothing
 *
 * Expected input: 3 valid Um_registers, each being a uint32_t
                   between 0 and 7
 * Success output: none (register a is updated with the product)
 * Failure output: none
 */
static inline void multiply(Um_register a, Um_register b, Um_register c)
{
    uint64_t mod_divisor = (uint64_t)1 << 32;
    registers[a] = (registers[b] * registers[c]) % mod_divisor;
}

/* divide
 * Purpose: divides the values in registers b and c, trunkating any decimals,
            and places that value in register a
 * Parameters: 3 Um_registers named a, b, and c
 * Returns: Nothing
 *
 * Expected input: 3 valid Um_registers, each being a uint32_t
                   between 0 and 7
 * Success output: none (register a is updated with the divided value)
 * Failure output: exits the program if given a divisor of 0
 */
static inline void divide(Um_register a, Um_register b, Um_register c)
{
    registers[a] = (registers[b] / registers[c]);
}

/* nand
 * Purpose: Executes a bitwise nand operation on the values in registers
            b and c, then places the result in register a
 * Parameters: 3 Um_registers named a, b, and c
 * Returns: Nothing
 *
 * Expected input: 3 valid Um_registers, each being a uint32_t
                   between 0 and 7
 * Success output: none (register a is updated with the new value)
 * Failure output: none
 */
static inline void nand(Um_register a, Um_register b, Um_register c)
{
    registers[a] = ~(registers[b] & registers[c]);
}

/* map_seg
 * Purpose: Creates a new segment with a number of words equal to the
            given size, and places the segment index in register b
 * Parameters: 2 Um_registers named b and c
 * Returns: Nothing
 *
 * Expected input: 2 valid Um_registers, each being a uint32_t
                   between 0 and 7
 * Success output: none (register b is updated with the proper index)
 * Failure output: none
 */
static inline void map_seg(Um_register b, Um_register c)
{
    registers[b] = new_segment(registers[c]);
}

/* unmap_seg
 * Purpose: unmaps the segment at the given segment rregister
 * Parameters: 1 Um_register named c
 * Returns: Nothing
 *
 * Expected input: 1 valid Um_register, a uint32_t
                   between 0 and 7
 * Success output: none (the given memory segment is unmapped)
 * Failure output: none
 */
static inline void unmap_seg(Um_register c)
{
    free_segment(registers[c]);
}

/* output
 * Purpose: outputs the value in the given register to the console
 * Parameters: 1 Um_register named c
 * Returns: Nothing
 *
 * Expected input: 1 valid Um_register, a uint32_t
                   between 0 and 7
 * Success output: none (value in register c is printed to console)
 * Failure output: Raises an exception if the value supplied is greater
                    than 255
 */
static inline void output(Um_register c)
{
    char output_char = registers[c];

    assert(registers[c] < 256);

    putchar(output_char);
}

/* input
 * Purpose: takes in a char value from I/O and puts it in register c
 * Parameters: 1 Um_register named c
 * Returns: Nothing
 *
 * Expected input: 1 valid Um_register, a uint32_t
                   between 0 and 7
 * Success output: none (register c is updated with the value)
 * Failure output: none
 */
static inline void input(Um_register c)
{
    int character = getchar();

    if (character == EOF) {
        registers[c] = ~0U;
    } else {
        registers[c] = character;
    }
}

/* loadval
 * Purpose: loads the supplied value into the given register
 * Parameters: 1 Um_register named a, and a uint32_t value
 * Returns: Nothing
 *
 * Expected input: 1 valid Um_register, a uint32_t
                   between 0 and 7, and a uint32_t value that is not
                   greater than 25 bits
 * Success output: none (value is placed into register a)
 * Failure output: none
 */
static inline void loadval(Um_instruction instruction)
{
    uint32_t val = instruction << 7;
    val = val >> 7;
    Um_register a = instruction << 4;
    a = a >> 29;

    registers[a] = val;
}

/* loadprog
 * Purpose: m[0] is replaced with the segment at the given index
 * Parameters: 1 Um_register named b
 * Returns: Nothing
 *
 * Expected input: 1 valid Um_register, a uint32_t
                   between 0 and 7,
 * Success output: none (m[0] is replaced (or not) accordingly)
 * Failure output: none
 */
static inline void loadprog(Um_register b)
{
    replace_segment_zero(registers[b]);
}

static inline uint32_t new_segment(int size)
{
    UArray_T segment = UArray_new(size, sizeof(uint32_t));

    if (Seq_length(available_indices) == 0) {
        int length = UArray_length(segments);

        if (next_free_slot == length) {
            UArray_resize(segments, length * 10);
        }

        UArray_T *temp = (UArray_T *)UArray_at(segments, next_free_slot);
        *temp = segment;

        next_free_slot++;
        return (next_free_slot - 1);
    } else {
        uint32_t *available_index_p = (uint32_t *)Seq_remlo(available_indices);
        uint32_t available_index = *available_index_p;
        free(available_index_p);

        UArray_T *temp = (UArray_T *)UArray_at(segments, available_index);
        *temp = segment; 

        return available_index;
    }
}

static inline void free_segment(uint32_t segment_index)
{
    UArray_T *seg = (UArray_T *)UArray_at(segments, segment_index);
    UArray_free(seg);
    seg = NULL;

    uint32_t *available_index = malloc(sizeof(uint32_t));
    assert (available_index != NULL);

    *available_index = segment_index;
    Seq_addlo(available_indices, available_index);
}

static inline void free_all_segments()
{
    for (int i = 0; i < next_free_slot; i++) {
        UArray_T seg = *(UArray_T *)UArray_at(segments, i); 

        if (seg != NULL) {
            UArray_free(&seg);
        }
    }

    UArray_free(&segments);

    int length = Seq_length(available_indices);

    for (int i = 0; i < length; i++) {
        uint32_t *available_index = (uint32_t *)Seq_remlo(available_indices); 
        free(available_index);
    }

    Seq_free(&available_indices);
}

static inline uint32_t get_word(uint32_t segment_index, uint32_t word_index)
{
    UArray_T seg = *(UArray_T *)UArray_at(segments, segment_index);

    uint32_t word = *(uint32_t *)UArray_at(seg, word_index);
   
    return word;
}

static inline void set_word(uint32_t segment_index, uint32_t word_index, uint32_t word)
{
    UArray_T seg = *(UArray_T *)UArray_at(segments, segment_index);

    *(uint32_t *)UArray_at(seg, word_index) = word;
}

static inline void replace_segment_zero(uint32_t new_segment_index)
{
    if (new_segment_index == 0) {
        return;
    }

    UArray_T seg = *(UArray_T *)UArray_at(segments, new_segment_index);
    UArray_T new_seg_zero = UArray_copy(seg, UArray_length(seg));

    UArray_T *temp = (UArray_T *)UArray_at(segments, 0);
    *temp = new_seg_zero;

    UArray_free(&segment_zero);

    segment_zero = new_seg_zero;
}

static inline uint32_t bitpack_get_c(Um_instruction instruction)
{
    instruction = instruction << 29;
    return instruction >> 29;
}

static inline uint32_t bitpack_get_b(Um_instruction instruction)
{
    instruction = instruction >> 3;
    instruction = instruction << 29;
    return instruction >> 29;
}

static inline uint32_t bitpack_get_a(Um_instruction instruction)
{
    instruction = instruction >> 6;
    instruction = instruction << 29;
    return instruction >> 29;
}

static inline uint64_t bitpack_new(uint64_t word, unsigned width, unsigned lsb,
                                    uint64_t value)
{
        unsigned hi = lsb + width; /* one beyond the most significant bit */
        return shl(shr(word, hi), hi)                 /* high part */
                | shr(shl(word, 64 - lsb), 64 - lsb)  /* low part  */
                | (value << lsb);                     /* new part  */
}

static inline uint64_t shl(uint64_t word, unsigned bits)
{
        assert(bits <= 64);
        if (bits == 64)
                return 0;
        else
                return word << bits;
}

static inline uint64_t shr(uint64_t word, unsigned bits)
{
        assert(bits <= 64);
        if (bits == 64)
                return 0;
        else
                return word >> bits;
}