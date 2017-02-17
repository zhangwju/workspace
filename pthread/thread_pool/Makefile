#
#Design of thread pool
#

CFLAGS += -g
LDFLAGS += -lpthread

OBJS += thpool.o

TARGET = thpool

RM =rm -f

$(TARGET):$(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o:%.c
	$(CC) $(CFLAGS) -c -o $@ $<


.PHONY:clean

clean:
	$(RM) $(OBJS) $(TARGET)
	
