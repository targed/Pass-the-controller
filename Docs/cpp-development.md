# C++ Development Guide (Dev Container)

This guide walks through using the pre-configured C++ Linux development container for building, running, and extending the C++ class and state machine code.

## Environment Overview

The project provides an automated VS Code Dev Container environment based on Ubuntu 24.04. This setup guarantees that every team member builds against the exact same toolchain regardless of host operating system (Windows, macOS, or Linux).

The container environment includes:
- Ubuntu 24.04 LTS base environment
- GCC and G++ compiler toolchain with C++20 support
- GDB debugger
- CMake (version 3.20 or newer)
- Ninja build system
- Clang tooling and Git
- Pre-installed VS Code extensions: C/C++ (`ms-vscode.cpptools`), CMake Tools (`ms-vscode.cmake-tools`), and CMake syntax (`twxs.cmake`)

The root executable target is `app`, compiled from source files in `src/` via `CMakeLists.txt`.

## Prerequisites

Before opening the project in the container, ensure you have:
1. Docker Desktop (Windows/macOS) or Docker Engine (Linux) installed and running.
2. Visual Studio Code installed.
3. Dev Containers extension (`ms-vscode-remote.remote-containers`) installed in VS Code.

## Opening the Project in the Container

1. Open the repository root folder (`Pass-the-controller`) in VS Code.
2. If VS Code displays a notification prompting to "Reopen in Container", click it.
3. Alternatively, open the Command Palette (`F1` or `Ctrl+Shift+P` on Windows/Linux, `Cmd+Shift+P` on macOS) and run:
   ```text
   Dev Containers: Reopen in Container
   ```
4. VS Code will pull the base container image (`mcr.microsoft.com/devcontainers/cpp:ubuntu-24.04`), install extensions inside the container, and mount the repository into `/workspaces/Pass-the-controller`.
5. Once loaded, you are operating as the `vscode` user with full development privileges.

## Building and Running the Code

You can build and execute the application either via the VS Code interface or using the integrated terminal.

### Method A: VS Code Status Bar (CMake Tools)

The CMake Tools extension adds interactive controls directly to the VS Code bottom status bar:
- Build: Click the **Build** button on the bottom bar, or press `F7`.
- Run: Click the **Run** button (play icon) on the bottom bar, or press `Shift+F5`.
- Target: Ensure the active build target is `[app]`.

Program output appears in the CMake and terminal output tabs.

### Method B: Terminal in Container

Open an integrated terminal inside VS Code (`Ctrl+\`` or `Terminal` -> `New Terminal`). From the repository root, run:

```bash
# Configure the build directory with Ninja
cmake -B build -G Ninja

# Compile the target executable
cmake --build build

# Run the compiled binary
./build/app
```

Expected output:
```text
Project ready. Controller State: Connected
```

## Adding New Classes and State Diagrams

During in-class activities, team members translate class diagrams and state machine specifications into C++ code.

### 1. Place Headers and Sources in `src/`

Create your header files (`.hpp`) and implementation files (`.cpp`) inside the `src/` directory.

Example layout:
```text
src/
├── main.cpp
├── controller.hpp
└── controller.cpp
```

Because `CMakeLists.txt` registers `src/` as a private include directory, you can include headers using quotes without writing relative paths:
```cpp
#include "controller.hpp"
```

### 2. Register Source Files in `CMakeLists.txt`

When creating a new `.cpp` file, add it to the `SOURCES` list in `CMakeLists.txt`:

```cmake
set(SOURCES
    src/main.cpp
    src/controller.cpp
)
```

Headers (`.hpp`) do not need to be added to `SOURCES`, though including them in your source files allows CMake Tools and IntelliSense to index them automatically.

Save `CMakeLists.txt`. CMake Tools will detect the file change and reconfigure the build.

### 3. Rebuild and Verify

Run `cmake --build build` or press `F7` to verify that your new classes compile without warnings or errors.

## Maintenance and Troubleshooting

- Docker daemon not running: Ensure Docker Desktop is active on your host machine before starting VS Code.
- Rebuilding the container: If container settings are modified, press `F1` and select `Dev Containers: Rebuild Container`.
- Cleaning build artifacts: To clean compilation output, delete the `build/` directory with `rm -rf build` inside the container terminal, or use the "Clean Rebuild" option in CMake Tools. Build directories are ignored by Git.
