#!/bin/bash
# Use the Script: ./test.sh -t test/2021/functional/001_var_defn.sy -p mem2reg -p dce
set -u
# set -x # print each command before executing it
# set -e # exit on error

qemu_riscv64="qemu-riscv64"
llvm_lli="llvm-lli-14"
riscv64_gpp="riscv64-linux-gnu-g++"
riscv64_gcc="riscv64-linux-gnu-gcc"
compiler_path="./compiler"
function riscv64_gcc_compile() {
    $riscv64_gcc -march=rv64gc -mabi=lp64d -mcmodel=medlow -ffp-contract=on -w $@
}

function riscv64_gpp_compile() {
    $riscv64_gpp -march=rv64gc -mabi=lp64d -mcmodel=medlow -ffp-contract=on -w $@
}

function qemu_riscv64_run() {
    $qemu_riscv64 -L "/usr/riscv64-linux-gnu/" $@
}

TIMEOUT=100
# 25 for ./test/2023/functional/65_color.sy
PASS_CNT=0
WRONG_CNT=0
ALL_CNT=0
TIMEOUT_CNT=0
SKIP_CNT=0

WRONG_FILES=()
TIMEOUT_FILES=()
PASSES=()

OPT_LEVEL="-O0"
LOG_LEVEL="-L0"

# Color setting
RED=$(tput setaf 1)
GREEN=$(tput setaf 2)
YELLOW=$(tput setaf 3)
RESET=$(tput sgr0)
CYAN=$(tput setaf 6)

# Default values
test_path="test/local_test"
output_dir="test/.out"
single_file=""
result_file="test/.out/result.txt"

error_code=0 # 0 for exe success

EC_MAIN=1
EC_RISCV_GCC=2
EC_LLI=3
EC_TIMEOUT=124

usage() {
    echo "Usage: $0 [-t <test_path>] [-o <output_dir>] [-r <result_file>] [-r <result_file>][-h]"
    echo "Options:"
    echo "  -t <test_path>  Specify the directory containing test files or single file (default: test/local_test)"
    echo "  -o <output_dir>     Specify the output directory (default: test/.out/)"
    echo "  -r <result_file>    Specify the file to store the test results (default: test/result.txt)"
    echo "  -h                  Print this help message"
}

SHORT="h,t:,o:,r:,p:,O:,L:"
LONG="help,test_path:,output_dir:,result_file:,pass:opt_level:,log_level:"
OPTS=$(getopt --options $SHORT --longoptions $LONG --name "$0" -- "$@")
if [ $? -ne 0 ]; then
    echo "Error parsing command line arguments" >&2
    usage
    exit 1
fi
eval set -- "$OPTS"

while true; do
    case "$1" in
    -h | --help)
        usage
        exit 0
        ;;
    -t | --test_path)
        test_path="$2"
        shift 2
        ;;
    -o | --output_dir)
        output_dir="$2"
        shift 2
        ;;
    -r | --result_file)
        result_file="$2"
        shift 2
        ;;
    -p | --pass)
        PASSES+=("$2")
        shift 2
        ;;
    -O | --opt_level)
        OPT_LEVEL="-O$2"
        shift 2
        ;;
    -L | --log_level)
        LOG_LEVEL="-L$2"
        shift 2
        ;;
    --)
        shift
        break
        ;;
    *)
        echo "Invalid option: $1" >&2
        usage
        exit 1
        ;;
    esac
done

# Handle remaining arguments (non-option arguments)
# Use $@ or $1, $2, etc. depending on the specific needs

PASSES_STR=$(
    IFS=" "
    echo "${PASSES[*]}"
)

# Ensure output directory exists
if [ ! -d "$output_dir" ]; then
    mkdir -p "$output_dir"
fi

echo "test_path      ${test_path} " >>${result_file}
echo "output_dir     ${output_dir} " >>${result_file}
echo "result_file    ${result_file} " >>${result_file}

echo "" >>${result_file}

sy_h="./test/link/sy.h"
sy_c="./test/link/link.c"
memset_s="./test/link/memset.s"

function run_gcc_test() {
    local single_file="$1"
    local output_dir="$2"
    local result_file="$3"

    local in_file="${single_file%.*}.in"

    local gcc_c="${output_dir}/gcc_test.c"
    local gcc_s="${output_dir}/gcc.s"
    local gcc_o="${output_dir}/gcc.o"

    gcc_out="${output_dir}/gcc_out"

    cat "${sy_c}" >"${gcc_c}"
    echo "" >>"${gcc_c}"
    cat "${single_file}" >>"${gcc_c}"

    riscv64_gpp_compile -S "${gcc_c}" -o "${gcc_s}" -O3
    riscv64_gpp_compile "${gcc_c}" -o "${gcc_o}" -O3
    if [ -f $in_file ]; then
        qemu_riscv64_run $gcc_o <$in_file >$gcc_out
    else
        qemu_riscv64_run $gcc_o >$gcc_out
    fi
    local qemu_res=$?
    return $qemu_res
}

function run_compiler_test() {
    local single_file="$1"
    local output_dir="$2"
    local result_file="$3"

    local in_file="${single_file%.*}.in"

    local gen_c="${output_dir}/gen_test.c"

    local gen_s="${output_dir}/gen.s"

    local gen_ll="${output_dir}/gen.ll"

    local gen_llinked="${output_dir}/gen_linked.ll"

    local gen_o="${output_dir}/gen.o"

    gen_out="${output_dir}/gen_out"

    if [ -f "${gen_c}" ]; then
        rm "${gen_c}"
    fi
    touch "${gen_c}"
    cat "${single_file}" >"${gen_c}"

    # emit llvm ir
    timeout $TIMEOUT $compiler_path -f "${single_file}" -i -t ${PASSES_STR} -o "${gen_ll}" "${OPT_LEVEL}" "${LOG_LEVEL}"
    if [ $? != 0 ]; then
        echo "${RED}[TIMEOUT]${RESET}: $compiler_path -f ${single_file} -i -t ${PASSES_STR} -o ${gen_ll} ${OPT_LEVEL} ${LOG_LEVEL}"
        return $EC_MAIN
    fi
    # emit assembly code
    timeout $TIMEOUT $compiler_path -f "${single_file}" -S -t ${PASSES_STR} -o "${gen_s}" "${OPT_LEVEL}" "${LOG_LEVEL}"
    mainres=$?
    if [ $mainres == $EC_TIMEOUT ]; then
        echo "${RED}[TIMEOUT]${RESET}: $compiler_path -f ${single_file} -S -t ${PASSES_STR} -o ${gen_s} ${OPT_LEVEL} ${LOG_LEVEL}"
        return $EC_TIMEOUT
    fi
    if [ $mainres != 0 ]; then
        return $EC_MAIN
    fi

    # for test
    riscv64_gcc_compile ${gen_s} ${sy_c} -o ${gen_o}
    if [ $? != 0 ]; then
        return $EC_RISCV_GCC
    fi

    if [ -f $in_file ]; then
        timeout $TIMEOUT $qemu_riscv64 -L "/usr/riscv64-linux-gnu/" "${gen_o}" <"${in_file}" >"${gen_out}"
        if [ $? == $EC_TIMEOUT ]; then # time out
            echo "${RED}[TIMEOUT]${RESET}: qemu-riscv64 -L /usr/riscv64-linux-gnu/ ${gen_o} <${in_file} >${gen_out}"
            return $EC_TIMEOUT
        fi
        $qemu_riscv64 -L "/usr/riscv64-linux-gnu/" "${gen_o}" <"${in_file}" >"${gen_out}"
    else
        timeout $TIMEOUT qemu-riscv64 -L "/usr/riscv64-linux-gnu/" "${gen_o}" >"${gen_out}"
        if [ $? == $EC_TIMEOUT ]; then # time out
            echo "${RED}[TIMEOUT]${RESET}: qemu-riscv64 -L /usr/riscv64-linux-gnu/ ${gen_o} >${gen_out}"
            return $EC_TIMEOUT
        fi
        $qemu_riscv64 -L "/usr/riscv64-linux-gnu/" "${gen_o}" >"${gen_out}"
    fi

    # qemu-riscv64 -L "/usr/riscv64-linux-gnu/" "${gen_o}" >"${gen_out}"
    local compiler_res=$?

    return $compiler_res

}

function run_test_asm() {
    local single_file="$1"
    local output_dir="$2"
    local result_file="$3"

    if [ -f "$single_file" ]; then
        echo "${YELLOW}[Testing]${RESET} $single_file"
        echo "${YELLOW}Our compiler output:${RESET}"
        run_compiler_test "${single_file}" "${output_dir}" "${result_file}"
        local res=$?

        echo "${YELLOW}GCC output:${RESET}"
        run_gcc_test "${single_file}" "${output_dir}" "${result_file}"
        local gccres=$?

        diff "${gen_out}" "${gcc_out}" >"${output_dir}/diff.out"
        local diff_res=$?
        # diff res or diff stdout
        echo "[RESULT] res (${RED}${res}${RESET}), gccres (${RED}${gccres}${RESET})"

        if [ ${res} != ${gccres} ] || [ ${diff_res} != 0 ]; then

            if [ ${res} == ${EC_MAIN} ]; then
                echo "${RED}[MAIN ERROR]${RESET} ${single_file}"
            elif [ ${res} == ${EC_RISCV_GCC} ]; then
                echo "${RED}[RISCV-GCC ERROR]${RESET} ${single_file}"
            elif [ ${res} == ${EC_LLI} ]; then
                echo "${RED}[LLI ERROR]${RESET} ${single_file}"
            elif [ ${res} == ${EC_TIMEOUT} ]; then
                echo "${RED}[TIMEOUT]${RESET} ${single_file}"
                echo "[TIMEOUT] ${single_file}" >>${result_file}
            else
                echo "${RED}[WRONG RES]${RESET} ${single_file}"
                echo "[WRONG RES] ${single_file}" >>${result_file}
                echo "  [WRONG RES]: res (${res}), gccres (${gccres})" >>${result_file}
            fi

            if [ ${res} == ${EC_TIMEOUT} ]; then
                TIMEOUT_CNT=$((TIMEOUT_CNT + 1))
                TIMEOUT_FILES+=(${single_file})
            else
                WRONG_CNT=$((WRONG_CNT + 1))
                WRONG_FILES+=(${single_file})
            fi
        else
            echo "${GREEN}[CORRECT]${RESET} ${single_file}"
            echo "[CORRECT] ${single_file}" >>${result_file}
            PASS_CNT=$((PASS_CNT + 1))
        fi
    else
        echo "File not found: $single_file"
        exit 1
    fi
}

# if not a file ot directory, exit
if [[ ! -f "$test_path" && ! -d "$test_path" ]]; then
    echo "Invalid test_path: $test_path"
    exit 1
fi

# if test_path is a file
if [ -f "$test_path" ]; then
    run_test_asm "$test_path" "$output_dir" "$result_file"
    echo "${GREEN}OPT PASSES${RESET}: ${PASSES_STR}"
fi

# if test_path is a directory
file_types=("*.c" "*.sy")

if [ -d "$test_path" ]; then
    for file_type in "${file_types[@]}"; do
        for file in "${test_path}"/${file_type}; do
            if [ ! -f "${file}" ]; then
                break
            else
                run_test_asm "${file}" "${output_dir}" "${result_file}"
            fi
        done

    done

    echo "====  RESULT  ===="

    echo "${RED}[WRONG]${RESET} files:"
    for file in "${WRONG_FILES[@]}"; do
        echo "${file}"
    done
    echo "${RED}[TIMEOUT]${RESET} files:"
    for file in "${TIMEOUT_FILES[@]}"; do
        echo "${file}"
    done

    echo "====   INFO   ===="
    echo "PASSES: ${PASSES_STR}"

    ALL_CNT=$((PASS_CNT + WRONG_CNT + SKIP_CNT + TIMEOUT_CNT))
    echo "${GREEN}PASS ${RESET}: ${PASS_CNT}"
    echo "${RED}WRONG${RESET}: ${WRONG_CNT}"
    echo "${RED}TIMEOUT${RESET}: ${TIMEOUT_CNT}"
    echo "${CYAN}SKIP ${RESET}: ${SKIP_CNT}"
    echo "${YELLOW}ALL  ${RESET}: ${ALL_CNT}"
fi
