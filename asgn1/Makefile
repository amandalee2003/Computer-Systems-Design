#CC = clang
#CFLAGS = -Wall -Werror -Wpedantic -Wextra -pedantic -gdwarf-4

#.PHONY: clean all format spotless

#all: memory

#memory: memory.o
#	$(CC) -o $@ $^

#%.o: %.c
#	$(CC) $(CFLAGS) -c $<

#$(EXECBIN): $(OBJECTS)
#	$(CC) -o $@ $^
#clean:
#	rm -f memory *.o

#spotless: clean
#	rm -f $(EXECBIN) memory
#$format:
#	clang-format -i -style=file *.[ch]

	


EXECBIN  = memory
SOURCES  = $(wildcard *.c)
OBJECTS  = $(SOURCES:%.c=%.o)
FORMATS  = $(SOURCES:%.c=%.fmt)

CC       = clang
FORMAT   = clang-format
CFLAGS   = -Wall -Wpedantic -Werror -Wextra -pedantic -gdwarf-4

.PHONY: all memory clean format

all: $(EXECBIN)

$(EXECBIN): $(OBJECTS)
	$(CC) -o $@ $^

#memory: memory.o
#	$(CC) -o $@ $^

%.o : %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(EXECBIN) $(OBJECTS)

format: $(FORMATS)

%.fmt: %.c
	$(FORMAT) -i $<
	touch $@

