# C++ Dev Container Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Create a VS Code C++ Linux dev container, CMake build configuration, starter C++ code, and documentation for group members.

**Architecture:** Use the official Microsoft C++ Ubuntu 24.04 container image with VS Code extensions for C/C++ and CMake Tools. The C++ code lives in `src/` managed by a root `CMakeLists.txt`.

**Tech Stack:** Docker, VS Code Dev Containers, Ubuntu 24.04, GCC, GDB, CMake, Ninja, C++20.

## Global Constraints
- Dev container image: `mcr.microsoft.com/devcontainers/cpp:ubuntu-24.04`
- C++ Standard: C++20
- Build tool: CMake with Ninja generator
- Executable binary name: `app`
- Dev container config path: `.devcontainer/devcontainer.json`
- Source code directory: `src/`
- Documentation path: `docs/cpp-development.md`

---

### Task 1: Dev Container Configuration and Git Ignore Updates

**Files:**
- Modify: `.gitignore`
- Create: `.devcontainer/devcontainer.json`
- Test: Validate JSON syntax with node

**Interfaces:**
- Consumes: Microsoft dev container standard schema
- Produces: `.devcontainer/devcontainer.json` ready for VS Code Dev Containers extension

- [ ] **Step 1: Update .gitignore with C++ build output directories**

Append `build/` and `.cache/` to `.gitignore` so local and container CMake artifacts are not tracked.

```gitignore
# C++ build directories
build/
.cache/
compile_commands.json
```

- [ ] **Step 2: Create .devcontainer/devcontainer.json**

Create `.devcontainer/devcontainer.json`:

```json
{
  "name": "C++ Linux Dev Environment",
  "image": "mcr.microsoft.com/devcontainers/cpp:ubuntu-24.04",
  "customizations": {
    "vscode": {
      "extensions": [
        "ms-vscode.cpptools",
        "ms-vscode.cmake-tools",
        "twxs.cmake"
      ],
      "settings": {
        "cmake.sourceDirectory": "${workspaceFolder}",
        "cmake.buildDirectory": "${workspaceFolder}/build",
        "C_Cpp.default.cppStandard": "c++20"
      }
    }
  },
  "remoteUser": "vscode"
}
```

- [ ] **Step 3: Validate JSON syntax**

Run: `node -e "JSON.parse(require('fs').readFileSync('.devcontainer/devcontainer.json', 'utf8')); console.log('Valid JSON')"`
Expected: `Valid JSON`

- [ ] **Step 4: Commit Task 1 changes**

```bash
git add .gitignore .devcontainer/devcontainer.json
git commit -m "feat(devcontainer): add C++ Linux devcontainer configuration"
```

---

### Task 2: C++ Scaffolding and CMake Build Setup

**Files:**
- Create: `CMakeLists.txt`
- Create: `src/main.cpp`
- Test: Verify CMake configuration and syntax

**Interfaces:**
- Consumes: `src/main.cpp`
- Produces: Executable target `app` configured with C++20

- [ ] **Step 1: Create root CMakeLists.txt**

Write `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.20)
project(PassTheControllerCpp VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCES
    src/main.cpp
)

add_executable(app ${SOURCES})
target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
```

- [ ] **Step 2: Create starter C++ file at src/main.cpp**

Write `src/main.cpp`:

```cpp
#include <iostream>
#include <string_view>

enum class ControllerState {
    Disconnected,
    Connected,
    Active
};

std::string_view stateToString(ControllerState state) {
    switch (state) {
        case ControllerState::Disconnected: return "Disconnected";
        case ControllerState::Connected:    return "Connected";
        case ControllerState::Active:       return "Active";
    }
    return "Unknown";
}

int main() {
    ControllerState current = ControllerState::Connected;
    std::cout << "Project ready. Controller State: " << stateToString(current) << '\n';
    return 0;
}
```

- [ ] **Step 3: Verify build generation with CMake**

Run: `cmake -B build -S .`
Expected: CMake generates build files successfully without errors.

- [ ] **Step 4: Verify build compilation**

Run: `cmake --build build`
Expected: Target `app` builds successfully with exit code 0.

- [ ] **Step 5: Run the compiled executable**

Run: `./build/Debug/app.exe` (or `./build/app` depending on generator)
Expected: Prints `Project ready. Controller State: Connected`

- [ ] **Step 6: Commit Task 2 changes**

```bash
git add CMakeLists.txt src/main.cpp
git commit -m "feat(cpp): add CMake build configuration and starter main.cpp"
```

---

### Task 3: Developer Documentation and Project README Update

**Files:**
- Create: `docs/cpp-development.md`
- Modify: `README.md`
- Test: Verify all referenced files exist

**Interfaces:**
- Consumes: User documentation requirements
- Produces: Step-by-step developer guide for group members

- [ ] **Step 1: Create docs/cpp-development.md**

Write `docs/cpp-development.md` detailing:
- Prerequisites (Docker Desktop, VS Code, Dev Containers extension).
- How to open the project in the container (`Dev Containers: Reopen in Container`).
- How to build and run using VS Code status bar and terminal.
- How to add new classes, headers, and update `CMakeLists.txt`.

- [ ] **Step 2: Update README.md with C++ quickstart section**

Add a section to `README.md` pointing teammates to `docs/cpp-development.md`.

- [ ] **Step 3: Verify document links and syntax**

Verify that all file references and paths resolve correctly.

- [ ] **Step 4: Commit Task 3 changes**

```bash
git add docs/cpp-development.md README.md
git commit -m "docs: add C++ development guide and update README"
```

---

### Task 4: Full Verification and Cleanup

**Files:**
- Test: Git status, clean working tree, verified build

- [ ] **Step 1: Clean temporary build artifacts**

Run: `Remove-Item -Recurse -Force build`
Ensure no build artifacts are tracked in git.

- [ ] **Step 2: Final git status check**

Run: `git status`
Expected: Working tree clean.
