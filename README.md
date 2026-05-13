# Mini OS Dashboard

A comprehensive Operating System simulation environment written in C, featuring both a modern GTK+ 3.0 graphical interface and a classic command-line interface. This project simulates core OS responsibilities including process management, memory allocation, CPU scheduling, and deadlock avoidance.

## 🚀 Features

### 1. Process Management
* **Creation**: Dynamically create processes with custom names, burst times, priorities, and memory requirements.
* **Monitoring**: Real-time process table tracking (PID, State, Remaining Time, etc.).
* **Termination**: Gracefully terminate processes by PID.

### 2. CPU Scheduling
Simulate various scheduling algorithms with Gantt Chart visualization:
* **First-Come-First-Serve (FCFS)**: Non-preemptive scheduling based on arrival.
* **Round Robin (RR)**: Preemptive scheduling using a configurable time quantum.
* **Priority Scheduling**: Schedules processes based on priority levels with arrival time tie-breaking.

### 3. Memory Management
* **Dynamic Allocation**: Linked-list based memory management simulating RAM allocation.
* **Memory Map**: Visual and tabular representation of allocated and free memory blocks.
* **Coalescing**: Automatic merging of adjacent free memory blocks upon process termination.

### 4. Deadlock Handling (Banker's Algorithm)
* **Avoidance**: Uses the Banker's Algorithm to ensure the system never enters an unsafe state.
* **Resource Management**: Initialize resource types, set maximum claims, and handle resource requests/releases.
* **Safety Check**: Simulate allocations to verify system safety before granting resources.

### 5. System Logging
* Persistent logging of all system events to `OS_Logs.txt` with high-resolution timestamps.

## 🏗 Project Structure

* **`Central Point/`**: Main entry points for the application.
    * `main_GUI.c`: The GTK+ 3.0 dashboard implementation.
    * `main.c`: The CLI-based interactive menu.
* **`src/`**: Core logic implementations.
    * `pcb.c`: Process Control Block management.
    * `scheduler.c`: CPU scheduling algorithms.
    * `memory.c`: Linked-list memory management logic.
    * `banker.c`: Resource allocation and safety check logic.
    * `logger.c`: Event logging system.
* **`include/`**: Header files defining system-wide structures and constants.

## 🛠 Technologies Used

* **Language**: C (C11 Standard)
* **GUI Framework**: GTK+ 3.0
* **Graphics**: Cairo Graphics API (for Gantt Chart rendering)
* **Styling**: GTK CSS for the "Phantom Blue" dark theme
* **Build Tools**: GCC, Pkg-config

## 🚦 How to Run

### Prerequisites
* **Windows**: MSYS2 with `mingw-w64-x86_64-gtk3` installed.
* **Linux**: `libgtk-3-dev` package.

### Compilation

**To build the GUI Dashboard:**
```bash
gcc -g -I include "Central Point/main_GUI.c" src/pcb.c src/scheduler.c src/memory.c src/banker.c src/logger.c -o mini_os_gui.exe $(pkg-config --cflags --libs gtk+-3.0)
```

**To build the CLI Version:**
```cmd
gcc -g -I include "Central Point/main.c" src/pcb.c src/scheduler.c src/memory.c src/banker.c src/logger.c -o mini_os.exe
```

### Execution
Run the generated executables:
```bash
./mini_os_gui.exe
# OR
./mini_os.exe
```

## 📝 Note on Color Scheme
The GUI features a custom **Black and Phantom Blue** theme with gray text for high readability and a professional aesthetic, implemented via a custom CSS provider in the main application loop.

---
*Developed as a 225 PROJECT workspace.*