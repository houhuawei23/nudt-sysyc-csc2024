#!/bin/bash

# ./test.sh -t test/2021/functional/ -p mem2reg -p dce -p scp -p sccp -p simplifycfg -L1

set -u # dont ignore unset variables
# set -x # print all executed commands
# set -e

compiler_path="./compiler"

PASS_CNT=0
WRONG_CNT=0
ALL_CNT=0
TIMEOUT_CNT=0

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

# Default values
test_path="test/local_test"
output_dir="test/.out"
single_file=""
result_file="test/.out/result.txt"

error_code=0 # 0 for exe success

EC_MAIN=1
EC_LLVMLINK=2
EC_LLI=3
EC_TIMEOUT=124

TIMEOUT=3

error_code=0 # 0 for exe success

EC_MAIN=1
EC_LLVMLINK=2
EC_LLI=3
EC_TIMEOUT=124

TIMEOUT=10

lli_cmd="lli-17"
llvm_link_cmd="llvm-link-17"
# Function to print usage information
usage() {
    echo "Usage: $0 [-t <test_path>] [-o <output_dir>] [-r <result_file>] [-r <result_file>][-h]"
    echo "Options:"
    echo "  -t <test_path>  Specify the directory containing test files or single file (default: test/local_test)"
    echo "  -o <output_dir>     Specify the output directory (default: test/.out/)"
    echo "  -r <result_file>    Specify the file to store the test results (default: test/result.txt)"
    echo "  -h                  Print this help message"
}
# ./test.sh -t test/2021/functional/001_var_defn.sy -p mem2reg -p dce
# Parse command line arguments

# getopt
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
        # check not empty and not start with --
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
    ?)
        echo "Invalid option: -$OPTARG" >&2
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

function run_llvm_test() {
    local single_file="$1"
    local output_dir="$2"
    local result_file="$3"

    local in_file="${single_file%.*}.in"

    local sy_h="./test/link/sy.h"
    local link_ll="./test/link/link.ll"

    local llvm_c="${output_dir}/llvm_test.c"
    local llvm_ll="${output_dir}/llvm.ll"
    local llvm_ll_opt="${output_dir}/llvmopt.ll"
    local llvm_llinked="${output_dir}/llvm_linked.ll"

    llvm_out="${output_dir}/llvm.out"

    # llvm compiler
    if [ -f "${llvm_c}" ]; then
        rm "${llvm_c}"
    fi
    touch "${llvm_c}"
    cat "${sy_h}" >"${llvm_c}"
    cat "${single_file}" >>"${llvm_c}"

    clang --no-warnings -emit-llvm -S "${llvm_c}" -o "${llvm_ll}"  -O3 -std=c90 
    # # ./compiler -f "${llvm_c}" -i -o "${llvm_ll}"
    # opt -O3 -debug-pass-manager "${llvm_ll}" -o "${llvm_ll}"
    # ./compiler -f "${single_file}" -i  -o "${llvm_ll}" "${OPT_LEVEL}" "${LOG_LEVEL}"
    # opt -S "${llvm_ll}" -o "${llvm_ll_opt}" -p mem2reg -p adce -p inline -p tailcallelim -p inline -p adce -p simplifycfg -p licm -p gvn -p instcombine -p adce -p sccp -p simplifycfg 
    # -Wimplicit-function-declaration
    $llvm_link_cmd --suppress-warnings "${llvm_ll}" "${link_ll}" -S -o "${llvm_llinked}"

    if [ -f "$in_file" ]; then
        $lli_cmd "${llvm_llinked}" >"${llvm_out}" <"${in_file}"
    else
        $lli_cmd "${llvm_llinked}" >"${llvm_out}"
    fi
    local llvmres=$?
    # llvm compiler end
    return ${llvmres}

}
lookup_ll="./test/link/lookup.ll"
link_ll="./test/link/link.ll"
parallelfor_ll="./test/link/parallelFor.ll"
# parallelfor_ll="./test/link/cmmcParallelFor.ll"
function run_gen_test() {
    local single_file="$1"
    local output_dir="$2"
    local result_file="$3"

    local in_file="${single_file%.*}.in"

    local gen_c="${output_dir}/gen_test.c"
    local gen_ll="${output_dir}/gen.ll"
    local gen_llinked="${output_dir}/gen_linked.ll"
    gen_out="${output_dir}/gen.out"

    if [ -f "${gen_c}" ]; then
        rm "${gen_c}"
    fi
    touch "${gen_c}"
    cat "${single_file}" >"${gen_c}"

    # ./compiler "$single_file" >"${gen_ll}"
    $compiler_path -f "${single_file}" -i -t ${PASSES_STR} -o "${gen_ll}" "${OPT_LEVEL}" "${LOG_LEVEL}"
    if [ $? != 0 ]; then
        return $EC_MAIN
    fi

    $llvm_link_cmd --suppress-warnings $link_ll $lookup_ll $parallelfor_ll "${gen_ll}" -S -o "${gen_llinked}"
    if [ $? != 0 ]; then
        return $EC_LLVMLINK
    fi

    if [ -f "$in_file" ]; then
        timeout $TIMEOUT $lli_cmd "${gen_llinked}" >"${gen_out}" <"${in_file}"
        if [ $? == $EC_TIMEOUT ]; then # time out
            return $EC_TIMEOUT
        fi
        # not timeout, re-run
        $lli_cmd "${gen_llinked}" >"${gen_out}" <"${in_file}"
    else
        timeout $TIMEOUT $lli_cmd "${gen_llinked}" >"${gen_out}"
        if [ $? == $EC_TIMEOUT ]; then
            return $EC_TIMEOUT
        fi
        # not timeout, re-run
        $lli_cmd "${gen_llinked}" >"${gen_out}"
    fi
    local res=$?
    # gen compiler end
    return ${res}
}
# define a function that test one file
function run_test() {
    local single_file="$1"
    local output_dir="$2"
    local result_file="$3"

    if [ -f "$single_file" ]; then
        echo "${YELLOW}[Testing]${RESET} $single_file"

        run_llvm_test "${single_file}" "${output_dir}" "${result_file}"
        local llvmres=$?

        run_gen_test "${single_file}" "${output_dir}" "${result_file}"
        local res=$?

        # diff "${output_dir}/gen.out" "${output_dir}/llvm.out" >"/dev/null"
        diff "${gen_out}" "${llvm_out}" >"${output_dir}/diff.out"
        local diff_res=$?
        # diff res or diff stdout
        echo "[RESULT] res (${RED}${res}${RESET}), llvmres (${RED}${llvmres}${RESET})"

        if [ ${res} != ${llvmres} ] || [ ${diff_res} != 0 ]; then

            if [ ${res} == ${EC_MAIN} ]; then
                echo "${RED}[MAIN ERROR]${RESET} ${single_file}"
            elif [ ${res} == ${EC_LLVMLINK} ]; then
                echo "${RED}[LINK ERROR]${RESET} ${single_file}"
            elif [ ${res} == ${EC_LLI} ]; then
                echo "${RED}[LLI ERROR]${RESET} ${single_file}"
            elif [ ${res} == ${EC_TIMEOUT} ]; then
                echo "${RED}[TIMEOUT]${RESET} ${single_file}"
                echo "[TIMEOUT] ${single_file}" >>${result_file}
            else
                echo "${RED}[WRONG RES]${RESET} ${single_file}"
                echo "[WRONG RES] ${single_file}" >>${result_file}
                echo "  [WRONG RES]: res (${res}), llvmres (${llvmres})" >>${result_file}
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

## main run

# if test_path is a file
if [ -f "$test_path" ]; then
    run_test "$test_path" "$output_dir" "$result_file"
    # run_gen_test $test_path $output_dir $result_file
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
                run_test "${file}" "${output_dir}" "${result_file}"
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

    ALL_CNT=$((PASS_CNT + WRONG_CNT))
    echo "${GREEN}PASS ${RESET}: ${PASS_CNT}"
    echo "${RED}WRONG${RESET}: ${WRONG_CNT}"
    echo "${RED}TIMEOUT${RESET}: ${TIMEOUT_CNT}"
    echo "${YELLOW}ALL  ${RESET}: ${ALL_CNT}"
fi
