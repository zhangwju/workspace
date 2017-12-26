RM = /bin/rm -f
FPIC = -fPIC -c
SHARED = -shared -o
TARGET_CFLAGS = -g #-Wall
# target
TARGET = test 

# list of generated object files
################################
SRC = $(wildcard ./*.c)
OBJECT = $(patsubst %.c,%.o,$(notdir $(SRC)))

# Header include
##############################
CFLAGS += $(TARGET_CFLAGS) \
		-I$(shell pwd)/include

# Library addition
##################
LDFLAGS += -lpthread 
	
all: $(TARGET)

$(OBJECT):$(SRC)
	$(CC) $(FPIC) $^ $(CFLAGS)

$(TARGET): $(OBJECT)
	$(CC) -o $@ $^ $(LDFLAGS)

clean: 
	$(RM) $(TARGET) $(OBJECT)
