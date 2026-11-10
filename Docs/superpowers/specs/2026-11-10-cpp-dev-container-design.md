# C++ Dev Container Environment Design

## Overview
This design provides a unified, cross-platform Linux development container and C++ build toolchain for the project. It enables group members on different host operating systems (macOS, Windows, Linux) to write, compile, and run C++ translations of class and state diagrams under an identical Linux environment.

## Goals and Requirements
* Provide a VS Code Dev Container configuration based on modern Ubuntu Linux.
* Include standard C++ build and debugging tools: GCC, GDB, CMake, and Ninja.
* Scaffolding for C++ code living in a dedicated `src/` folder.
* Support modern C++ standard (C++20).
* Provide clear documentation for opening the container, building, running, and adding new classes.
* Keep configuration minimal, standard, and easy to extend.

## Architecture and Components

### 1. Dev Container Configuration
* File: `.devcontainer/devcontainer.json`
* Base image: `mcr.microsoft.com/devcontainers/cpp:ubuntu-24.04`
* Pre-installed utilities in image: `gcc`, `g++`, `gdb`, `cmake`, `ninja-build`, `git`, `clang-format`.
* VS Code Extensions:
  * `ms-vscode.cpptools` (C/C++ IntelliSense, debugging)
  * `ms-vscode.cmake-tools` (CMake integration, build/run status bar buttons)
  * `twxs.cmake` (CMake language syntax)
* Settings:
  * `cmake.sourceDirectory`: `${workspaceFolder}`
  * `cmake.buildDirectory`: `${workspaceFolder}/build`
  * `C_Cpp.default.cppStandard`: `c++20`
* User: `vscode` (non-root with sudo privileges).

### 2. CMake Build Configuration
* File: `CMakeLists.txt`
* C++ Standard: C++20 (required)
* Compile commands: `CMAKE_EXPORT_COMPILE_COMMANDS ON` for IntelliSense and clang tooling.
* Executable target: `app`
* Source list: Defined via a `SOURCES` variable starting with `src/main.cpp`.
* Header include directory: `src/` added as private include directory.

### 3. Starter C++ Implementation
* File: `src/main.cpp`
* Demonstrates an initial enum class state model (`ControllerState`) and status reporter.
* Serves as an immediate smoke test when opening the workspace and compiling.

### 4. Documentation
* File: `docs/cpp-development.md`
* Section in `README.md` referencing `docs/cpp-development.md`.
* Contents:
  * Host prerequisites (Docker, VS Code, Dev Containers extension).
  * How to open in container (`Dev Containers: Reopen in Container`).
  * How to build and run via CMake Tools UI and terminal.
  * How to add new `.hpp` and `.cpp` files to `src/` and `CMakeLists.txt`.

## Error Handling and Edge Cases
* **Docker not running:** Documentation highlights checking Docker status before opening in container.
* **Architecture differences:** The official Microsoft image provides native multi-arch manifests (`amd64` and `arm64`), preventing emulation issues on Apple Silicon.
* **Build directory clean-up:** `build/` is added to `.gitignore` so generated build artifacts are not checked into git.
