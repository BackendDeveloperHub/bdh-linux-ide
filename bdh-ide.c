#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define MAX_FILES 1024
#define EDITOR_BUF_SIZE 4096

// --- Global States ---
int focus = 0; // 0 = Tree, 1 = Editor
int total_rows, total_cols, divider_col;

// Tree Variables
typedef struct { char name[256]; int is_dir; } FileEntry;
FileEntry files[MAX_FILES];
int file_count = 0, selected_idx = 0;

// Editor Variables
char editor_buf[EDITOR_BUF_SIZE] = "";
int editor_len = 0;
char current_file[256] = "untitled.txt";

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

// ஃபைலை எடிட்டரில் லோட் செய்யும் ஃபங்ஷன்
void load_file_to_editor(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        snprintf(editor_buf, EDITOR_BUF_SIZE, "// New File: %s\n", filename);
        editor_len = strlen(editor_buf);
        strncpy(current_file, filename, sizeof(current_file) - 1);
        return;
    }
    editor_len = fread(editor_buf, 1, EDITOR_BUF_SIZE - 1, f);
    editor_buf[editor_len] = '\0';
    fclose(f);
    strncpy(current_file, filename, sizeof(current_file) - 1);
}

// எடிட்டர் கண்டென்ட்டை சேவ் செய்யும் ஃபங்ஷன்
void save_editor_file() {
    FILE *f = fopen(current_file, "w");
    if (!f) return;
    fwrite(editor_buf, 1, editor_len, f);
    fclose(f);
}

// --- The Grand UI Drawer (With Over-write Fix) ---
void draw_ui() {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) strcpy(cwd, "Unknown");

    printf("\033[2J\033[H"); // முழு திரையையும் க்ளீன் செய்

    // 1. செங்குத்து பார்டர் (Vertical Divider)
    for (int i = 1; i <= total_rows; i++) {
        printf("\033[%d;%dH\033[1;30m│\033[0m", i, divider_col);
    }

    // 2. Left Pane (TREE)
    printf("\033[1;1H"); 
    printf(focus == 0 ? "\033[1;42m[ BDH-TREE (ACTIVE) ]\033[0m\r\n" : "\033[1;37m[ BDH-TREE ]\033[0m\r\n");
    printf("\033[1;33m PWD: %s \033[0m\r\n\r\n", cwd);
    
    for (int i = 0; i < file_count && i < total_rows - 5; i++) {
        if (i == selected_idx && focus == 0) printf("\033[1;32m  > \033[7m"); 
        else printf("    ");

        // ட்ரீ பெயர்களை டிவைடருக்குள் கட் செய்து காட்ட
        char display_name[256];
        int max_tree_width = divider_col - 6;
        if (max_tree_width < 5) max_tree_width = 5;
        
        strncpy(display_name, files[i].name, max_tree_width);
        display_name[max_tree_width] = '\0';

        if (files[i].is_dir) printf("\033[1;34m├── %s/\033[0m", display_name);
        else printf("├── %s", display_name);

        if (i == selected_idx && focus == 0) printf("\033[0m");
        printf("\r\n");
    }

    // 3. Right Pane (EDITOR Header)
    printf("\033[1;%dH", divider_col + 2); 
    if (focus == 1) {
        printf("\033[1;44m[ BDH-EDIT (ACTIVE) - File: %s ]\033[0m", current_file);
    } else {
        printf("\033[1;37m[ BDH-EDIT - File: %s ]\033[0m", current_file);
    }

    // எடிட்டர் டெக்ஸ்டை பார்டரை தாண்டாமல் கட் செய்து பிரிண்ட் செய்வது
    int row = 3;
    int max_editor_width = total_cols - divider_col - 4;
    if (max_editor_width < 10) max_editor_width = 10;

    char *buf_copy = strdup(editor_buf);
    char *line = strtok(buf_copy, "\n");
    while (line != NULL && row < total_rows - 1) {
        char clipped_line[1024];
        strncpy(clipped_line, line, max_editor_width);
        clipped_line[max_editor_width] = '\0';

        printf("\033[%d;%dH\033[0m%s", row, divider_col + 2, clipped_line);
        row++;
        line = strtok(NULL, "\n");
    }
    free(buf_copy);

    // 4. Footer
    printf("\033[%d;1H\033[1;33m [TAB]: Switch Pane | [Ctrl+S]: Save | [Q]: Quit \033[0m", total_rows);

    // கர்சரை சரியான இடத்தில் வைப்பது
    if (focus == 0) {
        printf("\033[%d;6H", selected_idx + 4);
    } else {
        printf("\033[3;%dH", divider_col + 2);
    }
    
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
            
            if (c == 9) { // TAB பட்டன்
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
                        else if (seq[1] == 'C' && files[selected_idx].is_dir) { // RIGHT
                            chdir(files[selected_idx].name); load_files("."); selected_idx = 0;
                        }
                        else if (seq[1] == 'D') { // LEFT
                            chdir(".."); load_files("."); selected_idx = 0;
                        }
                    }
                } 
                else if (c == 127 || c == 8) { chdir(".."); load_files("."); selected_idx = 0; }
                else if (c == '\n' || c == '\r') {
                    if (files[selected_idx].is_dir) { 
                        chdir(files[selected_idx].name); load_files("."); selected_idx = 0; 
                    } else {
                        // ஃபைலைத் தேர்ந்தெடுத்தால் அதை எடிட்டரில் லோட் செய்து Focus-ஐ மாற்று!
                        load_file_to_editor(files[selected_idx].name);
                        focus = 1; 
                    }
                }
                draw_ui();
            }
            // --- EDITOR LOGIC (Right Pane) ---
            else if (focus == 1) {
                if (c == '\033') { 
                    focus = 0; // ESC அழுத்தினால் Tree-க்கு திரும்பும்
                }
                else if (c == 19) { // Ctrl+S (ASCII 19) அழுத்தினால் சேவ் ஆகும்
                    save_editor_file();
                }
                else if (c >= 32 && c <= 126) { // சாதாரண எழுத்துக்கள் டைப் செய்தால்
                    if (editor_len < EDITOR_BUF_SIZE - 1) {
                        editor_buf[editor_len++] = c;
                        editor_buf[editor_len] = '\0';
                    }
                }
                else if (c == 127 || c == 8) { // Backspace
                    if (editor_len > 0) {
                        editor_len--;
                        editor_buf[editor_len] = '\0';
                    }
                }
                else if (c == '\n' || c == '\r') { // Enter
                    if (editor_len < EDITOR_BUF_SIZE - 1) {
                        editor_buf[editor_len++] = '\n';
                        editor_buf[editor_len] = '\0';
                    }
                }
                draw_ui();
            }
        }
    }
    
    printf("\033[2J\033[H");
    return 0;
}
