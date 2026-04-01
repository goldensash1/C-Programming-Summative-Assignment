#!/usr/bin/env bash

set -u

LOG_FILE="server_monitor.log"
PID_FILE="monitor.pid"
INTERVAL=60
CPU_THRESHOLD=80
MEM_THRESHOLD=80
DISK_THRESHOLD=85

require_cmd() {
  if ! command -v "$1" >/dev/null 2>&1; then
    echo "[ERROR] Missing command: $1"
    return 1
  fi
  return 0
}

check_environment() {
  local required=(top free df ps awk grep sed date)
  local ok=0
  local cmd
  for cmd in "${required[@]}"; do
    if ! require_cmd "$cmd"; then
      ok=1
    fi
  done
  if [[ $ok -ne 0 ]]; then
    echo "[ERROR] One or more required commands are missing."
    exit 1
  fi
}

log_message() {
  local msg="$1"
  echo "[$(date '+%Y-%m-%d %H:%M:%S')] $msg" >> "$LOG_FILE"
}

get_cpu_usage() {
  local idle
  idle=$(top -l 1 | awk '/CPU usage/ {gsub("%", "", $7); print $7}')
  if [[ -z "$idle" ]]; then
    echo "0"
    return
  fi
  awk -v idle="$idle" 'BEGIN { printf "%.0f", 100 - idle }'
}

get_mem_usage() {
  local used total
  used=$(free -m | awk '/Mem:/ {print $3}')
  total=$(free -m | awk '/Mem:/ {print $2}')
  if [[ -z "$used" || -z "$total" || "$total" -eq 0 ]]; then
    echo "0"
    return
  fi
  awk -v u="$used" -v t="$total" 'BEGIN { printf "%.0f", (u/t)*100 }'
}

get_disk_usage() {
  df -h / | awk 'NR==2 {gsub("%", "", $5); print $5}'
}

get_active_processes() {
  ps -e --no-headers | wc -l | awk '{print $1}'
}

show_health() {
  local cpu mem disk procs
  cpu=$(get_cpu_usage)
  mem=$(get_mem_usage)
  disk=$(get_disk_usage)
  procs=$(get_active_processes)

  echo "======================================"
  echo " Server Health Dashboard"
  echo "======================================"
  echo "CPU Usage      : ${cpu}%"
  echo "Memory Usage   : ${mem}%"
  echo "Disk Usage     : ${disk}%"
  echo "Active Process : ${procs}"
  echo "Thresholds     : CPU ${CPU_THRESHOLD}% | MEM ${MEM_THRESHOLD}% | DISK ${DISK_THRESHOLD}%"
  echo "Interval       : ${INTERVAL}s"
  echo "Log File       : $LOG_FILE"
  echo "======================================"

  log_message "HEALTH cpu=${cpu}% mem=${mem}% disk=${disk}% procs=${procs}"
}

check_alerts() {
  local cpu mem disk
  cpu=$(get_cpu_usage)
  mem=$(get_mem_usage)
  disk=$(get_disk_usage)

  if (( cpu > CPU_THRESHOLD )); then
    echo "[ALERT] High CPU usage: ${cpu}%"
    log_message "ALERT High CPU usage: ${cpu}%"
  fi

  if (( mem > MEM_THRESHOLD )); then
    echo "[ALERT] High memory usage: ${mem}%"
    log_message "ALERT High memory usage: ${mem}%"
  fi

  if (( disk > DISK_THRESHOLD )); then
    echo "[ALERT] High disk usage: ${disk}%"
    log_message "ALERT High disk usage: ${disk}%"
  fi
}

valid_percent() {
  [[ "$1" =~ ^[0-9]+$ ]] && (( $1 >= 1 && $1 <= 100 ))
}

valid_interval() {
  [[ "$1" =~ ^[0-9]+$ ]] && (( $1 >= 1 ))
}

configure_thresholds() {
  local cpu mem disk interval

  echo "Current CPU threshold: $CPU_THRESHOLD"
  read -r -p "Enter new CPU threshold (1-100): " cpu
  if valid_percent "$cpu"; then
    CPU_THRESHOLD=$cpu
  else
    echo "[ERROR] Invalid CPU threshold"
  fi

  echo "Current memory threshold: $MEM_THRESHOLD"
  read -r -p "Enter new memory threshold (1-100): " mem
  if valid_percent "$mem"; then
    MEM_THRESHOLD=$mem
  else
    echo "[ERROR] Invalid memory threshold"
  fi

  echo "Current disk threshold: $DISK_THRESHOLD"
  read -r -p "Enter new disk threshold (1-100): " disk
  if valid_percent "$disk"; then
    DISK_THRESHOLD=$disk
  else
    echo "[ERROR] Invalid disk threshold"
  fi

  echo "Current interval: $INTERVAL seconds"
  read -r -p "Enter monitoring interval (seconds, >=1): " interval
  if valid_interval "$interval"; then
    INTERVAL=$interval
  else
    echo "[ERROR] Invalid interval"
  fi

  log_message "CONFIG cpu=${CPU_THRESHOLD} mem=${MEM_THRESHOLD} disk=${DISK_THRESHOLD} interval=${INTERVAL}"
}

monitor_loop() {
  while true; do
    show_health
    check_alerts
    sleep "$INTERVAL"
  done
}

start_monitoring() {
  if [[ -f "$PID_FILE" ]]; then
    local existing_pid
    existing_pid=$(cat "$PID_FILE")
    if kill -0 "$existing_pid" 2>/dev/null; then
      echo "[INFO] Monitoring already running (PID: $existing_pid)"
      return
    fi
    rm -f "$PID_FILE"
  fi

  monitor_loop &
  local pid=$!
  echo "$pid" > "$PID_FILE"
  echo "[OK] Monitoring started in background (PID: $pid)"
  log_message "MONITOR START pid=${pid}"
}

stop_monitoring() {
  if [[ ! -f "$PID_FILE" ]]; then
    echo "[INFO] Monitoring is not running"
    return
  fi

  local pid
  pid=$(cat "$PID_FILE")
  if kill -0 "$pid" 2>/dev/null; then
    kill "$pid"
    echo "[OK] Monitoring stopped (PID: $pid)"
    log_message "MONITOR STOP pid=${pid}"
  else
    echo "[INFO] Monitor process not active"
  fi

  rm -f "$PID_FILE"
}

view_logs() {
  if [[ ! -f "$LOG_FILE" ]]; then
    echo "[INFO] No logs yet"
    return
  fi
  tail -n 50 "$LOG_FILE"
}

clear_logs() {
  : > "$LOG_FILE"
  echo "[OK] Logs cleared"
}

menu() {
  while true; do
    echo
    echo "========== Server Monitor Menu =========="
    echo "1. Display current system health"
    echo "2. Configure monitoring thresholds"
    echo "3. View activity logs"
    echo "4. Clear logs"
    echo "5. Start monitoring"
    echo "6. Stop monitoring"
    echo "7. Exit"
    echo "========================================="

    read -r -p "Choose an option (1-7): " choice

    case "$choice" in
      1) show_health; check_alerts ;;
      2) configure_thresholds ;;
      3) view_logs ;;
      4) clear_logs ;;
      5) start_monitoring ;;
      6) stop_monitoring ;;
      7)
        stop_monitoring
        echo "Exiting..."
        break
        ;;
      *) echo "[ERROR] Invalid option. Please enter 1-7." ;;
    esac
  done
}

main() {
  check_environment
  touch "$LOG_FILE" 2>/dev/null || {
    echo "[ERROR] Cannot write to log file: $LOG_FILE"
    exit 1
  }
  menu
}

main "$@"
