#================================================================
#  Makefile  -  LixOS / Ubuntu ARM64 + Box64 + DJGPP cross-compiler
#
#  Kullanim:
#    make          -> Derle
#    make run      -> DOSBox ile calistir
#    make clean    -> Temizle
#    make rebuild  -> Temizle + Derle
#
#  ARM64 notu:
#    DJGPP binary'leri x86_64 icin derlenmiştir.
#    ARM64 Ubuntu'da Box64 ile calistiriyoruz.
#    box64 i586-pc-msdosdjgpp-gcc seklinde kullanilir.
#================================================================

# ---- Box64 varsa kullan, yoksa direkt cagir (x86_64) ----
# ARM64: BOX64 = box64
# x86_64: BOX64 = (bos)
BOX64   = box64

# ---- Cross-compiler ----
CC      = $(BOX64) i586-pc-msdosdjgpp-gcc
CFLAGS  = -O2 -Wall -DDOS \
          -fno-stack-protector \
          -march=i386

# ---- Kaynak dosyalari ----
KERNEL_SRCS = kernel/vga.c \
              kernel/mouse.c \
              kernel/window.c

APP_SRCS    = apps/notepad.c \
              apps/lixver.c  \
              apps/setup.c   \
              apps/calc.c    \
              apps/fman.c

MAIN_SRC    = main.c

ALL_SRCS    = $(MAIN_SRC) $(KERNEL_SRCS) $(APP_SRCS)

# .c -> .o listesi
OBJS = $(ALL_SRCS:.c=.o)

# ---- Hedef ----
TARGET = LIXOS.EXE

# ---- Dizinler ----
DIST_DIR = dist

# ================================================================
# Ana hedef
# ================================================================
all: $(TARGET)
	@echo ""
	@echo "  Build basarili: $(TARGET)"
	@echo "  Test icin: make run"
	@echo ""

$(TARGET): $(OBJS)
	@echo "[LINK] $@"
	$(CC) $(CFLAGS) -o $@ $^

# ---- Genel derleme kurali ----
%.o: %.c
	@echo "[CC]   $<"
	$(CC) $(CFLAGS) -c $< -o $@

# ---- Bagimlılıklar (header degisirse yeniden derle) ----
kernel/vga.o:    kernel/vga.c    kernel/vga.h
kernel/mouse.o:  kernel/mouse.c  kernel/mouse.h kernel/vga.h
kernel/window.o: kernel/window.c kernel/window.h kernel/vga.h kernel/mouse.h
apps/notepad.o:  apps/notepad.c  apps/notepad.h  kernel/vga.h kernel/window.h kernel/mouse.h
apps/lixver.o:   apps/lixver.c   apps/lixver.h   kernel/vga.h kernel/window.h kernel/mouse.h
apps/setup.o:    apps/setup.c    apps/setup.h    kernel/vga.h kernel/window.h kernel/mouse.h
apps/calc.o:     apps/calc.c     apps/calc.h     kernel/vga.h kernel/window.h kernel/mouse.h
apps/fman.o:     apps/fman.c     apps/fman.h     kernel/vga.h kernel/window.h kernel/mouse.h
main.o:          main.c kernel/vga.h kernel/mouse.h kernel/window.h \
                 apps/notepad.h apps/lixver.h apps/setup.h apps/calc.h apps/fman.h

# ================================================================
# DOSBox ile test
# ================================================================
run: $(TARGET)
	@echo "DOSBox ile baslatiliyor..."
	@mkdir -p $(DIST_DIR)
	@cp $(TARGET) $(DIST_DIR)/
	@cp CWSDPMI.EXE $(DIST_DIR)/ 2>/dev/null || \
	    echo "  UYARI: CWSDPMI.EXE bulunamadi, DOSBox olmadan calisir."
	dosbox -conf dosbox.conf $(DIST_DIR)/$(TARGET) -exit

# ================================================================
# Temizlik
# ================================================================
clean:
	@echo "Temizleniyor..."
	rm -f $(OBJS) $(TARGET)
	rm -rf $(DIST_DIR)
	@echo "Temiz."

rebuild: clean all

# ================================================================
# CWSDPMI.EXE indir (DJGPP DPMI host, LIXOS.EXE icin gerekli)
# ================================================================
cwsdpmi:
	@echo "CWSDPMI indiriliyor..."
	wget -q "https://github.com/andrewwutw/build-djgpp/releases/download/v3.4/cwsdpmi.zip" \
	     -O /tmp/cwsdpmi.zip || \
	wget -q "http://www.delorie.com/pub/djgpp/v2misc/csdpmi7b.zip" \
	     -O /tmp/cwsdpmi.zip
	unzip -o /tmp/cwsdpmi.zip CWSDPMI.EXE -d . 2>/dev/null || \
	unzip -oj /tmp/cwsdpmi.zip '*/CWSDPMI.EXE' -d . 2>/dev/null
	@echo "CWSDPMI.EXE hazir."

# ================================================================
# Dagitim paketi olustur
# ================================================================
dist: all
	mkdir -p $(DIST_DIR)
	cp $(TARGET) $(DIST_DIR)/
	cp README.TXT $(DIST_DIR)/
	cp AUTOEXEC.BAT $(DIST_DIR)/
	cp CONFIG.SYS $(DIST_DIR)/
	cp CWSDPMI.EXE $(DIST_DIR)/ 2>/dev/null || true
	@echo ""
	@echo "  Dagitim paketi: $(DIST_DIR)/"
	@ls -lh $(DIST_DIR)/

.PHONY: all run clean rebuild cwsdpmi dist
