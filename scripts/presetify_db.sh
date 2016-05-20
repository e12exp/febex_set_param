#!/bin/bash

if [[ "$#" -lt "1" ]]; then
  echo "Usage: $0 dbfile" >&2
  exit -1
fi

FNAME="$1"

../setpar $FNAME list 2>/dev/null | gawk 'BEGIN { print "add sfp" }
  match($0, /Firmware ([a-zA-Z0-9_\-\.]+)/, a) {
    printf "add module 0 1 %s\n", a[1]
  }
  match($0, /^([0-3])\.([0-9]{3})\.([0-9]{2})\.(\w+)[^:]+:\s*(\w+)/, a) {
    printf "set %d.%d.%d.%s %s\n", a[1], a[2], a[3], a[4], a[5]
  }
  match($0, /^([0-3])\.([0-9]{3})\.([a-zA-Z]\w+)[^:]+:\s*(\w+)/, a) {
    printf "set %d.%d.%s %s\n", a[1], a[2], a[3], a[4]
  }
  END { print "save\nexit" }'
