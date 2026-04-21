# Inter-Process Communication (IPC) in Mini-OS

## Overview

Inter-Process Communication (IPC) is a mechanism that allows **different processes to communicate and synchronize with each other** in an operating system. Since processes run in separate memory spaces and can't directly access each other's data, IPC provides structured ways for them to exchange information and coordinate their activities.

## Why IPC is Needed

In a multi-process system like your Mini-OS:

1. **Data Sharing**: Processes need to exchange data (e.g., emergency response teams sharing incident information)
2. **Synchronization**: Processes need to coordinate actions (e.g., one process waits for another to complete a task)
3. **Resource Coordination**: Processes need to request/release shared resources safely
4. **Event Notification**: Processes need to signal events to other processes

## Types of IPC Mechanisms

### 1. **Message Queues** (Implemented in your Mini-OS)
- **How it works**: Processes send/receive structured messages through a queue
- **Use case**: Asynchronous communication where sender doesn't wait for receiver
- **Your implementation**: `send_message()` and `receive_message()` functions
- **Example**: Emergency dispatch sending alerts to response teams

### 2. **Pipes** (Implemented in your Mini-OS)
- **How it works**: One-way data channel between processes (like a hose)
- **Use case**: Streaming data from one process to another
- **Your implementation**: `create_pipe()`, `write_to_pipe()`, `read_from_pipe()`
- **Example**: Real-time sensor data flowing from monitoring process to analysis process

### 3. **Shared Memory** (Implemented in your Mini-OS)
- **How it works**: Processes access the same memory region directly
- **Use case**: High-speed data sharing (no copying needed)
- **Your implementation**: `create_shared_memory()`, `attach_shared_memory()`
- **Example**: Shared incident database that multiple processes can read/write

### 4. **Signals** (Common in real OS)
- **How it works**: Software interrupts sent to processes
- **Use case**: Process termination, alarms, or status changes
- **Example**: Parent process signaling child process to terminate

### 5. **Sockets** (Network IPC)
- **How it works**: Network-style communication between processes (even on different machines)
- **Use case**: Distributed systems communication
- **Example**: Web server communicating with database server

## How IPC Works in Your Mini-OS

Your implementation simulates IPC within a single program by:

1. **Global Data Structures**: All IPC mechanisms use shared data structures that all simulated processes can access
2. **Process Identification**: Each process has a unique PID for addressing messages/pipes
3. **Synchronization**: Built-in coordination through the scheduler and process states
4. **Logging**: All IPC operations are logged to `logs.txt` for debugging

## Implementation Details

### Files Created/Modified

#### New Files:
- **`include/ipc.h`**: Header file containing IPC structures and function prototypes
- **`src/ipc.c`**: Complete implementation of all IPC mechanisms

#### Modified Files:
- **`Makefile`**: Added `src/ipc.c` to compilation sources
- **`src/main.c`**: Added IPC initialization and GUI buttons for IPC operations

### IPC System Architecture

```c
// Core IPC System Structure
typedef struct {
    message_t messages[MAX_MESSAGES];      // Message queue
    int msg_count;
    pipe_t pipes[MAX_PIPES];              // Pipe system
    int pipe_count;
    shared_mem_t shared_memory[MAX_SHARED_MEM];  // Shared memory
    int shm_count;
} ipc_system_t;
```

### Message Queue Implementation

**Data Structure:**
```c
typedef struct {
    int msg_id;
    int sender_pid;
    int receiver_pid;
    int msg_type;
    char data[MAX_MSG_SIZE];
    time_t timestamp;
    int is_read;
} message_t;
```

**Key Functions:**
- `send_message(int sender_pid, int receiver_pid, int msg_type, const char *data)`
- `receive_message(int receiver_pid, int msg_type, message_t *msg)`
- `list_messages(void)`

### Pipe Implementation

**Data Structure:**
```c
typedef struct {
    int pipe_id;
    int reader_pid;
    int writer_pid;
    char buffer[MAX_MSG_SIZE];
    int buffer_size;
    int is_closed;
    time_t created_time;
} pipe_t;
```

**Key Functions:**
- `create_pipe(int reader_pid, int writer_pid)`
- `write_to_pipe(int pipe_id, const char *data)`
- `read_from_pipe(int pipe_id, char *data)`
- `close_pipe(int pipe_id)`

### Shared Memory Implementation

**Data Structure:**
```c
typedef struct {
    int shm_id;
    int owner_pid;
    char data[MAX_MSG_SIZE];
    int size;
    int attached_processes[MAX_PROCESSES];
    int attach_count;
    time_t created_time;
} shared_mem_t;
```

**Key Functions:**
- `create_shared_memory(int owner_pid, int size, const char *data)`
- `attach_shared_memory(int shm_id, int pid)`
- `detach_shared_memory(int shm_id, int pid)`
- `write_shared_memory(int shm_id, const char *data, int size)`
- `read_shared_memory(int shm_id, char *data)`

## GUI Integration (OS_linux version)

The Mini-OS GUI now includes an **"Inter-Process Communication (IPC)"** section with buttons:

- **List Messages**: Display all messages in the message queue
- **List Pipes**: Show active pipes and their status
- **List Shared Memory**: Display shared memory segments
- **IPC Statistics**: Show usage statistics for all IPC mechanisms

## Console Demo (Student version)

The student version includes a demonstration of IPC functionality in the main function:

```c
// Create some sample processes for IPC demo
int pid1 = create_process("Dispatch", 5, 1, 100);
int pid2 = create_process("Response", 8, 2, 150);
int pid3 = create_process("Monitor", 3, 1, 80);

// Demonstrate message queues
send_message(pid1, pid2, 1, "Emergency alert: Fire reported");

// Demonstrate pipes
int pipe_id = create_pipe(pid1, pid2);
write_to_pipe(pipe_id, "Real-time sensor data stream");

// Demonstrate shared memory
int shm_id = create_shared_memory(pid1, 200, "Incident Database");
attach_shared_memory(shm_id, pid2);
```

## Real-World IPC vs. Your Simulation

| Aspect | Real OS IPC | Your Mini-OS IPC |
|--------|-------------|------------------|
| **Memory** | Separate address spaces | Single program memory |
| **Security** | Process isolation | No isolation needed |
| **Performance** | System calls overhead | Direct function calls |
| **Persistence** | Survives process crashes | Lost when program exits |

## Educational Value

This IPC implementation demonstrates:

1. **Process Coordination**: How processes work together in a system
2. **Data Structures**: Efficient storage and retrieval of inter-process data
3. **Synchronization**: Managing concurrent access to shared resources
4. **System Design**: Building complex systems from simpler components
5. **Real-World Application**: Emergency response coordination scenarios

## Assignment Compliance

Your Mini-OS now implements **all 6 recommended OS components**:

✅ **Process Management** - PCB, process lifecycle
✅ **CPU Scheduling** - FCFS and Round Robin algorithms
✅ **Memory Management** - First Fit allocation with coalescing
✅ **Inter-Process Communication** - Message queues, pipes, shared memory
✅ **Deadlock Handling** - Banker's algorithm for resource allocation
✅ **File Management** - Logging system with timestamped events

The IPC implementation provides a complete, working example of how processes communicate in an operating system, specifically tailored for emergency response coordination scenarios.