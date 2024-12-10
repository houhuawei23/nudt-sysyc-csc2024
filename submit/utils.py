import os
import shutil
import subprocess

import colorama
from colorama import Fore, Style

import platform


def checkMachine(needed_machine: str):
    if platform.machine() != needed_machine:
        print(f"not correctly platform ({platform.machine()}), need run on {needed_machine}!")
        return False
    return True


def isZero(x):
    return abs(x) < 1e-5


def safeDivide(x, y):
    if isZero(y):
        return 0
    else:
        return x / y

def getRawName(filepath: str):
    return os.path.basename(os.path.splitext(filepath)[0])


def removePathSuffix(filename: str):
    """
    basename("a/b/c.txt") => "a/b/c"
    """
    return os.path.splitext(filename)[0]


def overwritten_or_create_dir(path):
    if os.path.exists(path):
        print(f"Warning: {path} found, will be overwritten")
        shutil.rmtree(path)
        os.mkdir(path)
    else:
        print(f"{path} not found, will be created")
        os.mkdir(path)


def check_args(
    compiler_path, tests_path, output_asm_path, output_exe_path, output_c_path
):
    if not os.path.exists(compiler_path):
        print(f"Compiler not found: {compiler_path}")
        print("Please run: `python compile.py ./ compiler` first")
        return False
    if not os.path.exists(tests_path):
        print(f"Tests path not found: {tests_path}")
        return False
    overwritten_or_create_dir(output_asm_path)
    overwritten_or_create_dir(output_exe_path)
    overwritten_or_create_dir(output_c_path)
    return True


def check_args_beta(compiler_path: str, tests_path: str, output_dir_path: str):
    if not os.path.exists(compiler_path):
        print(f"Compiler not found: {compiler_path}")
        print("Please run: `python compile.py ./ compiler` first")
        return False
    if not os.path.exists(tests_path):
        print(f"Tests path not found: {tests_path}")
        return False
    output_asm_path = os.path.join(output_dir_path, "asm")
    output_exe_path = os.path.join(output_dir_path, "exe")
    output_c_path = os.path.join(output_dir_path, "c")

    paths = [output_dir_path, output_asm_path, output_exe_path, output_c_path]

    for path in paths:
        os.makedirs(path, exist_ok=True)

    return True


def check_args_alpha(must_exist_paths: list, must_create_paths: list):
    for path in must_exist_paths:
        if not os.path.exists(path):
            print(f"Path not found: {path}")
            return False
    for path in must_create_paths:
        os.makedirs(path, exist_ok=True)
    return True


def compare_output_with_standard_file(
    standard_filename: str, output: str, returncode: int
):
    if len(output) != 0 and not output.endswith("\n"):
        output += "\n"
    output += str(returncode) + "\n"

    with open(standard_filename, encoding="utf-8", newline="\n") as f:
        standard_answer = f.read()
    if not standard_answer.endswith("\n"):
        standard_answer += "\n"

    standard_answer = standard_answer.replace("\r\n", "\n")

    if output != standard_answer:
        print(Fore.RED + " Output mismatch")
        print("--------")
        print("output:")
        print(repr(output[:100]))
        print("--------")
        print("stdans:")
        print(repr(standard_answer[:100]))
        print("--------")
        return False
    return True


def compare_and_parse_perf(src: str, out: subprocess.CompletedProcess):
    """
    compare and parse perf output
    """
    output_file = removePathSuffix(src) + ".out"
    if not compare_output_with_standard_file(output_file, out.stdout, out.returncode):
        raise RuntimeError("Output mismatch")

    for line in out.stderr.splitlines():
        if line.startswith("insns:"):
            used = int(line.removeprefix("insns:").strip())
            if "performance" in src:
                print(f" {used}", end="")
            return used

    for line in out.stderr.splitlines():
        if line.startswith("TOTAL:"):
            perf = line.removeprefix("TOTAL: ").split("-")
            used = (
                float(perf[0][:-1]) * 3600
                + float(perf[1][:-1]) * 60
                + float(perf[2][:-1])
                + float(perf[3][:-2]) * 1e-6
            )
            if "performance" in src:
                print(Fore.GREEN + f" {used:.6f}s")
            return max(1e-6, used)

    raise RuntimeError("No performance data")
