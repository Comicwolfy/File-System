#include <stdint.h>
#include <stddef.h>
#include "base_kernel.h"

#define MAX_FILES 16
#define MAX_FILENAME_LEN 32

typedef struct InMemFile {
    char name[MAX_FILENAME_LEN];
    uint32_t size;
    char* data;
    int active;
} InMemFile;

static int ramdisk_ext_id = -1;
static InMemFile files[MAX_FILES];

static int find_free_file_slot() {
    for (int i = 0; i < MAX_FILES; i++) {
        if (!files[i].active) {
            return i;
        }
    }
    return -1;
}

static int find_file_by_name(const char* name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].active && strcmp(files[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

void cmd_touch(const char* args) {
    const char* filename = args;
    if (strlen(filename) == 0 || strlen(filename) >= MAX_FILENAME_LEN) {
        terminal_writestring("RAMFS: Usage: touch <filename> (max 31 chars)\n");
        return;
    }

    if (find_file_by_name(filename) != -1) {
        terminal_writestring("RAMFS: Error: File '");
        terminal_writestring(filename);
        terminal_writestring("' already exists.\n");
        return;
    }

    int slot = find_free_file_slot();
    if (slot == -1) {
        terminal_writestring("RAMFS: Error: Maximum number of files reached.\n");
        return;
    }

    strcpy(files[slot].name, filename);
    files[slot].size = 0;
    files[slot].data = NULL;
    files[slot].active = 1;

    terminal_writestring("RAMFS: Created file '");
    terminal_writestring(filename);
    terminal_writestring("'.\n");
}

void cmd_echo_to_file(const char* args) {
    const char* filename_start = args;
    const char* delimiter = strstr(args, " > ");
    if (!delimiter) {
        terminal_writestring("RAMFS: Usage: echo <message> > <filename>\n");
        return;
    }

    int msg_len = delimiter - filename_start;
    char message[msg_len + 1];
    strncpy(message, filename_start, msg_len);
    message[msg_len] = '\0';

    const char* filename = delimiter + 3; // Skip " > "

    if (strlen(filename) == 0 || strlen(filename) >= MAX_FILENAME_LEN) {
        terminal_writestring("RAMFS: Usage: echo <message> > <filename> (max 31 chars)\n");
        return;
    }

    int slot = find_file_by_name(filename);
    if (slot == -1) {
        terminal_writestring("RAMFS: Error: File '");
        terminal_writestring(filename);
        terminal_writestring("' does not exist. Use 'touch' first.\n");
        return;
    }

    if (files[slot].data) {
        kfree(files[slot].data);
    }

    files[slot].size = strlen(message);
    files[slot].data = (char*)kmalloc(files[slot].size + 1);
    if (!files[slot].data) {
        terminal_writestring("RAMFS: Error: Failed to allocate memory for file content.\n");
        files[slot].size = 0;
        return;
    }
    strcpy(files[slot].data, message);

    terminal_writestring("RAMFS: Wrote ");
    char num_str[10]; int_to_str(files[slot].size, num_str); terminal_writestring(num_str);
    terminal_writestring(" bytes to '");
    terminal_writestring(filename);
    terminal_writestring("'.\n");
}

void cmd_ls(const char* args) {
    terminal_writestring("RAMFS: Files in memory:\n");
    char size_str[10];
    int found_any = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].active) {
            terminal_writestring("  ");
            terminal_writestring(files[i].name);
            terminal_writestring(" (");
            int_to_str(files[i].size, size_str); terminal_writestring(size_str);
            terminal_writestring(" bytes)\n");
            found_any = 1;
        }
    }
    if (!found_any) {
        terminal_writestring("  (No files found)\n");
    }
}

void cmd_cat(const char* args) {
    const char* filename = args;
    if (strlen(filename) == 0) {
        terminal_writestring("RAMFS: Usage: cat <filename>\n");
        return;
    }

    int slot = find_file_by_name(filename);
    if (slot == -1) {
        terminal_writestring("RAMFS: Error: File '");
        terminal_writestring(filename);
        terminal_writestring("' not found.\n");
        return;
    }

    if (files[slot].size == 0 || files[slot].data == NULL) {
        terminal_writestring("RAMFS: File '");
        terminal_writestring(filename);
        terminal_writestring("' is empty.\n");
        return;
    }

    terminal_writestring("--- Content of '");
    terminal_writestring(filename);
    terminal_writestring("' ---\n");
    terminal_writestring(files[slot].data);
    terminal_writestring("\n-----------------------------------\n");
}

void cmd_rm(const char* args) {
    const char* filename = args;
    if (strlen(filename) == 0) {
        terminal_writestring("RAMFS: Usage: rm <filename>\n");
        return;
    }

    int slot = find_file_by_name(filename);
    if (slot == -1) {
        terminal_writestring("RAMFS: Error: File '");
        terminal_writestring(filename);
        terminal_writestring("' not found.\n");
        return;
    }

    if (files[slot].data) {
        kfree(files[slot].data);
        files[slot].data = NULL;
    }
    files[slot].active = 0;
    files[slot].size = 0;
    files[slot].name[0] = '\0';

    terminal_writestring("RAMFS: Deleted file '");
    terminal_writestring(filename);
    terminal_writestring("'.\n");
}

int ramdisk_fs_extension_init(void) {
    terminal_writestring("RAMFS: In-Memory File System Extension Initializing...\n");

    for (int i = 0; i < MAX_FILES; i++) {
        files[i].active = 0;
        files[i].data = NULL;
        files[i].name[0] = '\0';
    }

    register_command("touch", cmd_touch, "Create an empty file in RAMFS", ramdisk_ext_id);
    register_command("echo", cmd_echo_to_file, "Write text to a file (e.g., echo hello > file.txt)", ramdisk_ext_id);
    register_command("ls", cmd_ls, "List files in RAMFS", ramdisk_ext_id);
    register_command("cat", cmd_cat, "Display file content from RAMFS", ramdisk_ext_id);
    register_command("rm", cmd_rm, "Delete a file from RAMFS", ramdisk_ext_id);

    terminal_writestring("RAMFS: Extension Initialized. Use 'ls', 'touch', 'echo', 'cat', 'rm'.\n");
    return 0;
}

void ramdisk_fs_extension_cleanup(void) {
    terminal_writestring("RAMFS: Cleaning up In-Memory File System...\n");
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].active) {
            if (files[i].data) {
                kfree(files[i].data);
            }
            files[i].active = 0;
            files[i].data = NULL;
            files[i].name[0] = '\0';
        }
    }
    terminal_writestring("RAMFS: Cleanup complete.\n");
}

__attribute__((section(".ext_register_fns")))
void __ramdisk_fs_auto_register(void) {
    ramdisk_ext_id = register_extension("RAMFS", "1.0",
                                        ramdisk_fs_extension_init,
                                        ramdisk_fs_extension_cleanup);
    if (ramdisk_ext_id >= 0) {
        load_extension(ramdisk_ext_id);
    } else {
        terminal_writestring("Failed to register RAMFS Extension (auto)!\n");
    }
}
