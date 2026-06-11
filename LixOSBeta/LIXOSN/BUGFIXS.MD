# LixOS Bugfixes and Improvements (Ultimate Update)

## Major Architectural Overhaul
- **New App Linking System:** Implemented a robust library-based linking system.
    - Applications now use a callback system (`update_cb`) registered with the Window Manager.
    - Objects are compiled into a static library (`liblix.a`) ensuring reliable connections between the kernel and applications.
    - Standardized drawing order: Window frame -> Application content.

## Fixed Issues
- **Text Orientation & Font Quality:**
    - Fixed `vga_putchar` rendering (MSB-first) and replaced the font with a high-quality 8x8 VGA-optimized typeface.
    - Resolved all remaining mirrored or inverted text issues.
- **Application Launching:**
    - Fixed "apps not opening" by ensuring update functions are properly called via the new callback system.
    - Corrected drawing order to prevent window backgrounds from erasing app content.
- **Screen & Mouse:**
    - Fixed `vga_flip` in `kernel/vga.c` for correct vertical orientation.
    - Calibrated `mouse_update` for full-screen range and proper axis orientation.
- **Mouse Interaction:** Fixed event prioritization; Start Menu and Windows now capture events correctly before they reach the desktop.
- **UI Quality:**
    - Professional Windows 3.x aesthetic with 3D borders and refined icons.
- **File Operations:** Implemented basic "Open" and "Save" logic in Notepad.

## New Applications
- **Calculator:** Fully functional basic calculator (`apps/calc.c`).
- **File Manager:** Directory browser for the DOS file system (`apps/fman.c`).

## Build System
- Optimized `Makefile` for **ARM64 Ubuntu** using **Box64** and **DJGPP**.
- Implemented static library linking for better binary stability.
