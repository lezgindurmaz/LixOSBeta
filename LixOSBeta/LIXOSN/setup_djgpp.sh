#!/bin/bash
#================================================================
#  setup_djgpp.sh  -  Ubuntu ARM64'te DJGPP cross-compiler kurulumu
#
#  ARM64 notu:
#    DJGPP binary'leri x86_64 icin build edilmistir.
#    ARM64'te Box64 ile calistiriyoruz (transparan emulasyon).
#    Yani: box64 i586-pc-msdosdjgpp-gcc ...
#
#  Kullanim: chmod +x setup_djgpp.sh && ./setup_djgpp.sh
#================================================================

set -e  # Hata olursa dur

DJGPP_PREFIX="/usr/local/djgpp"
DJGPP_VER="gcc1220"   # GCC 12.2.0
# ARM64'te bile linux64 (x86_64) binary indiriyoruz — Box64 calistiracak
TARBALL="djgpp-linux64-${DJGPP_VER}.tar.bz2"
DOWNLOAD_URL="https://github.com/andrewwutw/build-djgpp/releases/download/v3.4/${TARBALL}"

echo "=================================================="
echo "  LixOS - DJGPP Kurulum Scripti (ARM64 / Box64)"
echo "=================================================="
echo ""

# ---- Mimari kontrol ----
ARCH=$(uname -m)
echo "  Mimari: ${ARCH}"
if [ "${ARCH}" = "aarch64" ] || [ "${ARCH}" = "arm64" ]; then
    echo "  ARM64 tespit edildi -> Box64 kullanilacak"
    USE_BOX64=1
else
    echo "  x86_64 tespit edildi -> Box64 gerekmez"
    USE_BOX64=0
fi
echo ""

# ---- 1. Bağımlılıklar ----
echo "[1/5] Gerekli paketler kuruluyor..."
sudo apt-get update -qq
sudo apt-get install -y \
    wget curl \
    make \
    dosbox \
    2>/dev/null
echo "    OK"

# ---- 2. Box64 kur (ARM64 ise) ----
if [ "${USE_BOX64}" = "1" ]; then
    echo "[2/5] Box64 kuruluyor..."
    if command -v box64 &>/dev/null; then
        echo "    Box64 zaten kurulu: $(box64 --version 2>&1 | head -1)"
    else
        # Box64 resmi PPA
        sudo apt-get install -y box64 2>/dev/null || {
            echo "    PPA'dan denenecek..."
            sudo add-apt-repository -y ppa:kxxt/box64 2>/dev/null || true
            sudo apt-get update -qq
            sudo apt-get install -y box64 2>/dev/null || {
                echo ""
                echo "  HATA: Box64 otomatik kurulamadi."
                echo "  Manuel kurulum icin: https://github.com/ptitSeb/box64"
                echo "  Raspberry Pi / Ubuntu ARM icin:"
                echo "    sudo apt install box64-generic-arm"
                exit 1
            }
        }
        echo "    OK"
    fi
else
    echo "[2/5] x86_64 sistemi, Box64 atlaniyor..."
fi

# ---- 3. DJGPP indir ----
echo "[3/5] DJGPP indiriliyor (linux64 / ${DJGPP_VER})..."
if [ ! -f "/tmp/${TARBALL}" ]; then
    wget --show-progress -O "/tmp/${TARBALL}" "${DOWNLOAD_URL}"
else
    echo "    Zaten indirilmis, atlaniyor."
fi
echo "    OK"

# ---- 4. DJGPP kur ----
echo "[4/5] DJGPP ${DJGPP_PREFIX} altina kuruluyor..."
sudo mkdir -p "${DJGPP_PREFIX}"
sudo tar xjf "/tmp/${TARBALL}" -C "${DJGPP_PREFIX}" --strip-components=1
echo "    OK"

# ---- 5. PATH ve ortam ayarla ----
echo "[5/5] PATH ayarlaniyor..."

PROFILE_LINE="export PATH=\"${DJGPP_PREFIX}/bin:\$PATH\""
DJGPP_ENV_LINE="export DJGPP=\"${DJGPP_PREFIX}/etc/djgpp.env\""

if ! grep -q "djgpp" ~/.bashrc 2>/dev/null; then
    echo "" >> ~/.bashrc
    echo "# DJGPP cross-compiler (LixOS)" >> ~/.bashrc
    echo "${PROFILE_LINE}" >> ~/.bashrc
    echo "${DJGPP_ENV_LINE}" >> ~/.bashrc
    echo "    ~/.bashrc guncellendi"
fi

export PATH="${DJGPP_PREFIX}/bin:$PATH"
export DJGPP="${DJGPP_PREFIX}/etc/djgpp.env"
echo "    OK"
echo ""

# ---- Doğrulama ----
echo "=================================================="
echo "  Kurulum tamamlandi! Dogrulama:"
echo "=================================================="

GCC_BIN="${DJGPP_PREFIX}/bin/i586-pc-msdosdjgpp-gcc"

if [ "${USE_BOX64}" = "1" ]; then
    # ARM64: box64 ile test et
    if box64 "${GCC_BIN}" --version &>/dev/null; then
        echo "  box64 i586-pc-msdosdjgpp-gcc --version:"
        box64 "${GCC_BIN}" --version | head -1
        echo ""
        echo "  Kullanim: make"
        echo "  (Makefile BOX64=box64 ile ayarli)"
    else
        echo "  UYARI: Test basarisiz. source ~/.bashrc sonra tekrar dene."
    fi
else
    if command -v i586-pc-msdosdjgpp-gcc &>/dev/null; then
        i586-pc-msdosdjgpp-gcc --version | head -1
    fi
fi

echo ""
echo "  Simdi yeni terminal ac veya: source ~/.bashrc"
echo "  Sonra: cd LIXOS && make"
echo "=================================================="
