import pycdlib
import os

def create_iso():
    iso = pycdlib.PyCdlib()
    # Joliet destegi ile baslat (Windows/DOS uyumlulugu icin iyi olur)
    iso.new(interchange_level=3, joliet=3)

    source_dir = "LixOS_src"
    files = [
        "LIXOS.C", "NOTEPAD.C", "LIXVER.C", "SETUP.C", "LIXUI.H",
        "FILEMAN.C", "CALC.C", "SNAKE.C", "CALENDAR.C", "CLOCK.C",
        "BUILD.BAT", "README.TXT", "CTMOUSE.EXE"
    ]

    for file in files:
        path = os.path.join(source_dir, file)
        if os.path.exists(path):
            # ISO'da dosya isimleri 8.3 formatina yakin olmali veya Joliet kullanilmali
            # SETUP.C -> /SETUP.C;1
            iso.add_file(path, f'/{file};1', joliet_path=f'/{file}')

    iso.write('LixOS_v1.00.iso')
    iso.close()
    print("ISO olusturuldu: LixOS_v1.00.iso")

if __name__ == "__main__":
    create_iso()
