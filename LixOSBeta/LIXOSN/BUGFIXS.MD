# LixOS Bugfixes and Improvements

## Fixed Issues
- **Screen Inversion:** Fixed `vga_flip` in `kernel/vga.c` to blit the buffer in correct top-to-bottom order.
- **Mouse Inversion:** Corrected `mouse_update` in `kernel/mouse.c` to use standard, non-inverted coordinates.
- **UI Quality:**
    - Improved window borders with a 3D effect (Windows 3.x style).
    - Refined the color palette for a more professional appearance.
    - Enhanced desktop icons with better rendering and labels.
- **File Operations:** Implemented basic "Open" and "Save" logic in Notepad.

## New Applications
- **Calculator:** A basic arithmetic calculator (`apps/calc.c`).
- **File Manager:** A file browser to list files on the disk (`apps/fman.c`).

## Build System
- Verified compatibility with **ARM64 Ubuntu** via **Box64** and **DJGPP**.
- Updated `Makefile` to include the new applications.
