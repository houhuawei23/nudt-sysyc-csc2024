# python test.py compiler_path tests_path output_asm_path output_exe_path output_c_path
# python ./submit/test.py ./compiler ./test/ ./.tmp/asm ./.tmp/exe ./.tmp/c


import os
import sys
from datetime import datetime

from utils import check_args_beta

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


def submitTest():
    submit_timeout = 10
    test = Test("riscv", "2023", submit_timeout)
    # test.run()
    test.runSingleCase("functional", "04_arr_defn3.sy")


def functionalTest():
    functional_timeout = 150
    test = Test(
        compiler_path,
        tests_path,
        output_asm_path,
        output_exe_path,
        output_c_path,
        sysy_runtime,
        sysy_link_for_riscv_gpp,
    )
    test.set("riscv", "2024", functional_timeout, 1, 0)
    test.runFunctionalTest("functional")

    # test.set("riscv", "2023", functional_timeout, 1, 0)
    # test.runFunctionalTest("functional")
    # test.runFunctionalTest("hidden_functional")

    # test.set("riscv", "2022", functional_timeout, 1, 0)
    # test.runFunctionalTest("functional")
    # test.runFunctionalTest("hidden_functional")

    # test.set("riscv", "2021", functional_timeout, 1, 0)
    # test.runFunctionalTest("functional")
    # test.runFunctionalTest("hidden_functional")
    # test.run("hidden_functional")
    # test.run_single_case("functional", "00_main.sy")
    # test.run_single_case("functional", "11_BST.sy")
    # test.runSingleCase("functional", "04_arr_defn3.sy")
    # test.runSingleCase("performance", "01_mm1.sy")



def perfTest():
    perf_timeout = 200
    test = Test(
        compiler_path,
        tests_path,
        output_asm_path,
        output_exe_path,
        output_c_path,
        sysy_runtime,
        sysy_link_for_riscv_gpp,
    )
    test.set("riscv", "2024", perf_timeout, 1, 0)
    test.runPerformanceTest("performance")
    # test.runSingleCase("performance", "01_mm1.sy")
    # large_loop_array_2.sy
    # test.runSingleCase("performance", "2024-2D0-22.sy")


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

    test.set("riscv", "2024_final", our_compiler_timeout, 1, 0)
    test.runCompileOnly("performance")

if __name__ == "__main__":
    # submitTest()
    # perfTest()
    functionalTest()
    # compile_only()
