#!/usr/bin/env python3

import os
import sys


def is_owned_header(line: str):
    if line.find("antlr4-runtime.h") != -1:
        return False
    if line.startswith('#include "'):
        return True
    return False


submit_root = sys.argv[1]
for root, dirs, files in os.walk(submit_root):
    for file in files:
        lines = []
        with open(root + "/" + file) as f:
            for line in f.readlines():
                stripped = line.strip()
                if is_owned_header(stripped):
                    line = stripped
                    header = line[10 : line[10:].find('"') + 10]
                    # print(header)
                    newline = (
                        '#include "'
                        + os.path.relpath(os.path.abspath(submit_root), os.path.abspath(root))
                        + "/include/"
                        + header
                        + '"\n'
                    )
                    lines.append(newline)
                else:
                    lines.append(line)
        with open(root + "/" + file, "w") as f:
            # if file.endswith(".cpp"):
            #     f.write("#define NDEBUG\n")
            f.writelines(lines)
