#
# Design of curlbat
#

CFLAGS = -g
LDFLAGS = 

OBJS = nl_netinfo.o
TARGET = nl-net

#clean objs
RM = rm -f  
 
$(TARGET):$(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o:%.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY:clean

clean:
	$(RM) $(TARGET) $(OBJS)
