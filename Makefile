# Luan Machado Bernardt | GRR20190363

CC     = g++
CFLAGS =

PROG = escalona
OBJS = escalona.o schedule_funcs.o

.PHONY: clean purge all

all: $(PROG)

%.o: %.c
	$(CC) -c $(CFLAGS) $<

$(PROG) : % :  $(OBJS) %.o
	$(CC) -o $@ $^

clean:
	@rm -f *.o

purge:   
	@rm -f $(PROG)