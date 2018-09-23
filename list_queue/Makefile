#
#Design of datasync
#

FPIC = -fPIC -c
#SHARED = -shared -o
TARGET_CFLAGS = -g -Wall

TARGET = test 

SRC = $(wildcard *.c)
OBJECT = $(patsubst %.c,%.o,$(notdir $(SRC)))


CFLAGS += $(TARGET_CLFAGS) \
#        -I$(shell pwd)/include

LDFLAGS += -lpthread


all: $(TARGET)
$(OBJECT):$(SRC)
	$(CC) $(FPIC) $^ $(CFLAGS)

$(TARGET): $(OBJECT)
	$(CC) -o $@ $^ $(LDFLAGS)

clean: 
	$(RM) $(TARGET) $(OBJECT)
