================================================================
  LixOS v1.00  -  MS-DOS 6.22 icin Masaustu Ortami
  Build: 260426
================================================================

DOSYALAR
--------
  LIXUI.H      Ortak UI kutuphanesi (tum programlar bu dosyayi kullanir)
  LIXOS.C      Ana masaustu kabugu
  NOTEPAD.C    Metin duzenleyici
  LIXVER.C     Surum ve sistem bilgisi
  SETUP.C      Kurulum sihirbazi
  BUILD.BAT    Otomatik derleme scripti (Turbo C)
  README.TXT   Bu dosya

GEREKSINIMLER
-------------
  - MS-DOS 6.22
  - Turbo C 2.0 veya 3.0 (BORLAND)
  - En az 512 KB konvansiyonel bellek
  - PS/2 veya seri mouse (istege bagli, MOUSE.SYS gerekli)

DERLEME
-------
  1. Turbo C'yi yukleyin (TCC.EXE PATH'te olmali)
  2. BUILD.BAT dosyasini calistirin:
       BUILD.BAT
  3. Ciktı: LIXOS.EXE, NOTEPAD.EXE, LIXVER.EXE, SETUP.EXE

  Tek tek derlemek icin:
       TCC -ml LIXOS.C
       TCC -ml NOTEPAD.C
       TCC -ml LIXVER.C
       TCC -ml SETUP.C

KURULUM
-------
  1. EXE dosyalarini A: disketine kopyalayin
  2. MS-DOS 6.22 ile bilgisayari baslatın
  3. A:\SETUP.EXE calistirin
  4. Adim adim talimatları takip edin

MOUSE DESTEGI
-------------
  INT 33h uzerinden PS/2, seri ve bus mouse desteklenir.
  Setup, CONFIG.SYS'e otomatik olarak su satiri ekler:
       DEVICE=C:\DOS\MOUSE.SYS /Y
  Mouse surucusu yoksa Notepad ve masaustu klavye ile kullanilabilir.

KLAVYE KISAYOLLARI  (Masaustu)
------------------------------
  1-4          Programi direk ac
  TAB / Ok     Ikon sec
  ENTER        Secili ikonu ac
  ALT+L        LixOS menusu
  ALT+D        Dosyalar menusu
  ALT+P        Programlar menusu
  ALT+S        Sistem menusu
  ALT+Y        Yardim menusu
  F1           Yardim
  F3           Cikis

KLAVYE KISAYOLLARI  (Notepad)
------------------------------
  Ctrl+N       Yeni dosya
  Ctrl+O       Dosya ac
  Ctrl+S       Kaydet
  Ctrl+F / F5  Metin bul
  F3           Sonraki bul
  F2           Kaydet
  Ins          Ekle / Uzerine yaz modu
  Ctrl+Home    Dosya basi
  Ctrl+End     Dosya sonu
  Ctrl+Sol/Sag Kelime kelime git
  ESC          Menu cubuguna odaklan

LIXVER SEKMELERI
----------------
  1 / TAB      LixOS surum bilgisi
  2            Sistem (CPU, DOS, BIOS)
  3            Bellek (konv. + genisletilmis)
  4            Disk (C: kullanim durumu)
  Y            Bilgileri yenile
  ESC / Q      Kapat

================================================================
  LixOS - Acik kaynak, Turbo C ile yazilmistir.
================================================================
