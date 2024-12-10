from enum import Enum

import colorama
from colorama import Fore, Style

# Initializes colorama and autoresets color
colorama.init(autoreset=True)

from utils import isZero, safeDivide

from datetime import datetime

import os


class ResultType(Enum):
    PASSED = 0
    RUN_COMPILER_FAILED = 1
    LINK_EXECUTABLE_FAILED = 2
    RUN_EXECUTABLE_FAILED = 3
    OUTPUT_MISMATCH = 4


colorMap = {
    ResultType.PASSED: Fore.GREEN,
    ResultType.RUN_COMPILER_FAILED: Fore.RED,
    ResultType.LINK_EXECUTABLE_FAILED: Fore.RED,
    ResultType.RUN_EXECUTABLE_FAILED: Fore.RED,
    ResultType.OUTPUT_MISMATCH: Fore.RED,
}


class TestResult:
    # type -> list of (src, completed_process)
    cases_result = {key: [] for key in ResultType}
    # src -> (compiler_time, gcc_o3_time)
    qemu_run_time = {}

    board_run_time = dict()

    def __init__(self, test_name):
        self.test_name = test_name

    def print_result_overview(self):
        print(Fore.YELLOW + f"Test {self.test_name}:")
        all = self.all_cases()
        passed = self.cases_result[ResultType.PASSED]

        print(
            f"Total: {len(all)}, Passed: {len(passed)}, Failed: {len(all)-len(passed)}"
        )
        for type in ResultType:
            print(colorMap[type] + f"{type.name}: {len(self.cases_result[type])}")
        print()
        # for type in ResultType:
        #     if type == ResultType.PASSED:
        #         continue
        #     if len(self.cases_result[type]) == 0:
        #         continue
        #     self.print_result(type)

    def all_cases(self):
        all = []
        for key in ResultType:
            all.extend(self.cases_result[key])
        return all

    def print_result(self, type: ResultType):
        print(f"Test {self.test_name}" + colorMap[type] + f" {type.name}:")
        for src, process in self.cases_result[type]:
            print(Fore.YELLOW + f"test: {src}")
            print(f"returncode: {process.returncode}")
            print("stdout:")
            print(repr(process.stdout[:100]))
            print("stderr:")
            print(repr(process.stderr[:100]))

    def save_result(self, filename: str):
        with open(filename, "w", encoding="utf-8") as f:
            f.write(datetime.now().strftime("%Y_%m_%d_%H:%M"))
            f.write(f"\n\n")
            f.write(f"Test {self.test_name}")
            for type in ResultType:
                if type == ResultType.PASSED:
                    continue
                f.write(f"\n{type.name}:\n")
                for src, process in self.cases_result[type]:
                    f.write(f"test: {src}\n")
                    f.write(f"returncode: {process.returncode}\n")
                    f.write("stdout:\n")
                    f.write(repr(process.stdout[:100]))
                    f.write("\n")
                    f.write("stderr:\n")
                    f.write(repr(process.stderr[:100]))
                    f.write("\n")

    def cal_average_score(self):
        speedups = []
        for my, gcc in self.qemu_run_time.values():
            speedup = safeDivide(gcc, my)
            speedups.append(speedup)
        average_score = (sum(speedups) / len(speedups)) * 100
        return average_score

    def print_perf_overview(self):
        print(Fore.YELLOW + f"Test {self.test_name}:")
        print(f"QEMU run time:")

        for src, (compiler_time, gcc_o3_time) in self.qemu_run_time.items():
            print(Fore.YELLOW + f"test: {src}")
            print(f"compiler time: {compiler_time:.2f}s")
            print(f"gcc -O3 time: {gcc_o3_time:.2f}s")
            speedup = safeDivide(gcc_o3_time, compiler_time)
            # if not isZero(compiler_time):
            #     speedup = gcc_o3_time / compiler_time
            # else:
            #     speedup = 0
            print(f"speedup: {speedup:.2f}x")
            print()
        # average score = sum(speedup) * 100 / len(speedup)
        average_score = self.cal_average_score()
        print(Fore.RED + f"averge score: {average_score:.2f}")

    def save_perf_result(self, filename: str):
        with open(filename, "w", encoding="utf-8") as f:
            f.write(datetime.now().strftime("%Y_%m_%d_%H:%M"))
            f.write(f"\n\n")
            f.write(f"Test {self.test_name}\n")
            f.write(f"QEMU run time compare:\n\n")
            average_score = self.cal_average_score()
            f.write(f"averge score: {average_score:.2f}\n\n")

            for type in ResultType:
                if type == ResultType.PASSED:
                    continue
                f.write(f"\n{type.name}:\n")
                for src, process in self.cases_result[type]:
                    f.write(f"test: {src}\n")
                    f.write(f"returncode: {process.returncode}\n")
                    f.write("stdout:\n")
                    f.write(repr(process.stdout[:100]))
                    f.write("\n")
                    f.write("stderr:\n")
                    f.write(repr(process.stderr[:100]))
                    f.write("\n")

            f.write("\n")

            for src, (compiler_time, gcc_o3_time) in self.qemu_run_time.items():
                f.write(f"test: {src}\n")
                f.write(f"compiler time: {compiler_time:.2f}s\n")
                f.write(f"gcc -O3 time: {gcc_o3_time:.2f}s\n")
                if not isZero(compiler_time):
                    speedup = gcc_o3_time / compiler_time
                else:
                    speedup = 0
                f.write(f"speedup: {speedup:.2f}x\n")
                f.write("\n")

    def print_board_overview(self):
        print(Fore.RED + f"perf on board")
        # for filename, time in self.board_run_time.items():
        #     print(f"{filename}: {time:.4f}s")
        print("src,             compiler_time,       gcc_o3_time,         speedup")
        for src, (compiler_time, gcc_o3_time) in self.qemu_run_time.items():
            speedup = safeDivide(gcc_o3_time, compiler_time)
            print(f"{src: <20} {compiler_time: 6f}s {gcc_o3_time: 6f}s, {speedup:.2f}x")

    def save_board_result(self, filename: str):
        ds = datetime.now().strftime("%Y_%m_%d_%H:%M")
        with open(os.path.join("./record", f"{ds}.csv"), "w") as f:
            f.write("src,compiler_time,gcc_o3_time,speedup\n")
            for src, (compiler_time, gcc_o3_time) in self.qemu_run_time.items():
                speedup = safeDivide(gcc_o3_time, compiler_time)
                line = f"{src},{compiler_time: 6f},{gcc_o3_time: 6f}, {speedup:.3f}\n"
                f.write(line)
            # for src, time_used in self.board_run_time.items():
            #     line = f"{src},{time_used: 6f},\n"
            #     f.write(line)
                # print(f"{filename}: {time_used:.4f}s")

        with open(os.path.join("./record", filename), "w") as f:
            # for src, time_used in self.board_run_time.items():
            #     markdown_line = f"| {src} | {time_used:.4f}s |\n"
            #     f.write(markdown_line)
            for src, (compiler_time, gcc_o3_time) in self.qemu_run_time.items():
                markdown_line = f"| {src} | {compiler_time:.4f}s | {gcc_o3_time:.4f}s |\n"
                f.write(markdown_line)
                
