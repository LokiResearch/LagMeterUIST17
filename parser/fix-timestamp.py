#!/usr/bin/env python3
# -*- coding:utf-8 -*-
import argparse
import os.path
import re

# ---- Paramerers
parser = argparse.ArgumentParser(
    description='Print log file with fixed piezo timestamps. \
A latency is substracted from the piezo timestamp. \
Timestamps in log file are assumed to be in nanoseconds.')
parser.add_argument('input',
    help = 'Log file containing piezo timemstamps to fix')
parser.add_argument('latency',
    help='Time in µs to compensate')
args = parser.parse_args()

# ---- Consts
MICROS_TO_NANOS = 10**3

# ---- Regexes
REGEX_END_OF_HEADER = r"^# -- end config"
REGEX_ANY = r"^(?P<timestamp>\d+)(?P<content>\s.+)$"
REGEX_LEONARDO_PIEZO = r"^(?P<timestamp>\d+)(?P<content>\sleonardo(\s\d+){3}\s80.+)$"

# ---- Main
latency = int(args.latency)
entries = {} # key: timestamp, value: content
header = ""
with open(args.input) as logfile:
    # Skip header
    line = logfile.readline()
    lineno = 1
    while re.search(REGEX_END_OF_HEADER, line) is None:
        header += line
        line = logfile.readline()
        lineno += 1
    header += line + logfile.readline() # Skip blank line after header
    lineno += 1
    # Read log file
    for line in logfile.readlines():
        lineno += 1
        entry = re.search(REGEX_LEONARDO_PIEZO, line)
        if entry is None:
            entry = re.search(REGEX_ANY, line)
            if entry is None:
                print("Parse error @ {}. Process aborted.".format(lineno))
                exit(-1)
            else:
                entries[int(entry.group('timestamp'))] = entry.group('content')
        else:
            temp_timestamp = int(entry.group('timestamp')) - latency*MICROS_TO_NANOS
            entries[temp_timestamp] = entry.group('content')

# Rebuild log file
output_contents = header
timestamps = sorted(entries.keys())
last_timestamp = 0
for timestamp in timestamps:
    if timestamp < last_timestamp:
        print("Error: The resulting file is not sorted with timestamp. \
Process aborted. Please report this error to Mjolnir engineer team!")
        exit(-1)
    output_contents += str(timestamp) + entries[timestamp] + "\n"
    last_timestamp = timestamp

print(output_contents)
