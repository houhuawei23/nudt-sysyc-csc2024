
## Debug Compiler Using GDB with VSCode
- Install `C/C++ Extension Pack` and `CMake` extension in VSCode.
- Config and build your project using CMake.
- Edit `launch.json` file in `.vscode` folder in your project directory.
- Debug your compiler using GDB with VSCode.

exampe `launch.json` file:
```json

{
    "configurations": [
        {
            "name": "(gdb) Launch",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceRoot}/main",
            "args": [
                "-f",
                "${workspaceRoot}/test/.local_test/local_test.c",
                "-S",
                "-L1",
                "-o",
                "${workspaceRoot}/test/.local_test/local_test.s"
            ],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "externalConsole": false,
            "MIMode": "gdb",
            "setupCommands": [
                {
                    "description": "Enable pretty-printing for gdb",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                },
                {
                    "description": "Set Disassembly Flavor to Intel",
                    "text": "-gdb-set disassembly-flavor intel",
                    "ignoreFailures": true
                }
            ]
        },
    ],
    "version": "2.0.0"
}
```

## Debug Assembly Code Using GDB with VSCode

- Install `Native Debug` extension in VSCode.
- Add example configuration to `launch.json` file.
- Debug your assembly code using GDB with VSCode.
- [`Native Debug`](https://github.com/WebFreak001/code-debug?tab=readme-ov-file)


example `launch.json` file:
```json

{
    "configurations": [
        {
            "type": "gdb",
            "request": "attach",
            "name": "Attach to QEMU",
            "executable": "./gen.o",
            "target": "localhost:1235",
            "remote": true,
            "cwd": "${workspaceRoot}",
            "gdbpath": "gdb-multiarch",
            "valuesFormatting": "parseText",
            "autorun": [
                "b main",
            ]
        },
    ],
    "version": "2.0.0"
}
```

## Debug Python Using VSCode

- Install `Python` extension in VSCode.
- Add example configuration to `launch.json` file.
- Debug your Python code using VSCode.

Example `launch.json` file:
```json
{
    "configurations": [
        {
            "name": "Python Debugger: Current File with Arguments",
            "type": "debugpy",
            "request": "launch",
            "program": "${file}",
            "console": "integratedTerminal",
            "args": "${command:pickArgs}"
        },
        {
            "name": "Debug gen.py",
            "type": "debugpy",
            "request": "launch",
            "program": "${workspaceRoot}/src/target/template/gen.py",
            "console": "integratedTerminal",
            "args": [
                "${workspaceRoot}/src/target/generic/generic.yml",
                "${workspaceRoot}/src/target/riscv/riscv.yml",
                "${workspaceRoot}/"
            ]
        },

    ],
    "version": "2.0.0"
}
```