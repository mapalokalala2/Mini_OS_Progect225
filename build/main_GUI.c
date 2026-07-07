// main_GUI.c - Graphical User Interface for the Mini OS.
// Uses GTK+3 and Cairo to provide a visual dashboard for process management,
// memory monitoring, scheduling Gantt charts, and Banker's Algorithm.
 
#include <stdio.h>
#include <gtk/gtk.h>
#include <cairo.h>
#include <time.h>
#include <stdarg.h>
#include <stdbool.h>
#include "../include/os.h"
#include "../include/pcb.h"
#include "../include/scheduler.h"
#include "../include/memory.h"
#include "../include/banker.h"
#include "../include/logger.h"
#include <gtk/gtkbutton.h>

// Global pointers to UI widgets for state management and updates
static GtkWidget *window;
static GtkWidget *Text_output;
static GtkWidget *quantum_entry;
static GtkWidget *draw_area;
static GtkWidget *gantt_frame;
static GtkWidget *memory_draw_area; 
static GtkTextBuffer *banker_dialog_buffer = NULL; 

static segment segs [MAX_SEGMENTS];
static int num_segments = 0;

// Utility to generate current system time for logging in the GUI
static char* get_timestamp() {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    static char timestamp[64]; 
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", t);
    return timestamp;
}

// Functions to redirect logic output to the GUI text view
static void append_text(const char *text) {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(Text_output));
    GtkTextIter end;
    gtk_text_buffer_get_end_iter(buffer, &end);
    gtk_text_buffer_insert(buffer, &end, text, -1);
    gtk_text_buffer_insert(buffer, &end, "\n", -1);
}

static void append_fmt(const char *format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    append_text(buffer);
}

static void append_output(const char *format, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    append_text(buffer);
}

// Callback function for banker.c to use for output in the GUI dialog
static void gui_banker_output_callback(const char *format, ...) {
    if (!banker_dialog_buffer) {
        va_list args;
        va_start(args, format);
        vfprintf(stderr, format, args);
        fprintf(stderr, "\n");
        va_end(args);
        return;
    }

    char buffer[1024]; 
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    gtk_text_buffer_insert_at_cursor(banker_dialog_buffer, buffer, -1);
    gtk_text_buffer_insert_at_cursor(banker_dialog_buffer, "\n", -1);
}

// Triggers a redraw of the custom memory gauge
static void update_memory_display() {
    if (memory_draw_area) {
        gtk_widget_queue_draw(memory_draw_area);
    }
}

// this is the Cairo Drawing for the Semi-Circle Memory Gauge. 
gboolean on_draw_memory_gauge(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    int width = gtk_widget_get_allocated_width(widget);
    int height = gtk_widget_get_allocated_height(widget);

    int total_memory = MEMORY_SIZE;
    int used = 0;
    pcb *processes = get_process_table();
    int count = get_process_count();

    for (int i = 0; i < count; i++) {
        if (processes[i].state != TERMINATED && processes[i].pid != -1) {
            used += processes[i].mem_size;
        }
    }
    int available = total_memory - used;
    double fraction = (total_memory > 0) ? (double)used / total_memory : 0.0;

    // Define gauge geometry (semi-circle for the memory status)
    double xc = width / 2.0;
    double yc = height - 20; 
    double radius = MIN(width / 2.0 - 20, height - 30);

    cairo_set_line_width(cr, 15.0);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_arc(cr, xc, yc, radius, G_PI, 2 * G_PI); // Semi-circle from left to right
    cairo_set_source_rgb(cr, 0.15, 0.15, 0.2); // Dark grayish-blue
    cairo_stroke(cr);

    if (fraction > 0.0) {
        cairo_arc(cr, xc, yc, radius, G_PI, G_PI + (G_PI * fraction));
        cairo_set_source_rgb(cr, 0.45, 0.40, 0.95); 
        cairo_stroke(cr);
    }

    cairo_set_source_rgb(cr, 0.36, 0.88, 0.90);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 20.0); 

    char text[64];
    cairo_text_extents_t extents;

    // Y offsets to stack the text neatly inside the arch
    double text_start_y = yc - (radius / 1.5); 

    snprintf(text, sizeof(text), "Total: %d KB", total_memory);
    cairo_text_extents(cr, text, &extents);
    cairo_move_to(cr, xc - (extents.width / 2), text_start_y);
    cairo_show_text(cr, text);

    snprintf(text, sizeof(text), "Used: %d KB", used);
    cairo_text_extents(cr, text, &extents);
    cairo_move_to(cr, xc - (extents.width / 2), text_start_y + 26);
    cairo_show_text(cr, text);

    snprintf(text, sizeof(text), "Available: %d KB", available);
    cairo_text_extents(cr, text, &extents);
    cairo_move_to(cr, xc - (extents.width / 2), text_start_y + 52);
    cairo_show_text(cr, text);

    return FALSE;
}

// Handles the creation of a new process via a modal dialog
static void view_processes_creation_dialog(){
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Create Process", GTK_WINDOW(window), GTK_DIALOG_MODAL, "Create", GTK_RESPONSE_ACCEPT, "Cancel", GTK_RESPONSE_REJECT, NULL);
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 5);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 10);

    GtkWidget *name_label = gtk_label_new("Process Name:");
    GtkWidget *name_entry = gtk_entry_new();
    GtkWidget *burst_label = gtk_label_new("Burst Time:");
    GtkWidget *burst_entry = gtk_entry_new();
    GtkWidget *priority_label = gtk_label_new("Priority:");
    GtkWidget *priority_entry = gtk_entry_new();
    GtkWidget *memory_label = gtk_label_new("Memory Size (KB):");
    GtkWidget *memory_entry = gtk_entry_new();

    gtk_grid_attach(GTK_GRID(grid), name_label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), name_entry, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), burst_label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), burst_entry, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), priority_label, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), priority_entry, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), memory_label, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), memory_entry, 1, 3, 1, 1);

    gtk_container_add(GTK_CONTAINER(content_area), grid);
    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        const char *name = gtk_entry_get_text(GTK_ENTRY(name_entry));
        int burst_time = atoi(gtk_entry_get_text(GTK_ENTRY(burst_entry)));
        int priority = atoi(gtk_entry_get_text(GTK_ENTRY(priority_entry)));
        int mem_size = atoi(gtk_entry_get_text(GTK_ENTRY(memory_entry)));

        if (strlen(name) == 0 || burst_time <= 0 || priority < 0 || mem_size <= 0) {
            GtkWidget *error_dialog = gtk_message_dialog_new(GTK_WINDOW(dialog), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Invalid input. Please enter valid values.");
            gtk_dialog_run(GTK_DIALOG(error_dialog));
            gtk_widget_destroy(error_dialog);
        } else {
            int pid = create_process(name, burst_time, priority, mem_size);
            if (pid != -1) {
                append_fmt("[%s] Process created: PID=%d, Name=%s, Burst=%d, Priority=%d, Memory=%d KB", get_timestamp(), pid, name, burst_time, priority, mem_size);
                update_memory_display();
            } else {
                GtkWidget *error_dialog = gtk_message_dialog_new(GTK_WINDOW(dialog), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Failed to create process. Not enough memory or process table full.");
                gtk_dialog_run(GTK_DIALOG(error_dialog));
                gtk_widget_destroy(error_dialog);
            }
        }
    }
    gtk_widget_destroy(dialog);
}

// UI Event Handlers (Signals)
void on_create_process_clicked(GtkButton *button, gpointer user_data) {
    view_processes_creation_dialog();
}

void on_list_processes_clicked(GtkButton *button, gpointer user_data) {
    append_fmt("[%s] Listing processes:", get_timestamp()); 
    pcb *processes = get_process_table();

    append_text("PID | Name           | Priority | Burst Time | Remaining Time | State     | Memory (KB)");
    append_text("----|----------------|----------|------------|----------------|-----------|-------------");
    
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].pid != 0) {
            const char *state_str;
            switch (processes[i].state) {
                case NEW: state_str = "NEW"; break;
                case READY: state_str = "READY"; break;
                case RUNNING: state_str = "RUNNING"; break;
                case WAITING: state_str = "WAITING"; break;
                case TERMINATED: state_str = "TERMINATED"; break;
                default: state_str = "UNKNOWN"; break;
            }
            append_fmt("%3d | %-14s | %8d | %10d | %14d | %-9s | %11d", processes[i].pid, processes[i].name, processes[i].priority, processes[i].burst_time, processes[i].remaining_time, state_str, processes[i].mem_size);
        }
    }
    update_memory_display();
}

void on_fcfs_clicked(GtkButton *button, gpointer user_data) {
    num_segments = scheduler_selection(SCHED_FCFS, 0, segs, MAX_SEGMENTS);
    gtk_widget_show_all(gantt_frame);
    gtk_widget_queue_draw(draw_area);
    append_fmt("[%s] Starting First-Come-First-Serve scheduling", get_timestamp());
    update_memory_display();
}

void on_rr_clicked(GtkButton *button, gpointer user_data) {
    int time_quantum = atoi(gtk_entry_get_text(GTK_ENTRY(quantum_entry)));
    if (time_quantum <= 0) {
        GtkWidget *error_dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Invalid time quantum. Please enter a positive integer.");
        gtk_dialog_run(GTK_DIALOG(error_dialog));
        gtk_widget_destroy(error_dialog);
        return;
    }
    append_fmt("[%s] Starting Round Robin scheduling with time quantum: %d", get_timestamp(), time_quantum);
    num_segments = scheduler_selection(SCHED_ROUNDROBIN, time_quantum, segs, MAX_SEGMENTS);
    gtk_widget_show_all(gantt_frame);
    gtk_widget_queue_draw(draw_area);
    update_memory_display();
}

void on_priority_clicked(GtkButton *button, gpointer user_data) {
    num_segments = scheduler_selection(SCHED_PRIORITY, 0, segs, MAX_SEGMENTS);
    gtk_widget_show_all(gantt_frame);
    gtk_widget_queue_draw(draw_area);
    append_fmt("[%s] Starting Priority scheduling", get_timestamp());
    update_memory_display();
}

void on_delete_process_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Delete Process", GTK_WINDOW(window), GTK_DIALOG_MODAL, "Delete", GTK_RESPONSE_ACCEPT, "Cancel", GTK_RESPONSE_REJECT, NULL);
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(hbox), 10);
    gtk_box_pack_start(GTK_BOX(content_area), hbox, TRUE, TRUE, 5);

    gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new("PID:"), FALSE, FALSE, 5);
    GtkWidget *pid_input = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(pid_input), "Enter PID to delete");
    gtk_box_pack_start(GTK_BOX(hbox), pid_input, TRUE, TRUE, 5);

    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        int pid = atoi(gtk_entry_get_text(GTK_ENTRY(pid_input)));
        if (pid <= 0) {
            GtkWidget *error_dialog = gtk_message_dialog_new(GTK_WINDOW(dialog), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Invalid PID. Please enter a valid positive integer PID.");
            gtk_dialog_run(GTK_DIALOG(error_dialog));
            gtk_widget_destroy(error_dialog);
        } else {
            delete_process(pid);
            append_fmt("[%s] Deleted process with PID: %d", get_timestamp(), pid);
            update_memory_display();
        }
    }
    gtk_widget_destroy(dialog);
}

void on_show_memory_clicked(GtkButton *button, gpointer user_data) {
    append_fmt("[%s] Showing memory map", get_timestamp());
    append_text("PID   | Address    | Size (KB)  | Status");
    append_text("------|------------|------------|--------------");
    
    pcb *processes = get_process_table();
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].pid != 0 && processes[i].state != TERMINATED) {
            append_fmt("%-5d | %-10d | %-10d | %-12s", 
                       processes[i].pid,
                       processes[i].mem_address, 
                       processes[i].mem_size, 
                       "Allocated");
        }
    }
    update_memory_display();
}

void on_show_logs_clicked(GtkButton *button, gpointer user_data) {
    append_fmt("[%s] Showing system logs", get_timestamp());
    FILE *log_file = fopen("OS_Logs.txt", "r");
    if (log_file == NULL) {
        GtkWidget *error_dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Error opening system.log");
        gtk_dialog_run(GTK_DIALOG(error_dialog));
        gtk_widget_destroy(error_dialog);
        return;
    }
    char line[256];
    while (fgets(line, sizeof(line), log_file)) {
        append_output("%s", line);
    }
    fclose(log_file);
    update_memory_display();
}

void on_initalize_resources_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Initialize Resources", GTK_WINDOW(window), GTK_DIALOG_MODAL, "Init", GTK_RESPONSE_ACCEPT, "Cancel", GTK_RESPONSE_REJECT, NULL);
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(hbox), 20);
    gtk_box_pack_start(GTK_BOX(content_area), hbox, TRUE, TRUE, 0);

    gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new("Total Units:"), FALSE, FALSE, 0);
    GtkWidget *entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry), "10");
    gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 0);

    gtk_widget_show_all(dialog);
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        int total = atoi(gtk_entry_get_text(GTK_ENTRY(entry)));
        resource_init(total, gui_banker_output_callback); 
        append_fmt("[%s] Initialized system with %d total resource units.", get_timestamp(), total);
    }
    gtk_widget_destroy(dialog);
    update_memory_display();
}

void on_set_max_claim_clicked(GtkButton *button, gpointer user_data) {
    pcb *processes = get_process_table();
    int active_count = 0;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].pid != 0 && processes[i].state != TERMINATED) {
            active_count++;
        }
    }

    if (active_count == 0) {
        GtkWidget *info_dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, "There are no processes in the system.");
        gtk_dialog_run(GTK_DIALOG(info_dialog));
        gtk_widget_destroy(info_dialog);
        return;
    }

    GtkWidget *dialog = gtk_dialog_new_with_buttons("Set Max Claims", GTK_WINDOW(window), GTK_DIALOG_MODAL, "Save All", GTK_RESPONSE_ACCEPT, "Cancel", GTK_RESPONSE_REJECT, NULL);
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *grid = gtk_grid_new();
    gtk_box_pack_start(GTK_BOX(content_area), grid, TRUE, TRUE, 10);

    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("PID"), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Process Name"), 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Max Claim"), 2, 0, 1, 1);

    typedef struct {
        int pid;
        GtkWidget *max_entry;
    } row_data;
    row_data *rows = malloc(sizeof(row_data) * active_count);

    int row_idx = 1;
    int active_idx = 0;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].pid != 0 && processes[i].state != TERMINATED) {
            char pid_text[16];
            snprintf(pid_text, sizeof(pid_text), "%d", processes[i].pid);
            gtk_grid_attach(GTK_GRID(grid), gtk_label_new(pid_text), 0, row_idx, 1, 1);
            gtk_grid_attach(GTK_GRID(grid), gtk_label_new(processes[i].name), 1, row_idx, 1, 1);
            
            rows[active_idx].pid = processes[i].pid;
            rows[active_idx].max_entry = gtk_entry_new();
            gtk_entry_set_text(GTK_ENTRY(rows[active_idx].max_entry), "0");
            gtk_grid_attach(GTK_GRID(grid), rows[active_idx].max_entry, 2, row_idx, 1, 1);

            row_idx++;
            active_idx++;
        }
    }

    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        for (int i = 0; i < active_count; i++) {
            int max = atoi(gtk_entry_get_text(GTK_ENTRY(rows[i].max_entry)));
            if (set_process_max_claim(rows[i].pid, max) == 0) {
                append_fmt("[%s] Max claim set for PID %d: %d", get_timestamp(), rows[i].pid, max);
            } else {
                GtkWidget *error_dialog = gtk_message_dialog_new(GTK_WINDOW(dialog), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Failed to set claim for PID %d. Ensure resources are initialized and claims do not exceed totals.", rows[i].pid);
                gtk_dialog_run(GTK_DIALOG(error_dialog));
                gtk_widget_destroy(error_dialog);
            }
        }
        update_memory_display();
    }
    free(rows);
    gtk_widget_destroy(dialog);
}

void on_request_resources_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget *banker_output_dialog = gtk_dialog_new_with_buttons("Banker's Algorithm Simulation Output", GTK_WINDOW(window), GTK_DIALOG_MODAL, "Close", GTK_RESPONSE_CLOSE, NULL);
    gtk_window_set_default_size(GTK_WINDOW(banker_output_dialog), 600, 400);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(banker_output_dialog));
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_hexpand(scrolled_window, TRUE);
    gtk_widget_set_vexpand(scrolled_window, TRUE);
    gtk_container_add(GTK_CONTAINER(content_area), scrolled_window);

    GtkWidget *text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(text_view), TRUE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD_CHAR); 

    banker_dialog_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);
    gtk_text_buffer_set_text(banker_dialog_buffer, "", -1);

    gui_banker_output_callback("[%s] Starting Banker's Algorithm simulation...", get_timestamp());

    bool safe = run_banker_simulation(gui_banker_output_callback);

    GtkWidget *msg_dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, safe ? GTK_MESSAGE_INFO : GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
                                                   safe ? "System is in a SAFE STATE. Check simulation output for sequence." : "UNSAFE STATE DETECTED! Request denied to avoid deadlock. Check simulation output for details.");
    gtk_dialog_run(GTK_DIALOG(msg_dialog));
    gtk_widget_destroy(msg_dialog);
    append_output(safe ? "System is SAFE. Simulation output displayed in pop-up." : "System is UNSAFE. Potential deadlock detected. Simulation output displayed in pop-up.");

    gtk_widget_show_all(banker_output_dialog);
    gtk_dialog_run(GTK_DIALOG(banker_output_dialog));
    gtk_widget_destroy(banker_output_dialog);
    banker_dialog_buffer = NULL; 
}

void on_toggle_gantt_clicked(GtkButton *button, gpointer user_data) {
    if (gtk_widget_get_visible(gantt_frame)) {
        gtk_widget_hide(gantt_frame);
    } else {
        gtk_widget_show_all(gantt_frame);
    }
}

void on_terminate_system_clicked(GtkButton *button, gpointer user_data) {
    log_event("Mini OS Dashboard: Manual termination requested.");
    log_dump(); 
    gtk_main_quit();
}

gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    int width = gtk_widget_get_allocated_width(widget);
    int height = gtk_widget_get_allocated_height(widget);

    cairo_set_source_rgb(cr, 0.05, 0.05, 0.05);
    cairo_paint(cr);

    int segment_width = width / num_segments;
    int total_time = 0;
    for (int i = 0; i < num_segments; i++) {
        total_time += segs[i].duration;
    }
    double scale = (total_time > 0) ? (double)width / total_time : 1.0;

double color_palette[10][3] = {
    {0.50, 0.48, 0.65}, // Dull Purple-Blue
    {0.45, 0.55, 0.65}, // Dull Cyan-Blue
    {0.48, 0.65, 0.55}, // Dull Mint
    {0.75, 0.55, 0.55}, // Dull Red
    {0.70, 0.60, 0.50}, // Dull Orange
    {0.55, 0.50, 0.65}, // Dull Lavender
    {0.50, 0.55, 0.65}, // Dull Steel Blue
    {0.70, 0.50, 0.60}, // Dull Pink
    {0.55, 0.65, 0.55}, // Dull Sage
    {0.65, 0.65, 0.45}  // Dull Yellow
};

    int color_index = 0;
    int y = height / 4;
    for (int i = 0; i < num_segments; i++) {
        double seg_width = segs[i].duration * scale;
        double x = segs[i].start_time * scale;
        int color_idx = segs[i].pid % 10;
        cairo_set_source_rgb(cr, color_palette[color_idx][0], color_palette[color_idx][1], color_palette[color_idx][2]);
        cairo_rectangle(cr, x, y, seg_width, height / 4);
        cairo_fill(cr);
        color_index = (color_index + 1) % 10;

        cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 12);
        char pid_str[16];
        snprintf(pid_str, sizeof(pid_str), "%d", segs[i].pid);
        cairo_move_to(cr, x + seg_width / 2 - 10, y + height / 8);
        cairo_show_text(cr, pid_str);
    }
    return FALSE;
}

static gboolean on_draw_circle_icon(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    GdkPixbuf *pixbuf = g_object_get_data(G_OBJECT(widget), "pixbuf");
    if (!pixbuf) return FALSE;

    int w = gtk_widget_get_allocated_width(widget);
    int h = gtk_widget_get_allocated_height(widget);

    // this will create a circular clipping region
    cairo_arc(cr, w / 2.0, h / 2.0, MIN(w, h) / 2.0, 0, 2 * G_PI);
    cairo_clip(cr);

    // this will Scale the image to fit the widget size
    int img_w = gdk_pixbuf_get_width(pixbuf);
    int img_h = gdk_pixbuf_get_height(pixbuf);
    double scale = MIN((double)w / img_w, (double)h / img_h);

    cairo_scale(cr, scale, scale);

    // this will Center the image
    double x_offset = (w / scale - img_w) / 2.0;
    double y_offset = (h / scale - img_h) / 2.0;

    gdk_cairo_set_source_pixbuf(cr, pixbuf, x_offset, y_offset);
    cairo_paint(cr);

    return FALSE;
}

static GtkWidget* create_image_from_file(const char *filename) {
    GError *error = NULL;
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(filename, &error);
    
    if (!pixbuf) {
        fprintf(stderr, "Could not load image '%s': %s\n", filename, error ? error->message : "Unknown error");
        if (error) g_error_free(error);
        return gtk_image_new();
    }

    // Create a DrawingArea widget which we will use to draw our circle
    GtkWidget *draw_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(draw_area, 40, 40); // Sets icon size

    // Associate the pixbuf with this widget so the draw function can use it
    g_object_set_data_full(G_OBJECT(draw_area), "pixbuf", pixbuf, g_object_unref);
    
    // Connect the "draw" signal
    g_signal_connect(draw_area, "draw", G_CALLBACK(on_draw_circle_icon), NULL);

    return draw_area;
}


int main(int argc, char *argv[]) {
    init_system();
    memory_init();
    log_init();
    log_event("Mini OS initialized successfully.");

    gtk_init(&argc, &argv);

    
    GtkCssProvider *provider = gtk_css_provider_new();
    const char *css =
        "window, box, grid { background-color: #0F0F14; color: #E0E0E0; }" 
        ".sidebar-custom { background-color: #0F0F14; border: none; }" 
        ".sidebar-btn { background-color: transparent; border: none; border-radius: 20px; min-width: 40px; min-height: 40px; padding: 0px; background-image: none; box-shadow: none; }" 
        ".sidebar-btn:hover { background-color: rgba(255, 255, 255, 0.1); }" 
        ".sidebar-btn:active { background-color: rgba(255, 255, 255, 0.2); }" 
        "frame { background-color: #171720; border: 1px solid #252530; margin: 5px; border-radius: 12px; }" 
        "frame label { color: #5CE1E6; font-size: 18px; font-weight: bold; margin-bottom: 4px; padding: 5px; }" 
        "button { background-image: none; background-color: #1D1D26; color: #FFFFFF; border: 1px solid #303040; border-radius: 10px; padding: 8px; }" 
        "button:hover { background-color: #2A2A38; }" 
        "button:active { background-color: #15151A; }" 
        "entry { background-color: #0F0F14; color: #FFFFFF; border: 1px solid #303040; border-radius: 8px; }" 
        "textview text { background-color: #07070A; color: #A0A0B0; font-family: monospace; }" 
        "separator { border-color: #252530; }";


    gtk_css_provider_load_from_data(provider, css, -1, NULL);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
    g_object_unref(provider);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "MINI_OS Dashboard");
    gtk_window_set_default_size(GTK_WINDOW(window), 1000, 850);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // --- MAIN LAYOUT ---
    GtkWidget *main_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add(GTK_CONTAINER(window), main_hbox);

    // --- LEFT SIDEBAR ---
    GtkWidget *sidebar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    
    GtkStyleContext *context = gtk_widget_get_style_context(sidebar);
    gtk_style_context_add_class(context, "sidebar-custom");
    gtk_widget_set_app_paintable(sidebar, TRUE); 

    gtk_widget_set_size_request(sidebar, 100, -1); 
    gtk_container_set_border_width(GTK_CONTAINER(sidebar), 10);
    gtk_box_pack_start(GTK_BOX(main_hbox), sidebar, FALSE, FALSE, 0);

    GtkWidget *home_btn = gtk_button_new();
    GtkWidget *home_img = create_image_from_file("assets/home_icon.png");
    gtk_button_set_image(GTK_BUTTON(home_btn), home_img);
    gtk_style_context_add_class(gtk_widget_get_style_context(home_btn), "sidebar-btn"); // Apply circular style
    gtk_box_pack_start(GTK_BOX(sidebar), home_btn, FALSE, FALSE, 5);

    GtkWidget *chart_btn = gtk_button_new();
    GtkWidget *chart_img = create_image_from_file("assets/graph_icon.png");
    gtk_button_set_image(GTK_BUTTON(chart_btn), chart_img);
    gtk_style_context_add_class(gtk_widget_get_style_context(chart_btn), "sidebar-btn"); // Apply circular style
    g_signal_connect(chart_btn, "clicked", G_CALLBACK(on_toggle_gantt_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(sidebar), chart_btn, FALSE, FALSE, 5);

    GtkWidget *sidebar_spacer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(sidebar), sidebar_spacer, TRUE, TRUE, 0);

    GtkWidget *power_btn = gtk_button_new();
    GtkWidget *power_img = create_image_from_file("assets/power_icon.png");
    gtk_button_set_image(GTK_BUTTON(power_btn), power_img);
    gtk_style_context_add_class(gtk_widget_get_style_context(power_btn), "sidebar-btn"); // Apply circular style
    g_signal_connect(power_btn, "clicked", G_CALLBACK(on_terminate_system_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(sidebar), power_btn, FALSE, FALSE, 5);

    // --- MAIN CONTENT AREA ---
    GtkWidget *content_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(content_vbox), 15);
    gtk_box_pack_start(GTK_BOX(main_hbox), content_vbox, TRUE, TRUE, 0);

    // --- HEADER SECTION ---
    GtkWidget *header_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(content_vbox), header_vbox, FALSE, FALSE, 5);

    // Title Row
    GtkWidget *title_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    
    GtkWidget *dash_label = gtk_label_new("<span font_desc='22' weight='bold' foreground='#5CE1E6'>MINI OS DASHBOARD</span>");
    gtk_label_set_use_markup(GTK_LABEL(dash_label), TRUE);
    
    GtkWidget *welcome_label = gtk_label_new("<span font_desc='22' weight='bold' foreground='#5CE1E6'>WELCOME</span>");
    gtk_label_set_use_markup(GTK_LABEL(welcome_label), TRUE);
    
    gtk_box_pack_start(GTK_BOX(title_hbox), dash_label, FALSE, FALSE, 0);
    GtkWidget *title_spacer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(title_hbox), title_spacer, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(title_hbox), welcome_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(header_vbox), title_hbox, FALSE, FALSE, 0);

    // Divider Line
    GtkWidget *header_line = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(header_vbox), header_line, FALSE, FALSE, 5);

    // Sub-Header Row (Home, Create Button, Welcome Icon)
    GtkWidget *sub_header_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);

    GtkWidget *home_label = gtk_label_new("<span size='large' weight='bold' foreground='#5CE1E6'>HOME</span>");
    gtk_label_set_use_markup(GTK_LABEL(home_label), TRUE);
    gtk_box_pack_start(GTK_BOX(sub_header_hbox), home_label, FALSE, FALSE, 0);

    GtkWidget *create_btn = gtk_button_new_with_label("+ Create Process");
    g_signal_connect(create_btn, "clicked", G_CALLBACK(on_create_process_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(sub_header_hbox), create_btn, FALSE, FALSE, 0); 

    GtkWidget *sub_spacer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(sub_header_hbox), sub_spacer, TRUE, TRUE, 0);

    GtkWidget *welcome_icon = create_image_from_file("assets/dashboard_icon.png");
    gtk_box_pack_start(GTK_BOX(sub_header_hbox), welcome_icon, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(header_vbox), sub_header_hbox, FALSE, FALSE, 0);

    // --- MIDDLE SECTION (3 Columns) ---
    GtkWidget *mid_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);
    gtk_box_pack_start(GTK_BOX(content_vbox), mid_hbox, FALSE, FALSE, 10);

    // Column 1: System Operations
    GtkWidget *operations_frame = gtk_frame_new("System Operation");
    GtkWidget *operations_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(operations_vbox), 10);
    gtk_container_add(GTK_CONTAINER(operations_frame), operations_vbox);
    gtk_box_pack_start(GTK_BOX(mid_hbox), operations_frame, TRUE, TRUE, 0);
    
    GtkWidget *btn;
    btn = gtk_button_new_with_label("List Processes");
    g_signal_connect(btn, "clicked", G_CALLBACK(on_list_processes_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(operations_vbox), btn, FALSE, FALSE, 5);
    btn = gtk_button_new_with_label("Show Memory Map");
    g_signal_connect(btn, "clicked", G_CALLBACK(on_show_memory_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(operations_vbox), btn, FALSE, FALSE, 5);
    btn = gtk_button_new_with_label("Show Logs");
    g_signal_connect(btn, "clicked", G_CALLBACK(on_show_logs_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(operations_vbox), btn, FALSE, FALSE, 5);
    btn = gtk_button_new_with_label("Delete Process");
    g_signal_connect(btn, "clicked", G_CALLBACK(on_delete_process_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(operations_vbox), btn, FALSE, FALSE, 5);

    // Column 2: CPU Scheduling
    GtkWidget *scheduler_frame = gtk_frame_new("CPU Scheduling");
    GtkWidget *scheduler_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(scheduler_vbox), 10);
    gtk_container_add(GTK_CONTAINER(scheduler_frame), scheduler_vbox);
    gtk_box_pack_start(GTK_BOX(mid_hbox), scheduler_frame, TRUE, TRUE, 0);

    btn = gtk_button_new_with_label("First Come First Serve");
    g_signal_connect(btn, "clicked", G_CALLBACK(on_fcfs_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(scheduler_vbox), btn, FALSE, FALSE, 5);
    btn = gtk_button_new_with_label("Priority Scheduling");
    g_signal_connect(btn, "clicked", G_CALLBACK(on_priority_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(scheduler_vbox), btn, FALSE, FALSE, 5);
    
    GtkWidget *rr_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  
    btn = gtk_button_new_with_label("Round Robin");
    g_signal_connect(btn, "clicked", G_CALLBACK(on_rr_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(rr_hbox), btn, TRUE, TRUE, 0);
    quantum_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(quantum_entry), "Quantum");
    gtk_widget_set_size_request(quantum_entry, 80, -1);
    gtk_box_pack_start(GTK_BOX(rr_hbox), quantum_entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(scheduler_vbox), rr_hbox, FALSE, FALSE, 5);

    GtkWidget *deadlock_frame = gtk_frame_new("Bankers Algorithm");
    GtkWidget *deadlock_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(deadlock_vbox), 10);
    gtk_container_add(GTK_CONTAINER(deadlock_frame), deadlock_vbox);
    gtk_box_pack_start(GTK_BOX(mid_hbox), deadlock_frame, TRUE, TRUE, 0);
    
    btn = gtk_button_new_with_label("Initialize");
    g_signal_connect(btn, "clicked", G_CALLBACK(on_initalize_resources_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(deadlock_vbox), btn, FALSE, FALSE, 5);
    btn = gtk_button_new_with_label("Set Max Claim"); 
    g_signal_connect(btn, "clicked", G_CALLBACK(on_set_max_claim_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(deadlock_vbox), btn, FALSE, FALSE, 5);
    btn = gtk_button_new_with_label("Run Banker's Simulation");
    g_signal_connect(btn, "clicked", G_CALLBACK(on_request_resources_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(deadlock_vbox), btn, FALSE, FALSE, 5);

    // --- GANTT CHART (Hidden by default, triggered by sidebar icon) ---
    gantt_frame = gtk_frame_new("Gantt Chart");
    draw_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(draw_area, 800, 60); 
    gtk_container_add(GTK_CONTAINER(gantt_frame), draw_area);
    g_signal_connect(draw_area, "draw", G_CALLBACK(on_draw), NULL);
    gtk_box_pack_start(GTK_BOX(content_vbox), gantt_frame, FALSE, FALSE, 5);

    // --- BOTTOM SECTION  ---
    GtkWidget *bottom_grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(bottom_grid), 15);
    gtk_grid_set_column_homogeneous(GTK_GRID(bottom_grid), TRUE); // Forces columns to share equal space
    gtk_box_pack_start(GTK_BOX(content_vbox), bottom_grid, TRUE, TRUE, 0);

    // System Output Panel (Spans 2 columns)
    GtkWidget *text_frame = gtk_frame_new("System Output");
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(text_frame), scrolled_window);
    gtk_widget_set_hexpand(text_frame, TRUE);
    gtk_widget_set_vexpand(text_frame, TRUE);
    gtk_grid_attach(GTK_GRID(bottom_grid), text_frame, 0, 0, 2, 1); // Spans 2 out of 3
    
    Text_output = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(Text_output), FALSE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(Text_output), TRUE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(Text_output), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(Text_output), GTK_WRAP_NONE);
    gtk_container_add(GTK_CONTAINER(scrolled_window), Text_output);

    // Memory Status Panel 
    GtkWidget *mem_frame = gtk_frame_new("Memory Status");
    memory_draw_area = gtk_drawing_area_new();
    gtk_widget_set_hexpand(mem_frame, TRUE);
    gtk_widget_set_vexpand(mem_frame, TRUE);
    gtk_container_add(GTK_CONTAINER(mem_frame), memory_draw_area);
    g_signal_connect(memory_draw_area, "draw", G_CALLBACK(on_draw_memory_gauge), NULL);
    gtk_grid_attach(GTK_GRID(bottom_grid), mem_frame, 2, 0, 1, 1); // Spans 1 out of 3

    // Start
    append_fmt("[%s] Mini OS Dashboard initialized successfully.", get_timestamp());
    log_event("Mini OS Dashboard initialized successfully.");
    
    gtk_widget_show_all(window);
    gtk_widget_hide(gantt_frame); 
    
    gtk_main();
    return 0;
}