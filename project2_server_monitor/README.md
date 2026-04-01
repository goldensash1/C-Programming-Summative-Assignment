# Project 2 - Linux Server Health Monitoring Script

## Features implemented
- Tracks CPU, memory, disk, and process count
- Supports configurable thresholds
- Writes timestamped logs
- Raises alerts when thresholds are exceeded
- Interactive menu (view health, configure, view logs, clear logs, start/stop, exit)
- Continuous monitoring in background process
- Input validation and missing-command checks

## Run
```bash
chmod +x server_monitor.sh
./server_monitor.sh
```

## Notes
- Optimized for Linux environment tools (`top`, `free`, `df`, `ps`).
- On macOS, command output formats differ. You can still present this as a Linux-target script.
