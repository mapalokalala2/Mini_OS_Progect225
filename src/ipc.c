#include "../include/ipc.h"
#include "../include/logger.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static ipc_system_t ipc_sys;

// Initialize IPC system
void ipc_init(void) {
    memset(&ipc_sys, 0, sizeof(ipc_system_t));
    ipc_sys.msg_count = 0;
    ipc_sys.pipe_count = 0;
    ipc_sys.shm_count = 0;
    log_event("IPC system initialized");
}

// ============== MESSAGE QUEUE OPERATIONS ==============

int send_message(int sender_pid, int receiver_pid, int msg_type, const char *data) {
    if (ipc_sys.msg_count >= MAX_MESSAGES) {
        log_event("Message queue full - cannot send message");
        return -1;
    }

    message_t *msg = &ipc_sys.messages[ipc_sys.msg_count];
    msg->msg_id = ipc_sys.msg_count;
    msg->sender_pid = sender_pid;
    msg->receiver_pid = receiver_pid;
    msg->msg_type = msg_type;
    strncpy(msg->data, data, MAX_MSG_SIZE - 1);
    msg->data[MAX_MSG_SIZE - 1] = '\0';
    msg->timestamp = time(NULL);
    msg->is_read = 0;

    ipc_sys.msg_count++;

    char log_msg[256];
    sprintf(log_msg, "Message sent: PID %d -> PID %d (type: %d)", sender_pid, receiver_pid, msg_type);
    log_event(log_msg);

    return msg->msg_id;
}

int receive_message(int receiver_pid, int msg_type, message_t *msg) {
    for (int i = 0; i < ipc_sys.msg_count; i++) {
        if (ipc_sys.messages[i].receiver_pid == receiver_pid &&
            ipc_sys.messages[i].msg_type == msg_type &&
            !ipc_sys.messages[i].is_read) {

            memcpy(msg, &ipc_sys.messages[i], sizeof(message_t));
            ipc_sys.messages[i].is_read = 1;

            char log_msg[256];
            sprintf(log_msg, "Message received: PID %d from PID %d (type: %d)",
                    receiver_pid, ipc_sys.messages[i].sender_pid, msg_type);
            log_event(log_msg);

            return 1;  // Success
        }
    }
    return 0;  // No message found
}

void list_messages(void) {
    printf("\n========== MESSAGES IN QUEUE ==========\n");
    if (ipc_sys.msg_count == 0) {
        printf("No messages in queue.\n");
        return;
    }

    printf("ID | Sender | Receiver | Type | Status   | Data\n");
    printf("----|--------|----------|------|----------|------------------------\n");

    for (int i = 0; i < ipc_sys.msg_count; i++) {
        message_t *msg = &ipc_sys.messages[i];
        printf("%2d | %6d | %8d | %4d | %s | %.20s\n",
               msg->msg_id, msg->sender_pid, msg->receiver_pid, msg->msg_type,
               msg->is_read ? "READ  " : "UNREAD",
               msg->data);
    }
}

void show_message(int msg_id) {
    if (msg_id < 0 || msg_id >= ipc_sys.msg_count) {
        printf("Invalid message ID.\n");
        return;
    }

    message_t *msg = &ipc_sys.messages[msg_id];
    printf("\n========== MESSAGE DETAILS ==========\n");
    printf("Message ID:    %d\n", msg->msg_id);
    printf("Sender PID:    %d\n", msg->sender_pid);
    printf("Receiver PID:  %d\n", msg->receiver_pid);
    printf("Message Type:  %d\n", msg->msg_type);
    printf("Status:        %s\n", msg->is_read ? "READ" : "UNREAD");
    printf("Data:          %s\n", msg->data);
    printf("Timestamp:     %s", ctime(&msg->timestamp));
}

// ============== PIPE OPERATIONS ==============

int create_pipe(int reader_pid, int writer_pid) {
    if (ipc_sys.pipe_count >= MAX_PIPES) {
        log_event("Pipe table full - cannot create pipe");
        return -1;
    }

    pipe_t *pipe = &ipc_sys.pipes[ipc_sys.pipe_count];
    pipe->pipe_id = ipc_sys.pipe_count;
    pipe->reader_pid = reader_pid;
    pipe->writer_pid = writer_pid;
    pipe->buffer_size = 0;
    pipe->is_closed = 0;
    pipe->created_time = time(NULL);
    memset(pipe->buffer, 0, MAX_MSG_SIZE);

    ipc_sys.pipe_count++;

    char log_msg[256];
    sprintf(log_msg, "Pipe created: PID %d (writer) -> PID %d (reader)", writer_pid, reader_pid);
    log_event(log_msg);

    return pipe->pipe_id;
}

int write_to_pipe(int pipe_id, const char *data) {
    if (pipe_id < 0 || pipe_id >= ipc_sys.pipe_count) {
        log_event("Invalid pipe ID");
        return -1;
    }

    pipe_t *pipe = &ipc_sys.pipes[pipe_id];

    if (pipe->is_closed) {
        log_event("Cannot write to closed pipe");
        return -1;
    }

    strncpy(pipe->buffer, data, MAX_MSG_SIZE - 1);
    pipe->buffer[MAX_MSG_SIZE - 1] = '\0';
    pipe->buffer_size = strlen(data);

    char log_msg[256];
    sprintf(log_msg, "Data written to pipe %d: %.30s", pipe_id, data);
    log_event(log_msg);

    return pipe->buffer_size;
}

int read_from_pipe(int pipe_id, char *data) {
    if (pipe_id < 0 || pipe_id >= ipc_sys.pipe_count) {
        return -1;
    }

    pipe_t *pipe = &ipc_sys.pipes[pipe_id];

    if (pipe->buffer_size == 0) {
        return 0;  // No data
    }

    strcpy(data, pipe->buffer);
    int size = pipe->buffer_size;
    pipe->buffer_size = 0;
    memset(pipe->buffer, 0, MAX_MSG_SIZE);

    char log_msg[256];
    sprintf(log_msg, "Data read from pipe %d", pipe_id);
    log_event(log_msg);

    return size;
}

void list_pipes(void) {
    printf("\n========== PIPES ==========\n");
    if (ipc_sys.pipe_count == 0) {
        printf("No pipes created.\n");
        return;
    }

    printf("ID | Writer | Reader | Status  | Data Size\n");
    printf("----|--------|--------|---------|----------\n");

    for (int i = 0; i < ipc_sys.pipe_count; i++) {
        pipe_t *pipe = &ipc_sys.pipes[i];
        printf("%2d | %6d | %6d | %s | %9d\n",
               pipe->pipe_id, pipe->writer_pid, pipe->reader_pid,
               pipe->is_closed ? "CLOSED " : "OPEN  ",
               pipe->buffer_size);
    }
}

int close_pipe(int pipe_id) {
    if (pipe_id < 0 || pipe_id >= ipc_sys.pipe_count) {
        return -1;
    }

    ipc_sys.pipes[pipe_id].is_closed = 1;

    char log_msg[256];
    sprintf(log_msg, "Pipe %d closed", pipe_id);
    log_event(log_msg);

    return 0;
}

// ============== SHARED MEMORY OPERATIONS ==============

int create_shared_memory(int owner_pid, int size, const char *data) {
    if (ipc_sys.shm_count >= MAX_SHARED_MEM) {
        log_event("Shared memory table full");
        return -1;
    }

    if (size > MAX_MSG_SIZE) {
        log_event("Shared memory size exceeds limit");
        return -1;
    }

    shared_mem_t *shm = &ipc_sys.shared_memory[ipc_sys.shm_count];
    shm->shm_id = ipc_sys.shm_count;
    shm->owner_pid = owner_pid;
    shm->size = size;
    shm->attach_count = 1;
    shm->created_time = time(NULL);

    strncpy(shm->data, data, size - 1);
    shm->data[size - 1] = '\0';
    shm->attached_processes[0] = owner_pid;

    ipc_sys.shm_count++;

    char log_msg[256];
    sprintf(log_msg, "Shared memory segment %d created by PID %d (size: %d)",
            shm->shm_id, owner_pid, size);
    log_event(log_msg);

    return shm->shm_id;
}

int attach_shared_memory(int shm_id, int pid) {
    if (shm_id < 0 || shm_id >= ipc_sys.shm_count) {
        return -1;
    }

    shared_mem_t *shm = &ipc_sys.shared_memory[shm_id];

    if (shm->attach_count >= MAX_PROCESSES) {
        log_event("Shared memory attachment limit reached");
        return -1;
    }

    // Check if already attached
    for (int i = 0; i < shm->attach_count; i++) {
        if (shm->attached_processes[i] == pid) {
            return 0;  // Already attached
        }
    }

    shm->attached_processes[shm->attach_count] = pid;
    shm->attach_count++;

    char log_msg[256];
    sprintf(log_msg, "PID %d attached to shared memory segment %d", pid, shm_id);
    log_event(log_msg);

    return 0;
}

int detach_shared_memory(int shm_id, int pid) {
    if (shm_id < 0 || shm_id >= ipc_sys.shm_count) {
        return -1;
    }

    shared_mem_t *shm = &ipc_sys.shared_memory[shm_id];

    for (int i = 0; i < shm->attach_count; i++) {
        if (shm->attached_processes[i] == pid) {
            // Remove by shifting
            for (int j = i; j < shm->attach_count - 1; j++) {
                shm->attached_processes[j] = shm->attached_processes[j + 1];
            }
            shm->attach_count--;

            char log_msg[256];
            sprintf(log_msg, "PID %d detached from shared memory segment %d", pid, shm_id);
            log_event(log_msg);

            return 0;
        }
    }

    return -1;  // Not found
}

int write_shared_memory(int shm_id, const char *data, int size) {
    if (shm_id < 0 || shm_id >= ipc_sys.shm_count) {
        return -1;
    }

    if (size > MAX_MSG_SIZE) {
        return -1;
    }

    shared_mem_t *shm = &ipc_sys.shared_memory[shm_id];
    strncpy(shm->data, data, size - 1);
    shm->data[size - 1] = '\0';
    shm->size = size;

    char log_msg[256];
    sprintf(log_msg, "Shared memory segment %d written (size: %d)", shm_id, size);
    log_event(log_msg);

    return size;
}

int read_shared_memory(int shm_id, char *data) {
    if (shm_id < 0 || shm_id >= ipc_sys.shm_count) {
        return -1;
    }

    shared_mem_t *shm = &ipc_sys.shared_memory[shm_id];
    strcpy(data, shm->data);

    char log_msg[256];
    sprintf(log_msg, "Shared memory segment %d read", shm_id);
    log_event(log_msg);

    return shm->size;
}

void list_shared_memory(void) {
    printf("\n========== SHARED MEMORY SEGMENTS ==========\n");
    if (ipc_sys.shm_count == 0) {
        printf("No shared memory segments created.\n");
        return;
    }

    printf("ID | Owner | Size | Attached | Data\n");
    printf("----|-------|------|----------|------------------------\n");

    for (int i = 0; i < ipc_sys.shm_count; i++) {
        shared_mem_t *shm = &ipc_sys.shared_memory[i];
        printf("%2d | %5d | %4d | %8d | %.20s\n",
               shm->shm_id, shm->owner_pid, shm->size, shm->attach_count, shm->data);
    }
}

// ============== STATISTICS AND LOGGING ==============

void show_ipc_statistics(void) {
    printf("\n========== IPC SYSTEM STATISTICS ==========\n");
    printf("Messages in Queue:       %d/%d\n", ipc_sys.msg_count, MAX_MESSAGES);

    int unread = 0;
    for (int i = 0; i < ipc_sys.msg_count; i++) {
        if (!ipc_sys.messages[i].is_read) unread++;
    }
    printf("Unread Messages:         %d\n", unread);

    printf("Active Pipes:            %d/%d\n", ipc_sys.pipe_count, MAX_PIPES);

    int open_pipes = 0;
    for (int i = 0; i < ipc_sys.pipe_count; i++) {
        if (!ipc_sys.pipes[i].is_closed) open_pipes++;
    }
    printf("Open Pipes:              %d\n", open_pipes);

    printf("Shared Memory Segments:  %d/%d\n", ipc_sys.shm_count, MAX_SHARED_MEM);
}

void log_ipc_event(const char *event) {
    log_event(event);
}