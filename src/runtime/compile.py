#!/usr/bin/env python3

import sys
import subprocess
import os

target = sys.argv[1]
# infile = sys.argv[2]
outfile = sys.argv[2]
# src = os.path.dirname(os.path.abspath(__file__)) + "/LoopParallel.cpp"
runtime_dir = os.path.dirname(os.path.abspath(__file__))

memset_cpp = os.path.join(runtime_dir, "memset.cpp")
lookup_cpp = os.path.join(runtime_dir, "Lookup.cpp")

# parallelFor_dir = os.path.join(runtime_dir, "./MultiThreads")
# parallelFor_cpp = os.path.join(parallelFor_dir, "MultiThreads.cpp")

parallelFor_dir = os.path.join(runtime_dir, "./LoopParallel")
parallelFor_cpp = os.path.join(parallelFor_dir, "LoopParallel.cpp")
infiles = [memset_cpp, lookup_cpp, parallelFor_cpp]

gcc_ref_command = {
    "RISCV": "riscv64-linux-gnu-g++-12 -Ofast -DNDEBUG -march=rv64gc_zba_zbb -fno-stack-protector -fomit-frame-pointer -mcpu=sifive-u74 -mabi=lp64d -mcmodel=medlow -ffp-contract=on -w ".split(),
    "ARM": "arm-linux-gnueabihf-g++-12 -Ofast -DNDEBUG -march=armv7 -fno-stack-protector -fomit-frame-pointer -mcpu=cortex-a72 -mfpu=vfpv4 -ffp-contract=on -w -no-pie ".split(),
}[target]

# gcc_ref_command = {
#     "RISCV": "riscv64-linux-gnu-g++-12 -O2 -DNDEBUG -march=rv64gc -fno-stack-protector -fomit-frame-pointer -mabi=lp64d -mcmodel=medlow -ffp-contract=on -w ".split(),
#     "ARM": "arm-linux-gnueabihf-g++-12 -O3 -DNDEBUG -march=armv7 -fno-stack-protector -fomit-frame-pointer -mcpu=cortex-a72 -mfpu=vfpv4 -ffp-contract=on -w -no-pie ".split(),
# }[target]


def fixHeader(f) -> str:
    lines = []
    for line in f.readlines():
        if line.startswith('#include "'):
            header = line[10 : line[10:].find('"') + 10]
            print(header)
            print(runtime_dir)
            print(parallelFor_dir)
            newHeader = os.path.relpath(parallelFor_dir, runtime_dir) + "/" + header
            newline = '#include "' + newHeader + '"\n'
            lines.append(newline)
        else:
            lines.append(line)
    return "".join(lines)


merge = ""
for infile in infiles:
    if not os.path.exists(infile):
        print(f"Error: {infile} does not exist")
        sys.exit(1)
    with open(infile, "r") as f:
        merge += "// " + infile + "\n"
        # merge += f.read()
        merge += fixHeader(f)

mergefile = os.path.join(runtime_dir, ".merge.cpp")
with open(mergefile, "w") as f:
    f.write(merge)

runtime = subprocess.check_output(
    gcc_ref_command + [mergefile, "-S", "-o", "/dev/stdout"]
).decode("utf-8")

command = gcc_ref_command + [mergefile, "-S", "-o", "/dev/stdout"]

with open(outfile, "w") as f:
    f.write("// Automatically generated file, do not edit!\n")
    f.write("// Command: " + " ".join(command) + "\n")
    f.write('R"(')
    f.write(runtime)
    f.write(')"')
