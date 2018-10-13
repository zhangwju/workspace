#
# Design of curlbat
#

LDFLAGS = -lpthread
FPIC = -fPIC -c -g -Wall
SHARED = -shared -o

CFLAGS += $(TARGET_CFLAGS) \
        -I$(shell pwd)/src

SRC = $(wildcard src/*.c)
OBJECT = $(patsubst %.c,%.o,$(notdir $(SRC)))

TARGET = test

#clean objs
RM = rm -f  

all: $(TARGET)
 
$(OBJECT):$(SRC)
	$(CC) $(FPIC) $^ $(CFLAGS)
$(TARGET): $(OBJECT)
	$(CC) -o $@ $^ $(LDFLAGS)

.PHONY:clean

clean:
	$(RM) $(TARGET) $(OBJECT)
