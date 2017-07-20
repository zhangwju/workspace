#
# Design of mgnl_route
#

LDFLAGS = -lpthread
TARGET_CFLAGS = -g -Wall
FPIC = -fPIC -c
SHARED = -shared -o

CFLAGS += $(TARGET_CFLAGS) \
	-I$(shell pwd) 

SRC = $(wildcard ./*.c)
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
