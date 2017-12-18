#
# Design of net_monitor
# Author: zhangwj
# Date: 2017-10-27
#

LDFLAGS = -lpthread
TARGET_CFLAGS = -g #-Wall
FPIC = -fPIC -c
SHARED = -shared -o
BINDIR = ./bin
CONFDIR = ./conf
INSTALL_BIN_PATH = /usr/sbin/
INSTALL_CONF_PATH = /etc/

CFLAGS += $(TARGET_CFLAGS) \
	-I$(shell pwd)/src

SRC = $(wildcard src/*.c)
#OBJECT = $(patsubst %.c,%.o,$(notdir $(SRC)))
OBJECT = $(patsubst %.c,%.o, $(SRC))

TARGET = net_monitor

#clean objs
RM = rm -f  

all: $(TARGET)
 
$(CURR_PATH)/src/%.o:%c
	$(CC) $(FPIC) $^ $(CFLAGS)

$(TARGET): $(OBJECT)
	$(CC) -o ${BINDIR}/$@ $^ $(LDFLAGS)

.PHONY:clean

install:
	@install -m 0600 $(CONFDIR)/net_monitor $(INSTALL_CONF_PATH)
	@install -m 0700 $(BINDIR)/$(TARGET) $(INSTALL_BIN_PATH)
clean:
	$(RM) $(BINDIR)/$(TARGET) $(OBJECT)                             
	
