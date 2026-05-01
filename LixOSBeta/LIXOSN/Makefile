#================================================================
#  Makefile  -  LixOS / Ultimate Library-based Linking System
#================================================================

BOX64   = box64
CC      = $(BOX64) i586-pc-msdosdjgpp-gcc
AR      = $(BOX64) i586-pc-msdosdjgpp-ar
CFLAGS  = -O2 -Wall -DDOS -fno-stack-protector -march=i386

# ---- Kernel Objects ----
KERNEL_OBJS = kernel/vga.o \
              kernel/mouse.o \
              kernel/window.o

# ---- App Objects ----
APP_OBJS    = apps/notepad.o \
              apps/lixver.o  \
              apps/setup.o   \
              apps/calc.o    \
              apps/fman.o

# ---- Library ----
LIBLIX = liblix.a

# ---- Final Target ----
TARGET = LIXOS.EXE

all: $(TARGET)
	@echo "Build successful: $(TARGET)"

$(LIBLIX): $(KERNEL_OBJS) $(APP_OBJS)
	@echo "[AR]   $@"
	$(AR) rcs $@ $^

$(TARGET): main.o $(LIBLIX)
	@echo "[LINK] $@"
	$(CC) $(CFLAGS) -o $@ main.o -L. -llix

%.o: %.c
	@echo "[CC]   $<"
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(KERNEL_OBJS) $(APP_OBJS) main.o $(LIBLIX) $(TARGET)
	rm -rf dist

run: $(TARGET)
	dosbox -conf dosbox.conf $(TARGET) -exit

.PHONY: all clean run
