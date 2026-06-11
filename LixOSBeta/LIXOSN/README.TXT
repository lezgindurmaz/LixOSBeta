================================================================
  LixOS v1.0  -  Lightweight Interactive eXtended OS Shell
  For MS-DOS 6.22 / DJGPP 2.x
================================================================

GEREKSINIMLER
-------------
- MS-DOS 6.22
- DJGPP 2.x (gcc, ld, make)
- CWSDPMI.EXE  (DJGPP ile birlikte gelir, C:\LIXOS\ altina koy)
- DOS mouse surucusu (MOUSE.COM veya MOUSE.SYS)
- VGA uyumlu ekran karti

DOSYA YAPISI
------------
LIXOS/
  MAIN.C              Ana shell ve desktop dongusu
  BUILD.BAT           Derleme scripti
  AUTOEXEC.BAT        Otomatik baslangic
  CONFIG.SYS          DOS yapilandirmasi
  KERNEL/
    VGA.H / VGA.C     Mode 13h grafik motoru (320x200, 256 renk)
    MOUSE.H / MOUSE.C DOS INT 33h mouse surucusu
    WINDOW.H / WINDOW.C  Win 3.1 tarzi pencere yoneticisi
  APPS/
    NOTEPAD.H / NOTEPAD.C  Mavi arkaplanli metin duzenleyici
    LIXVER.H / LIXVER.C    Surum bilgisi penceresi
    SETUP.H  / SETUP.C     Kurulum / HDD sifirlama araci

DERLEME
-------
1. DJGPP'yi kurun ve PATH'e ekleyin
   SET PATH=C:\DJGPP\BIN;%PATH%
   SET DJGPP=C:\DJGPP\DJGPP.ENV

2. LixOS klasorune girin:
   CD C:\LIXOS

3. Derleyin:
   BUILD.BAT

4. Calistirin (mouse surucusu yuklu olmali):
   MOUSE.COM
   LIXOS.EXE

KULLANIM
--------
- Masaustu ikonlarina CIFT TIKLAYIN veya Start menusunu kullanin
- Pencereleri baslik cubugunden surukleyebilirsiniz
- [X] butonu pencereyi kapatir
- ESC tusu LixOS'u kapatir (DOS'a doner)

NOTPAD
------
- Metin girebilir, duzenleme yapabilirsiniz
- Ok tuslari ile gezin, Enter ile yeni satir
- File menusu -> New : temizler
- File menusu -> Close : kapatir

SETUP
-----
- Ilk kurulumda: FDISK -> FORMAT -> Dosya kopyalama adimlarini goster
- Zaten kuruluysa: Uyari gosterir, "YES" yazmanizi ister (buyuk harf)
  Onay sonrasi FDISK ve FORMAT C: /S calistirilir

LIXVER
------
- LixOS surum ve sistem bilgilerini gosterir

TEKNIK NOTLAR
-------------
- DJGPP, 32-bit protected mode (DPMI) kullanir
- VGA buffer (320x200 = 64000 byte) memory'de tutulur, her karede
  tek memcpy() ile ekrana aktarilir (double buffering)
- Mouse INT 33h: x koordinati VGA mode 13h'da 0-639 gelir, /2 yapilir
- Font: 8x8 piksel, ASCII 32-126 arasi, kaynak kod icinde tanimli

================================================================
