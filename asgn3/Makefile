CC       = clang
FORMAT   = clang-format
CFLAGS   = -Wall -Wpedantic -Werror -Wextra

SRC_FILES = queue.c rwlock.c
HEADER_FILES = queue.h rwlock.h

.PHONY: all  clean format queue rwlock

all: queue rwlock

queue: queue.c queue.h
	$(CC) $(CFLAGS) -c queue.c
	
rwlock: rwlock.c rwlock.h
	$(CC) $(CFLAGS) -c rwlock.c

%.o : %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f queue.o rwlock.o

format:
	clang-format -i $(SRC_FILES) $(HEADER_FILES)

#format: $(FORMATS)

#%.fmt: %.c
#	$(FORMAT) -i $<
#	touch $@

	
