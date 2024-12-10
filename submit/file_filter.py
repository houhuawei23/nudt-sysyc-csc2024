#!/usr/bin/env python3
import os
import sys

# remove all files except for yml, jinja2, py, hpp, and cpp files
cmmc_root = sys.argv[1]
for root, dirs, files in os.walk(cmmc_root):
    for file in files:
        if not (
            file.endswith(".py")
            or file.endswith(".hpp")
            or file.endswith(".h")
            or file.endswith(".cpp")
            or file.startswith("LICENSE")
            or file.startswith("Makefile")
            or file.startswith("CMakeLists.txt")
        ):
            os.remove(root + "/" + file)
