# LixOS Bugfixes and Improvements

## Fixed Issues
- **Screen & Text Inversion:**
    - Fixed `vga_flip` in `kernel/vga.c` to blit the buffer in correct top-to-bottom order.
    - Fixed `vga_putchar` to use LSB-first bit order, resolving mirrored/inverted text.
- **Text Quality:** Replaced the minimal font with a higher-quality, more readable 8x8 font.
- **Application Launching:**
    - Improved double-click detection logic in `main.c`.
    - Corrected drawing order: windows are now drawn *before* application content to prevent overwriting.
- **Mouse Interaction:** Fixed `mouse_update` coordinates and ensured `wm_process_mouse` doesn't block other handlers.
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
