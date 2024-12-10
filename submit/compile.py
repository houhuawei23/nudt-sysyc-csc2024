# compile only on offical docker image container
# https://pan.educg.net/#/s/V2oiq?path=%2F

import os
import subprocess
from pathlib import Path
import shutil
import sys
def bfs_search(directory):
    link_file_list = []
    cpp_file_list = []
    java_file_list = []
    is_cpp_project = False
    is_java_project = False
    linked_dirs = set()

    queue = [directory]

    while queue:
        current_dir = queue.pop(0)

        for entry in os.scandir(current_dir):
            if entry.name in ['.git', 'build', '3rd_party']:
                continue
            if entry.is_dir():
                queue.append(entry.path)
            elif entry.is_file():
                file_suffix = os.path.splitext(entry.name)[1]
                file_path = entry.path
                if file_suffix in ['.hpp', '.hh', '.hxx', '.H', '.h']:
                    is_cpp_project = True
                    link_file_list.append(file_path)
                    linked_dirs.add(os.path.dirname(file_path))
                elif file_suffix in ['.cpp', '.CPP', '.cxx', '.C', '.cc', '.cp', '.c']:
                    is_cpp_project = True
                    cpp_file_list.append(file_path)
                elif file_suffix == '.java':
                    is_java_project = True
                    java_file_list.append(file_path)

    return is_cpp_project, is_java_project, cpp_file_list, link_file_list, java_file_list, linked_dirs

def compile_project(source_path, target_path):
    is_cpp_project, is_java_project, cpp_files, link_files, java_files, linked_dirs = bfs_search(source_path)
    compile_cmd = ""
    mv_cmd = ""

    if is_java_project:
        third_lib_base = "/coursegrader/dockerext/"
        libs = [os.path.join(third_lib_base, name) for name in os.listdir(third_lib_base) if name.endswith(".jar") and name != "ARMKernel.jar"]
        out_folder = "/path/to/exec_folder"  # Adjust to the actual exec folder path
        out_classes_path = Path(out_folder, "classes")
        
        if out_classes_path.exists():
            shutil.rmtree(out_classes_path)
        out_classes_path.mkdir(parents=True)

        compile_cmd = f"javac -d {out_classes_path} -encoding utf-8 -cp .:{':'.join(libs)} -sourcepath {source_path} {' '.join(java_files)}"
        target_jar = Path(out_folder, "compiled_project.jar")  # Adjust the jar name as needed
        if target_jar.exists():
            target_jar.unlink()

        compile_cmd += f" && cd {out_classes_path} && {' && '.join([f'jar xf {lib}' for lib in libs])} && jar --create --file {target_jar} --main-class Compiler -C {out_classes_path} ."
    
    elif is_cpp_project:
        compile_cmd_header = "clang++ -std=c++20 -O2 -g -lm -L/extlibs -I/extlibs -lantlr4-runtime -lpthread"
        compile_cmd = f"{compile_cmd_header} {' '.join(['-I' + dir for dir in linked_dirs])} -o {target_path} {' '.join(cpp_files)} "
        # compile_cmd += "-L/usr/local/lib/ -I/usr/local/include/antlr4-runtime/ -lm -lantlr4-runtime"

    else:
        raise Exception("Unsupported project type or no source files found")

    print(f"Compilation command: {compile_cmd}")
    if mv_cmd:
        print(f"Move command: {mv_cmd}")

    try:
        result = subprocess.run(compile_cmd, shell=True, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        print(result.stdout.decode())
        if mv_cmd:
            result = subprocess.run(mv_cmd, shell=True, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            print(result.stdout.decode())
    except subprocess.CalledProcessError as e:
        print("Compilation failed")
        print(e.stdout.decode())
        print(e.stderr.decode())
        raise

# Example usage:
# python compile.py . ./compiler
source_path = sys.argv[1]
target_path = sys.argv[2]
compile_project(source_path, target_path)
