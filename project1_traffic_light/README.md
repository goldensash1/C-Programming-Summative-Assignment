# Project 1 - Smart Traffic Light Controller

## What this project demonstrates
- 2-intersection traffic light simulation in Arduino/Tinkercad
- Vehicle detection using push buttons
- Dynamic timing adjustment from traffic counts
- Non-blocking operation using `millis()`
- Data management with structures, pointers, and dynamic memory
- Serial menu for status, reset, and manual override
- Error handling for invalid commands and states

## Tinkercad wiring
Intersection A:
- RED -> pin 2
- YELLOW -> pin 3
- GREEN -> pin 4
- Sensor button -> pin 8 (INPUT_PULLUP, button to GND)

Intersection B:
- RED -> pin 5
- YELLOW -> pin 6
- GREEN -> pin 7
- Sensor button -> pin 9 (INPUT_PULLUP, button to GND)

## Serial commands
- `help`
- `status`
- `auto`
- `manual A RED`
- `manual A YELLOW`
- `manual A GREEN`
- `manual B RED|YELLOW|GREEN`
- `reset`

## Notes for presentation
- Show non-blocking behavior by pressing buttons while lights keep cycling.
- Show traffic-adaptive timing by generating more vehicle presses.
- Demonstrate manual override and return to auto mode.
