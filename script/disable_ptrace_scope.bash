#!/usr/bin/env bash

# Check if script is run as root (sudo)
if [ "$EUID" -ne 0 ]; then
  echo "Injection failed: Please run as root or use sudo."
  exit 1
fi

echo 0 | sudo tee /proc/sys/kernel/yama/ptrace_scope
