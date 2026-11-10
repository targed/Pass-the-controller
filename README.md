# C++ Class & State Diagram Modeling Activity

This repository branch contains the C++ starter project and Linux Dev Container for our group activity translating class and state diagrams into working C++ code.

---

## Quickstart

### Prerequisites
1. Install [Docker Desktop](https://www.docker.com/products/docker-desktop/) (Windows or macOS) or Docker Engine (Linux).
2. Install [Visual Studio Code](https://code.visualstudio.com/).
3. Install the **Dev Containers** extension (`ms-vscode-remote.remote-containers`) in VS Code.

### Launching the Dev Container
1. Open this repository folder in VS Code.
2. When prompted with "Reopen in Container", click it. Alternatively, press `F1`, type `Dev Containers: Reopen in Container`, and press Enter.
3. VS Code will load the Ubuntu 24.04 environment pre-configured with GCC, GDB, CMake, Ninja, and CMake Tools.

---

## Building and Running

### Option 1: VS Code CMake Tools (Status Bar)
* **Build:** Click **Build** on the bottom status bar or press `F7`.
* **Run:** Click the **Play** icon on the bottom status bar or press `Shift+F5`.
* **Debug:** Press `F5` to start interactive debugging with GDB.

### Option 2: Terminal (Inside the Container)
Open a terminal in VS Code inside the container (`Ctrl+\``):
```bash
cmake -B build -G Ninja
cmake --build build
./build/app
```

---

## Project Structure

```
Pass-the-controller/
├── .devcontainer/
│   └── devcontainer.json   # Ubuntu 24.04 C++ Dev Container definition
├── src/
│   └── main.cpp            # Starter application and state model
├── Docs/
│   ├── cpp-development.md  # Detailed developer guide and workflow
│   └── superpowers/        # Design specifications and implementation plans
├── CMakeLists.txt          # CMake build configuration (C++20)
└── README.md               # This file
```

---

## Adding New Classes and State Models

1. Place your C++ header files (`.hpp`) and implementation files (`.cpp`) in the `src/` directory.
2. Open `CMakeLists.txt` and add your `.cpp` files to the `SOURCES` list:
   ```cmake
   set(SOURCES
       src/main.cpp
       src/your_class.cpp
   )
   ```
3. Re-run your build using `F7` or `cmake --build build`.

For more details, see the [C++ Development Guide](Docs/cpp-development.md).
