#
# Makefile for the UM
#    Authors:  Eli Intriligator (eintri01), Max Behrendt (mbehre01)
#    Date:     Nov 23, 2021
# 
CC = gcc

IFLAGS  = -I/comp/40/build/include -I/usr/sup/cii40/include/cii
CFLAGS  = -g -std=gnu99 -Wall -Wextra -Werror -pedantic $(IFLAGS)
LDFLAGS = -g -L/comp/40/build/lib -L/usr/sup/cii40/lib64
LDLIBS  = -lcii40-O2 -l40locality -lcii40 -lm -lbitpack

# Optimization flags
PROFILEFLAGS = -O2

all: um

um: um-main.o
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

# To get *any* .o file, compile its .c file with the following rule.
%.o: %.c
	$(CC) $(CFLAGS) -c $(PROFILEFLAGS) $< -o $@

clean:
	rm -f $(EXECS)  *.o

