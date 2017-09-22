RM = /bin/rm -f
fPIC = -fPIC -c
SHARED = -shared -o
TARGET_CFLAGS = -g

# OBJS list
################################
SRC = $(wildcard ./*c)
OBJECT = $(patsubst %.c,%.o,$(notdir $(SRC)))

# Header include 
###############################
CFLAGS += $(TARGET_CFLAGS) \
		-I$(shell pwd)

# Librarys links
###############################
LDFLAGS += -lpthread -luci

#TARGET FILE
TARGET = uci_opt

all: $(TARGET)

$(OBJECT):$(SRC)
	$(CC) $(fPIC) $^ $(CFLAGS)

$(TARGET):$(OBJECT)
	$(CC) -o $@ $^ $(LDFLAGS)

clean: 
	$(RM) $(TARGET) $(OBJECT)
	
