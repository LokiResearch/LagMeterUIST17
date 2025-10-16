#!/usr/bin/env python

# Author: Gery Casiez

import sys

if __name__ == "__main__":
    with open(sys.argv[1]) as f:
        data = f.readlines()
    data = data[8:]
    print data