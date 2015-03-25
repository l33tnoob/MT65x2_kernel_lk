#!/usr/bin/env python

import sys

tags = set()

def read_default(filename):
    f = open(filename)
    while True:
        line = f.readline()
        if len(line) == 0:
            break
        rec = line.split(",")
        if len(rec) == 5:
            if (rec[1] == "xlog-java") or (rec[1] == "xlog-native") or (rec[1] == "xlog-kernel"):
                tags.add(rec[3])
    f.close()

read_default(sys.argv[1])

tags = sorted(tags, key=str.lower)
for t in tags:
    d = "default"
    print "%s %s" % (t, d)
