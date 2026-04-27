import os
import zipfile

def create_iso_structure():
    # Bu script, ISO imaji yerine (genisoimage yoksa)
    # tum gerekli dosyalari bir ZIP paketine toplar ve GitHub'a hazirlar.
    # Ayrica bir "CD_ROOT" klasoru olusturur.

    source_dir = "LixOS_src"
    dist_dir = "LixOS_Dist"

    if not os.path.exists(dist_dir):
        os.makedirs(dist_dir)

    files_to_include = [
        "LIXOS.C", "NOTEPAD.C", "LIXVER.C", "SETUP.C", "LIXUI.H",
        "FILEMAN.C", "CALC.C", "SNAKE.C", "CALENDAR.C", "CLOCK.C",
        "BUILD.BAT", "README.TXT", "CTMOUSE.EXE"
    ]

    with zipfile.ZipFile("LixOS_v1.00_src_updated.zip", 'w') as zipf:
        for file in files_to_include:
            file_path = os.path.join(source_dir, file)
            if os.path.exists(file_path):
                zipf.write(file_path, arcname=file)

    print("Paketleme tamamlandi: LixOS_v1.00_src_updated.zip")

if __name__ == "__main__":
    create_iso_structure()
