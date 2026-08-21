CC	?= $(CROSS_COMPILE)gcc
LD	?= $(CROSS_COMPILE)ld
AR	?= $(CROSS_COMPILE)ar
NM	?= $(CROSS_COMPILE)nm
OBJCOPY	?= $(CROSS_COMPILE)objcopy
OBJDUMP	?= $(CROSS_COMPILE)objdump
READELF	?= $(CROSS_COMPILE)readelf

OBJS = main.o sg_serialization.o

# Keep host/include before ta/include so the host uses its own headers
# when both host and TA provide headers with the same name.
CFLAGS += -Wall -I./include -I../ta/include -I$(TEEC_EXPORT)/include


#CFLAGS += -Wall -I ../ta/include -I$(TEEC_EXPORT)/include -I./include

LDADD += -lteec -L$(TEEC_EXPORT)/lib

BINARY = sg-tee-abi-test

.PHONY: all
all: $(BINARY)

$(BINARY): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDADD)

.PHONY: clean
clean:
	rm -f $(OBJS) $(BINARY)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
