# in submit_test dir
# python3 run_visionfive.py ../test/ ./.output
# link and run, collect perf data


import os
import sys
from datetime import datetime

from utils import check_args_alpha

from Test import Test

tests_path = sys.argv[1]  #
output_dir = sys.argv[2]

output_asm_path = os.path.join(output_dir, "asm")
output_exe_path = os.path.join(output_dir, "exe")
output_c_path = os.path.join(output_dir, "c")

if not check_args_alpha([tests_path], [output_dir]):
    sys.exit(1)

sysy_runtime = os.path.join(tests_path, "sysy/sylib.c")
sysy_header = os.path.join(tests_path, "sysy/sylib.h")

sysy_link_for_riscv_gpp = os.path.join(tests_path, "link/link.c")

test = Test(
    None,
    tests_path,
    output_asm_path,
    output_exe_path,
    output_c_path,
    sysy_runtime,
    sysy_link_for_riscv_gpp,
)
run_timeout = 300
test.set("riscv", "2024", run_timeout)
# test.runCompileOnly("performance")
test.runOnVisionFive("performance")
