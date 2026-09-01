APP_BINARY := sg-drm-modeset

SRC_DIR := src
INC_DIR := include

SRCS := \
	$(SRC_DIR)/main.c

OBJS := $(SRCS:.c=.o)

PKG_CONFIG ?= pkg-config

CFLAGS += -I$(INC_DIR)
CFLAGS += $(shell $(PKG_CONFIG) --cflags libdrm)

LDFLAGS += $(shell $(PKG_CONFIG) --libs libdrm)

.PHONY: clean

$(APP_BINARY): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(APP_BINARY)
