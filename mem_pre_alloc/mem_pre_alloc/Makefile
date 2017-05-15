# Makefile for packet-analyzer

CFLAGS:=$(CFLAGS)

RM = /bin/rm -f 

# list of generated object files for paket-analyzer. 
OBJS = pkt_al_main.o pkt_al_mem.o
 
%.o:%.c
	$(CC) $(CFLAGS) -g $(FPIC) -c -o $@ $<

# program executables. 
NAME = pre-mem 

all: $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $(NAME) 

clean: 
	$(RM) $(NAME) $(OBJS)

