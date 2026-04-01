# EasyEDA PCB Checklist (Project 1)

Use this checklist while building your schematic and PCB:

- Place microcontroller header (Arduino-compatible)
- Add 6 LEDs (A/B: red, yellow, green)
- Add current-limiting resistors (220 to 330 ohm) for each LED
- Add 2 push buttons for vehicle sensors
- Add pull-up strategy (internal pull-up in code, or external resistors)
- Add power connectors (5V and GND)
- Label nets clearly (A_RED, A_YELLOW, A_GREEN, B_RED, ...)
- Run ERC in schematic and fix warnings
- Convert to PCB and place components cleanly
- Route traces with clear separation and low crossing
- Add board outline, mounting holes, and silkscreen labels
- Run DRC and fix all critical issues

Recommended demo screenshots for report/video:
- Schematic overview
- PCB top layer
- 3D view (if available)
