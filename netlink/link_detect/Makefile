RM = /bin/rm -f
FPIC = -fPIC -c
SHARED = -shared -o
TARGET_CFLAGS = -g #-Wall
# target
TARGET = link_detect

# list of generated object files
################################
SRC = $(wildcard ./*.c)
OBJECT = $(patsubst %.c,%.o,$(notdir $(SRC)))

# Header include
##############################
CFLAGS += $(TARGET_CFLAGS) \
		-I$(shell pwd)

# Library addition
##################
LDFLAGS += -lpthread \
		-lubox \
		-lubus \
		-lnanomsg
	
all: $(TARGET)

$(OBJECT):$(SRC)
	$(CC) $(FPIC) $^ $(CFLAGS)

$(TARGET): $(OBJECT)
	$(CC) -o $@ $^ $(LDFLAGS)
	$(CC) -o linkserver test/test.c $(LDFLAGS) $(CFLAGS)

clean: 
	$(RM) $(TARGET) $(OBJECT)
