#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#define MAX_FILES 1024

// ஃபைல்களை ஸ்டோர் செய்ய ஒரு Structure
typedef struct {
    char name[256];
    int is_dir;
} FileEntry;

FileEntry files[MAX_FILES];
int file_count = 0;
int selected_idx = 0;

// டெர்மினலை Raw Mode-க்கு மாற்றும் ஃபங்ஷன் (Enter, Arrow keys-ஐ பிடிக்க)
struct termios orig_termios;
void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON); // Enter அடிக்காமல் பட்டனைப் பிடிக்க
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

// ஃபைல்களைப் படிக்கும் ஃபங்ஷன்
void load_files(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) return;
    struct dirent *entry;
    file_count = 0;
    while ((entry = readdir(dir)) != NULL && file_count < MAX_FILES) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        
        struct stat statbuf;
        stat(entry->d_name, &statbuf);
        
        strncpy(files[file_count].name, entry->d_name, 255);
        files[file_count].is_dir = S_ISDIR(statbuf.st_mode);
        file_count++;
    }
    closedir(dir);
}

// திரையை வரையும் ஃபங்ஷன்
void draw_tree() {
    printf("\033[2J\033[H"); // திரையை க்ளீன் செய்து கர்சரை மேலே கொண்டு செல்ல
    printf("\033[1;36m[ BDH Workspace: Interactive Tree ]\033[0m\r\n");
    printf("│\r\n");
    
    for (int i = 0; i < file_count; i++) {
        // செலக்ட் ஆன வரியை Highlight செய்வது
        if (i == selected_idx) {
            printf("\033[1;32m  > \033[7m"); // பச்சை நிற ' > ' மற்றும் Inverted Colors
        } else {
            printf("    ");
        }

        printf("├── ");
        
        if (files[i].is_dir) {
            printf("\033[1;36m%s/\033[0m", files[i].name); // போல்டர் - Blue
        } else {
            printf("%s", files[i].name); // ஃபைல் - White
        }

        if (i == selected_idx) {
            printf("\033[0m"); // Highlight-ஐ முடிப்பது
        }
        printf("\r\n");
    }
    printf("\r\n[UP/DOWN: Navigate] | [ENTER: Open] | [Q: Quit]\r\n");
}

int main() {
    load_files(".");
    enable_raw_mode();
    
    while (1) {
        draw_tree();
        
        char c;
        read(STDIN_FILENO, &c, 1);
        
        if (c == 'q') {
            break; // Q அழுத்தினால் வெளியேற
        } 
        else if (c == 'w' || c == 'k' || c == 'A') { // Up (A is part of arrow key sequence)
            if (selected_idx > 0) selected_idx--;
        } 
        else if (c == 's' || c == 'j' || c == 'B') { // Down (B is part of arrow key sequence)
            if (selected_idx < file_count - 1) selected_idx++;
        } 
        else if (c == '\n' || c == '\r') { // ENTER கீ
            if (!files[selected_idx].is_dir) {
                // ஃபைலாக இருந்தால் bdh-edit-ஐ ஓபன் செய்வது
                disable_raw_mode(); // டெர்மினலை பழைய நிலைக்கு மாற்றிவிட்டு ஓபன் செய்ய வேண்டும்
                printf("\033[2J\033[H"); 
                
                char cmd[512];
                snprintf(cmd, sizeof(cmd), "bdh-edit %s", files[selected_idx].name);
                system(cmd); // bdh-edit-ஐ இயக்குதல்
                
                enable_raw_mode(); // திரும்பி வந்ததும் மறுபடியும் Raw Mode
            }
        }
    }
    
    printf("\033[2J\033[H");
    return 0;
}
