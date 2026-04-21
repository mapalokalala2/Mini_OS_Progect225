#include <stdio.h>
#include "../include/os.h"
#include "../include/pcb.h"
#include "../include/scheduler.h"
#include "../include/memory.h"
#include "../include/banker.h"
#include "../include/logger.h"
#include "../include/ipc.h"

int main(void) {
    // 1. Initialize system and modules
    init_system();
    pcb_init_table();
    memory_init();
    log_init();
    ipc_init();

    // 2. Sample process creation + scheduler call
    // TODO: create actual process requests using create_process()
    // TODO: use scheduler_run(SCHED_FCFS, 0) and scheduler_run(SCHED_ROUNDROBIN, quantum)

    printf("Mini OS skeleton started.\n");

    // 3. Display state
    list_processes();
    show_memory();
    show_logs();

    // 4. Demonstrate IPC functionality
    printf("\n--- IPC Demonstration ---\n");

    // Create some sample processes for IPC demo
    int pid1 = create_process("Dispatch", 5, 1, 100);
    int pid2 = create_process("Response", 8, 2, 150);
    int pid3 = create_process("Monitor", 3, 1, 80);

    if (pid1 >= 0 && pid2 >= 0 && pid3 >= 0) {
        // Demonstrate message queues
        printf("Sending messages between processes...\n");
        send_message(pid1, pid2, 1, "Emergency alert: Fire reported");
        send_message(pid1, pid3, 2, "Sensor data: Temperature=85C");
        send_message(pid2, pid1, 3, "Response team dispatched");

        // Demonstrate pipes
        printf("Creating pipe for data streaming...\n");
        int pipe_id = create_pipe(pid1, pid2);
        if (pipe_id >= 0) {
            write_to_pipe(pipe_id, "Real-time sensor data stream");
        }

        // Demonstrate shared memory
        printf("Creating shared memory segment...\n");
        int shm_id = create_shared_memory(pid1, 200, "Incident Database");
        if (shm_id >= 0) {
            attach_shared_memory(shm_id, pid2);
            attach_shared_memory(shm_id, pid3);
        }

        // Display IPC status
        printf("\nIPC System Status:\n");
        list_messages();
        list_pipes();
        list_shared_memory();
        show_ipc_statistics();
    }

    return 0;
}
