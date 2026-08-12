#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "../include/bdh-tree.h"
#include "../include/bdh-edit.h"

int focus = 0; // 0 = Tree (Left Pane), 1 = Editor (Right Pane)
int total_rows, total_cols, divider_col;
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

void get_terminal_size() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    total_rows = w.ws_row;
    total_cols = w.ws_col;
    divider_col = total_cols / 3; // 30% Tree, 70% Editor
}

void draw_ui() {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) strcpy(cwd, "Unknown");

    printf("\033[2J\033[H"); // திரையை க்ளீன் செய்

    // 1. செங்குத்து பார்டர் (Vertical Divider)
    for (int i = 1; i <= total_rows; i++) {
        printf("\033[%d;%dH\033[1;30m│\033[0m", i, divider_col);
    }

    // 2. Left Pane (TREE)
    printf("\033[1;1H"); 
    printf(focus == 0 ? "\033[1;42m[ BDH-TREE (ACTIVE) ]\033[0m\r\n" : "\033[1;37m[ BDH-TREE ]\033[0m\r\n");
    
    char clipped_cwd[256];
    int max_pwd_width = divider_col - 8;
    if (max_pwd_width < 5) max_pwd_width = 5;
    strncpy(clipped_cwd, cwd, max_pwd_width);
    clipped_cwd[max_pwd_width] = '\0';
    printf("\033[1;33m PWD: %s \033[0m\r\n\r\n", clipped_cwd);
    
    int max_display = total_rows - 7; 
    if (max_display < 5) max_display = 5;

    // ஸ்க்ரோலிங் Viewport லாஜிக்
    if (selected_idx < window_start) window_start = selected_idx;
    else if (selected_idx >= window_start + max_display) window_start = selected_idx - max_display + 1;
    
    for (int i = window_start; i < file_count && i < window_start + max_display; i++) {
        int max_name_len = divider_col - 8;
        if (max_name_len < 4) max_name_len = 4;

        char display_name[256];
        strncpy(display_name, files[i].name, max_name_len);
        display_name[max_name_len] = '\0';

        if (i == selected_idx && focus == 0) printf("\033[1;32m  > \033[7m"); 
        else printf("    ");

        if (files[i].is_dir) printf("\033[1;34m├── %s/\033[0m", display_name);
        else printf("├── %s", display_name);

        if (i == selected_idx && focus == 0) printf("\033[0m");
        
        int current_len = strlen(display_name) + 7; 
        for (int spaces = current_len; spaces < divider_col; spaces++) printf(" ");
        printf("\r\n");
    }

    // 3. Right Pane (EDITOR)
    printf("\033[1;%dH", divider_col + 2); 
    if (focus == 1) printf("\033[1;44m[ BDH-EDIT (ACTIVE) - File: %s ]\033[0m", current_file);
    else printf("\033[1;37m[ BDH-EDIT - File: %s ]\033[0m", current_file);

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
    printf("\033[%d;1H\033[1;33m [TAB]: Switch Pane | [Ctrl+S]: Save | [Q]: Quit Tree / [ESC]: Quit Editor \033[0m", total_rows);

    // கர்சரை நிலைநிறுத்துதல்
    if (focus == 0) printf("\033[%d;6H", (selected_idx - window_start) + 4);
    else printf("\033[3;%dH", divider_col + 2); // (தற்காலிகமாக எடிட்டரின் தொடக்கத்தில்)
    
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
            
            // Pane Switching Logic (TAB பட்டன்)
            if (c == 9) { 
                focus = !focus; 
                draw_ui();
                continue;
            }

            // Tree Active (Left)
            if (focus == 0) {
                int status = handle_tree_input(c);
                if (status == 0) { // Quit சிக்னல்
                    break;
                } else if (status == 2) { // Open File சிக்னல்
                    load_file_to_editor(files[selected_idx].name);
                    focus = 1; 
                }
            } 
            // Editor Active (Right)
            else if (focus == 1) {
                int status = handle_editor_input(c);
                if (status == 0) { // ESC சிக்னல்
                    focus = 0; 
                }
            }
            
            // எந்த கீ அழுத்தினாலும் ஸ்கிரீன் அளவு மாறினால் அப்டேட் செய்து ரீ-ட்ரா செய்ய
            get_terminal_size(); 
            draw_ui();
        }
    }
    
    printf("\033[2J\033[H");
    return 0;
}
