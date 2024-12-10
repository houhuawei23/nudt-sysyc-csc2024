# in {$WORKSPACE}/submit/
# python3 ./run.py ../compiler ../test ./test_output
import os
import sys
from datetime import datetime

from utils import check_args, check_args_beta

from Test import Test


compiler_path = sys.argv[1]
tests_path = sys.argv[2]  #
output_dir = sys.argv[3]

output_asm_path = os.path.join(output_dir, "asm")
output_exe_path = os.path.join(output_dir, "exe")
output_c_path = os.path.join(output_dir, "c")


if not check_args_beta(compiler_path, tests_path, output_dir):
    sys.exit(1)

sysy_runtime = os.path.join(tests_path, "sysy/sylib.c")
sysy_header = os.path.join(tests_path, "sysy/sylib.h")

sysy_link_for_riscv_gpp = os.path.join(tests_path, "link/link.c")


def compile_only():
    our_compiler_timeout = 100
    test = Test(
        compiler_path,
        tests_path,
        output_asm_path,
        output_exe_path,
        output_c_path,
        sysy_runtime,
        sysy_link_for_riscv_gpp,
    )

    test.set("riscv", "2024", our_compiler_timeout)
    test.runCompileOnly("functional")

    test.runCompileOnly("performance")

if __name__ == "__main__":
    compile_only()
