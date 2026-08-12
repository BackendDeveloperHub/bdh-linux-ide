#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h> // டெர்மினல் அளவைக் கண்டுபிடிக்க இது அவசியம்

#define MAX_FILES 1024

typedef struct {
    char name[256];
    int is_dir;
} FileEntry;

FileEntry files[MAX_FILES];
int file_count = 0;
int selected_idx = 0;
int window_start = 0; // ஸ்க்ரோலிங் வியூபோர்ட்டின் தொடக்கம்

struct termios orig_termios;

void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON); 
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void load_files(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) return;
    struct dirent *entry;
    file_count = 0;
    window_start = 0; // புதிய ஃபோல்டருக்குப் போகும்போது வியூபோர்ட்டை ரீசெட் செய்ய
    
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

void draw_tree() {
    // 1. டெர்மினலின் உயரத்தைக் (Rows) கண்டுபிடிக்கிறோம்
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int term_rows = w.ws_row;
    
    // Header மற்றும் Footer-க்காக கொஞ்சம் வரிகளை விட்டுவிடுகிறோம்
    int max_display = term_rows - 7; 
    if (max_display < 5) max_display = 5; // குறைந்தபட்சம் 5 வரிகளாவது காட்ட

    // 2. Viewport Auto-Scroll Logic 
    if (selected_idx < window_start) {
        window_start = selected_idx; // மேலே சென்றால் வியூபோர்ட்டை மேலே நகர்த்து
    } else if (selected_idx >= window_start + max_display) {
        window_start = selected_idx - max_display + 1; // கீழே சென்றால் கீழே நகர்த்து
    }

    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        strcpy(cwd, "Unknown Path");
    }

    printf("\033[2J\033[H"); 
    
    printf("\033[1;36m[ BDH Workspace: Interactive Tree ]\033[0m\r\n");
    printf("\033[1;33m📍 PWD: %s\033[0m\r\n", cwd); 
    printf("│\r\n");
    
    // 3. ஸ்க்ரோலிங் படியிலான பிரிண்டிங் (மொத்த ஃபைல்களையும் காட்டாமல்)
    for (int i = window_start; i < file_count && i < window_start + max_display; i++) {
        if (i == selected_idx) {
            printf("\033[1;32m  > \033[7m"); 
        } else {
            printf("    ");
        }

        printf("├── ");
        
        if (files[i].is_dir) {
            printf("\033[1;36m%s/\033[0m", files[i].name); 
        } else {
            printf("%s", files[i].name); 
        }

        if (i == selected_idx) {
            printf("\033[0m"); 
        }
        printf("\r\n");
    }
    
    // ஒருவேளை இன்னும் ஃபைல்கள் இருந்தால் அதைக் குறிக்க
    if (window_start + max_display < file_count) {
        printf("    \033[1;30m... (%d more files)\033[0m\r\n", file_count - (window_start + max_display));
    } else {
        printf("│\r\n");
    }

    printf("\r\n[UP/DOWN: Navigate] | [RIGHT/ENTER: Open] | [LEFT/BACKSPACE: Back] | [Q: Quit]\r\n");
}

int main() {
    load_files(".");
    enable_raw_mode();
    
    while (1) {
        draw_tree();
        
        char c;
        if (read(STDIN_FILENO, &c, 1) == 1) {
            if (c == '\033') { 
                char seq[3];
                if (read(STDIN_FILENO, &seq[0], 1) == 0) continue;
                if (read(STDIN_FILENO, &seq[1], 1) == 0) continue;

                if (seq[0] == '[') {
                    if (seq[1] == 'A') { // Up
                        if (selected_idx > 0) selected_idx--;
                    } 
                    else if (seq[1] == 'B') { // Down
                        if (selected_idx < file_count - 1) selected_idx++;
                    }
                    else if (seq[1] == 'C') { // Right
                        if (files[selected_idx].is_dir) {
                            chdir(files[selected_idx].name);
                            load_files(".");
                            selected_idx = 0;
                        }
                    }
                    else if (seq[1] == 'D') { // Left
                        chdir(".."); 
                        load_files(".");
                        selected_idx = 0;
                    }
                }
            } 
            else {
                if (c == 'q') {
                    break; 
                } 
                else if (c == 'w' || c == 'k') { 
                    if (selected_idx > 0) selected_idx--;
                } 
                else if (c == 's' || c == 'j') { 
                    if (selected_idx < file_count - 1) selected_idx++;
                } 
                else if (c == 127 || c == 8 || c == 'b') { 
                    chdir("..");
                    load_files(".");
                    selected_idx = 0;
                }
                else if (c == '\n' || c == '\r') { 
                    if (files[selected_idx].is_dir) {
                        chdir(files[selected_idx].name);
                        load_files(".");
                        selected_idx = 0; 
                    } else {
                        disable_raw_mode(); 
                        printf("\033[2J\033[H"); 
                        
                        char cmd[512];
                        snprintf(cmd, sizeof(cmd), "bdh-edit %s", files[selected_idx].name);
                        system(cmd); 
                        
                        enable_raw_mode(); 
                    }
                }
            }
        }
    }
    
    printf("\033[2J\033[H");
    return 0;
}
