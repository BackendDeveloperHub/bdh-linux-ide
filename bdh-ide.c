#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define MAX_FILES 1024

// --- Global States ---
int focus = 0; // 0 = Tree, 1 = Editor
int total_rows, total_cols, divider_col;

// Tree Variables
typedef struct { char name[256]; int is_dir; } FileEntry;
FileEntry files[MAX_FILES];
int file_count = 0, selected_idx = 0;

// Terminal Raw Mode
struct termios orig_termios;
void disable_raw_mode() { tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios); }
void enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void get_terminal_size() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    total_rows = w.ws_row;
    total_cols = w.ws_col;
    divider_col = total_cols / 3; // 30% Tree, 70% Editor
}

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

// --- The Grand UI Drawer ---
void draw_ui() {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) strcpy(cwd, "Unknown");

    printf("\033[2J\033[H"); // முழு திரையையும் க்ளீன் செய்

    // 1. செங்குத்து பார்டர் (Vertical Divider)
    for (int i = 1; i <= total_rows; i++) {
        printf("\033[%d;%dH\033[1;30m│\033[0m", i, divider_col);
    }

    // 2. Left Pane (TREE)
    printf("\033[1;1H"); // இடதுபுறம் கர்சரை வை
    printf(focus == 0 ? "\033[1;42m[ BDH-TREE (ACTIVE) ]\033[0m\r\n" : "\033[1;37m[ BDH-TREE ]\033[0m\r\n");
    printf("\033[1;33m PWD: %s \033[0m\r\n\r\n", cwd);
    
    for (int i = 0; i < file_count && i < total_rows - 5; i++) {
        if (i == selected_idx && focus == 0) printf("\033[1;32m  > \033[7m"); 
        else printf("    ");

        if (files[i].is_dir) printf("\033[1;34m├── %s/\033[0m", files[i].name);
        else printf("├── %s", files[i].name);

        if (i == selected_idx && focus == 0) printf("\033[0m");
        printf("\r\n");
    }

    // 3. Right Pane (EDITOR)
    printf("\033[1;%dH", divider_col + 2); // வலதுபுறம் கர்சரை வை
    printf(focus == 1 ? "\033[1;44m[ BDH-EDIT (ACTIVE) - Tamizhi ]\033[0m" : "\033[1;37m[ BDH-EDIT ]\033[0m");
    printf("\033[3;%dH\033[1;30mEditor logic will be injected here...\033[0m", divider_col + 2);

    // 4. Footer
    printf("\033[%d;1H\033[1;33m [TAB]: Switch Pane | [Q]: Quit \033[0m", total_rows);

    // கர்சரை ஆக்டிவ் பேனலில் வைக்க
    if (focus == 0) printf("\033[%d;6H", selected_idx + 4);
    else printf("\033[3;%dH", divider_col + 2);
    
    fflush(stdout);
}

int main() {
    get_terminal_size();
    load_files(".");
    enable_raw_mode();
    draw_ui();

    while (1) {
        char c;
        if (read(STDIN_FILENO, &c, 1) == 1) {
            
            if (c == 'q' && focus == 0) break; // Tree-ல் இருந்து Q அழுத்தினால் வெளியேற
            
            if (c == 9) { // TAB பட்டன் அழுத்தினால் Focus மாறுதல்
                focus = !focus; 
                draw_ui();
                continue;
            }

            // --- TREE LOGIC (Left Pane) ---
            if (focus == 0) {
                if (c == '\033') { 
                    char seq[3];
                    if (read(STDIN_FILENO, &seq[0], 1) == 0 || read(STDIN_FILENO, &seq[1], 1) == 0) continue;
                    if (seq[0] == '[') {
                        if (seq[1] == 'A' && selected_idx > 0) selected_idx--; // UP
                        else if (seq[1] == 'B' && selected_idx < file_count - 1) selected_idx++; // DOWN
                        else if (seq[1] == 'C' && files[selected_idx].is_dir) { // RIGHT (Enter Dir)
                            chdir(files[selected_idx].name); load_files("."); selected_idx = 0;
                        }
                        else if (seq[1] == 'D') { // LEFT (Go Back)
                            chdir(".."); load_files("."); selected_idx = 0;
                        }
                    }
                } 
                else if (c == 127 || c == 8) { chdir(".."); load_files("."); selected_idx = 0; } // Backspace
                else if (c == '\n' || c == '\r') {
                    if (files[selected_idx].is_dir) { chdir(files[selected_idx].name); load_files("."); selected_idx = 0; }
                    else { focus = 1; } // ஃபைலைத் திறக்க வலதுபுறம் (Editor-க்கு) Focus-ஐ மாற்று!
                }
                draw_ui();
            }
            // --- EDITOR LOGIC (Right Pane) ---
            else if (focus == 1) {
                // இங்கு உங்களது 1-Byte தமிழ் கீபோர்டு லாஜிக்கை இணைப்போம்!
                // இப்போதைக்கு ESC அழுத்தினால் மீண்டும் Tree-க்கு வர ஒரு சின்ன ஷார்ட்கட்:
                if (c == '\033') { focus = 0; draw_ui(); }
            }
        }
    }
    
    printf("\033[2J\033[H");
    return 0;
}
