#!/bin/bash



echo "===> Ensuring directories exist"
mkdir -p /config

export SYS_NUM_CPU=$(grep processor /proc/cpuinfo | wc -l)
export SYS_TOTAL_MEM=$(grep MemTotal /proc/meminfo | awk '{print $2}')
export SYS_TOTAL_MEM=$((SYS_TOTAL_MEM / 1024)) # convert to MB

# run any setup & config generation scripts
/usr/bin/openbmpd-configure

if [[ $# -eq 0 ]]; then
  echo "ERROR: No CMD to execute. Exiting."
  exit 1
fi

echo "===> Delaying 30 seconds for other containers to startup"
sleep 30

exec "$@"
