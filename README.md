# C Programming Summative - Complete Solutions

This workspace contains simple, readable implementations for all 5 required projects.

## Folder structure
- `project1_traffic_light/` - Arduino smart traffic controller + EasyEDA checklist
- `project2_server_monitor/` - Linux server health monitoring Bash script
- `project3_academic_records/` - C academic records analyzer
- `project4_data_toolkit/` - C data analysis toolkit using function pointers and callbacks
- `project5_web_scraper/` - C pthread multi-threaded web scraper

## Quick run commands

### Project 1 (Arduino/Tinkercad)
- Open `project1_traffic_light/traffic_controller.ino` in Arduino IDE or paste into Tinkercad code editor.
- Use README in that folder for exact pin wiring and serial commands.

### Project 2 (Linux script)
```bash
cd project2_server_monitor
chmod +x server_monitor.sh
./server_monitor.sh
```

### Project 3
```bash
cd project3_academic_records
gcc -Wall -Wextra -std=c11 academic_records.c -o academic_records
./academic_records
```

### Project 4
```bash
cd project4_data_toolkit
gcc -Wall -Wextra -std=c11 data_toolkit.c -o data_toolkit
./data_toolkit
```

### Project 5
```bash
cd project5_web_scraper
gcc -Wall -Wextra -std=c11 web_scraper.c -o web_scraper -lpthread
./web_scraper
```

## Rubric alignment checklist

### Traffic Light Optimization
- [x] Two intersections simulated
- [x] Dynamic timing based on vehicle count
- [x] Structures, pointers, dynamic memory
- [x] Non-blocking control via `millis()`
- [x] Serial status logging + manual override
- [x] Error handling for invalid states/commands
- [x] PCB design checklist provided

### Linux Monitoring Script
- [x] CPU/memory/disk/process monitoring
- [x] Configurable thresholds
- [x] Alerts and timestamped logs
- [x] Menu for monitor controls and logs
- [x] Continuous monitoring loop with interval
- [x] Input validation and command checks

### Academic Records Analyzer
- [x] Dynamic memory with `malloc/realloc/free`
- [x] Record structure and CRUD operations
- [x] Manual search and sorting algorithms
- [x] Reports: average/high/low/median/top N/course summaries
- [x] File save/load with validation and duplicate ID prevention

### Data Toolkit (Function Pointers)
- [x] Function pointer dispatcher (menu mapping)
- [x] Callback-based filter/transform/comparison
- [x] Dataset create/display/sum/min/max/search/sort
- [x] Dynamic memory and file operations
- [x] Validation + NULL callback handling

### Web Scraper
- [x] POSIX threads for parallel URL fetching
- [x] One output file per thread
- [x] Error handling for failures/unreachable URLs

## 8-minute presentation plan
1. Problem statement and requirements (45 sec)
2. Project 1 demo in Tinkercad + serial menu (2 min)
3. Project 2 script menu, alerts, logs (1.5 min)
4. Project 3 CRUD + reports + save/load (1.5 min)
5. Project 4 function pointers + callback demo (1 min)
6. Project 5 parallel downloader demo (1 min)
7. Wrap-up: mapping to rubric and learning outcomes (30 sec)

## Important submission reminders
- Keep your GitHub repository public until grades are published.
- Record one clear demo video (max 8 minutes).
- Show both code and execution output for each project.
# C-Programming-Summative-Assignment
