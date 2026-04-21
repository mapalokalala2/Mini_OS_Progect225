// Inter-Process Communication (IPC) Header
#ifndef IPC_H
#define IPC_H

#include <time.h>
#include "os.h"

#define MAX_MESSAGES 128
#define MAX_PIPES 64
#define MAX_SHARED_MEM 32
#define MAX_MSG_SIZE 256

// Message Queue Structures
typedef struct {
    int msg_id;
    int sender_pid;
    int receiver_pid;
    int msg_type;
    char data[MAX_MSG_SIZE];
    time_t timestamp;
    int is_read;
} message_t;

// Pipe Structure (simulated)
typedef struct {
    int pipe_id;
    int reader_pid;
    int writer_pid;
    char buffer[MAX_MSG_SIZE];
    int buffer_size;
    int is_closed;
    time_t created_time;
} pipe_t;

// Shared Memory Region
typedef struct {
    int shm_id;
    int owner_pid;
    char data[MAX_MSG_SIZE];
    int size;
    int attached_processes[MAX_PROCESSES];
    int attach_count;
    time_t created_time;
} shared_mem_t;

// IPC System Management
typedef struct {
    message_t messages[MAX_MESSAGES];
    int msg_count;
    pipe_t pipes[MAX_PIPES];
    int pipe_count;
    shared_mem_t shared_memory[MAX_SHARED_MEM];
    int shm_count;
} ipc_system_t;

// Function Prototypes
void ipc_init(void);

// Message Queue Operations
int send_message(int sender_pid, int receiver_pid, int msg_type, const char *data);
int receive_message(int receiver_pid, int msg_type, message_t *msg);
void list_messages(void);
void show_message(int msg_id);

// Pipe Operations
int create_pipe(int reader_pid, int writer_pid);
int write_to_pipe(int pipe_id, const char *data);
int read_from_pipe(int pipe_id, char *data);
void list_pipes(void);
int close_pipe(int pipe_id);

// Shared Memory Operations
int create_shared_memory(int owner_pid, int size, const char *data);
int attach_shared_memory(int shm_id, int pid);
int detach_shared_memory(int shm_id, int pid);
int write_shared_memory(int shm_id, const char *data, int size);
int read_shared_memory(int shm_id, char *data);
void list_shared_memory(void);

// IPC Statistics
void show_ipc_statistics(void);
void log_ipc_event(const char *event);

#endif // IPC_H