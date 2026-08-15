#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "../include/bdh-tree.h"
#include "../include/bdh-edit.h"
#include "../include/bdh-db.h" // புதிய DB மாட்யூல் இணைக்கப்பட்டுள்ளது

// --- 🔥 ANTI-GLITCH DOUBLE BUFFERING MAGIC 🔥 ---
struct abuf {
    char *b;
    int len;
};
#define ABUF_INIT {NULL, 0}

static void abAppend(struct abuf *ab, const char *s, int len) {
    char *new = realloc(ab->b, ab->len + len);
    if (new == NULL) return;
    memcpy(&new[ab->len], s, len);
    ab->b = new;
    ab->len += len;
}

static void abFree(struct abuf *ab) {
    free(ab->b);
}
// -------------------------------------------------

int focus = 0; // 0 = Tree, 1 = Editor, 2 = DB Console
int total_rows, total_cols, divider_col;
struct termios orig_termios;

// DB Console-க்கான பிரத்யேக வேரியபிள்கள்
char db_query_buf[1024] = "";
int db_query_len = 0;
PGconn *db_conn = NULL; // டேட்டாபேஸ் கனெக்ஷன்

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
    divider_col = total_cols / 3; 
}

void draw_ui() {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) strcpy(cwd, "Unknown");

    struct abuf ab = ABUF_INIT;
    char buf[2048];
    int len;

    // கர்சரை மறைத்து, ஸ்க்ரீனை க்ளியர் செய்கிறோம்
    abAppend(&ab, "\033[?25l", 6);
    abAppend(&ab, "\033[2J\033[H", 7); 

    // 1. செங்குத்து பார்டர் (Vertical Divider)
    for (int i = 1; i <= total_rows; i++) {
        len = snprintf(buf, sizeof(buf), "\033[%d;%dH\033[1;30m│\033[0m", i, divider_col);
        abAppend(&ab, buf, len);
    }

    // 2. Left Pane (TREE)
    len = snprintf(buf, sizeof(buf), "\033[1;1H%s\r\n", 
                   focus == 0 ? "\033[1;42m[ BDH-TREE (ACTIVE) ]\033[0m" : "\033[1;37m[ BDH-TREE ]\033[0m");
    abAppend(&ab, buf, len);
    
    char clipped_cwd[256];
    int max_pwd_width = divider_col - 8;
    if (max_pwd_width < 5) max_pwd_width = 5;
    strncpy(clipped_cwd, cwd, max_pwd_width);
    clipped_cwd[max_pwd_width] = '\0';
    
    len = snprintf(buf, sizeof(buf), "\033[1;33m PWD: %s \033[0m\r\n\r\n", clipped_cwd);
    abAppend(&ab, buf, len);
    
    int max_display = total_rows - 7; 
    if (max_display < 5) max_display = 5;

    if (selected_idx < window_start) window_start = selected_idx;
    else if (selected_idx >= window_start + max_display) window_start = selected_idx - max_display + 1;
    
    for (int i = window_start; i < file_count && i < window_start + max_display; i++) {
        int max_name_len = divider_col - 8;
        if (max_name_len < 4) max_name_len = 4;

        char display_name[256];
        strncpy(display_name, files[i].name, max_name_len);
        display_name[max_name_len] = '\0';

        char row_buf[1024] = "";
        
        if (i == selected_idx && focus == 0) strcat(row_buf, "\033[1;32m  > \033[7m"); 
        else strcat(row_buf, "    ");

        char temp[512];
        if (files[i].is_dir) snprintf(temp, sizeof(temp), "\033[1;34m├── %s/\033[0m", display_name);
        else snprintf(temp, sizeof(temp), "├── %s", display_name);
        strcat(row_buf, temp);

        if (i == selected_idx && focus == 0) strcat(row_buf, "\033[0m");
        
        int current_len = strlen(display_name) + 7; 
        for (int spaces = current_len; spaces < divider_col; spaces++) strcat(row_buf, " ");
        strcat(row_buf, "\r\n");
        
        abAppend(&ab, row_buf, strlen(row_buf));
    }

    // 3. Right Pane (EDITOR or DB CONSOLE)
    if (focus == 0 || focus == 1) {
        len = snprintf(buf, sizeof(buf), "\033[1;%dH%s", divider_col + 2, 
                       focus == 1 ? "\033[1;44m[ BDH-EDIT (ACTIVE) - File: " : "\033[1;37m[ BDH-EDIT - File: ");
        abAppend(&ab, buf, len);
        
        len = snprintf(buf, sizeof(buf), "%s ]\033[0m", current_file);
        abAppend(&ab, buf, len);

        int row = 3;
        int max_editor_width = total_cols - divider_col - 4;
        if (max_editor_width < 10) max_editor_width = 10;

        char *buf_copy = strdup(editor_buf);
        char *line = strtok(buf_copy, "\n");
        while (line != NULL && row < total_rows - 1) {
            char clipped_line[1024];
            strncpy(clipped_line, line, max_editor_width);
            clipped_line[max_editor_width] = '\0';
            
            len = snprintf(buf, sizeof(buf), "\033[%d;%dH\033[0m%s", row, divider_col + 2, clipped_line);
            abAppend(&ab, buf, len);
            
            row++;
            line = strtok(NULL, "\n");
        }
        free(buf_copy);
    } 
    else if (focus == 2) {
        len = snprintf(buf, sizeof(buf), "\033[1;%dH\033[1;45m[ BDH-DB CONSOLE (ACTIVE) - PostgreSQL ]\033[0m", divider_col + 2);
        abAppend(&ab, buf, len);
        
        len = snprintf(buf, sizeof(buf), "\033[3;%dH\033[1;36mSQL > \033[0m%s", divider_col + 2, db_query_buf);
        abAppend(&ab, buf, len);
        
        len = snprintf(buf, sizeof(buf), "\033[5;%dH\033[1;30mStatus: %s\033[0m", divider_col + 2, 
                       (db_conn != NULL) ? "\033[1;32mConnected\033[0m" : "\033[1;31mDisconnected\033[0m");
        abAppend(&ab, buf, len);
    }

    // 4. Footer
    len = snprintf(buf, sizeof(buf), "\033[%d;1H\033[1;33m [TAB]: Switch Pane | [Ctrl+S]: Save | [Ctrl+P]: DB Console | [Q]/[ESC]: Quit \033[0m\033[K", total_rows);
    abAppend(&ab, buf, len);

    // கர்சரை நிலைநிறுத்துதல்
    if (focus == 0) len = snprintf(buf, sizeof(buf), "\033[%d;6H", (selected_idx - window_start) + 4);
    else if (focus == 1) len = snprintf(buf, sizeof(buf), "\033[3;%dH", divider_col + 2);
    else if (focus == 2) len = snprintf(buf, sizeof(buf), "\033[3;%dH", divider_col + 8 + db_query_len);
    abAppend(&ab, buf, len);

    // கர்சரை மீண்டும் காண்பித்து மொத்தமாக ஸ்க்ரீனில் எழுதுகிறோம்!
    abAppend(&ab, "\033[?25h", 6);
    write(STDOUT_FILENO, ab.b, ab.len);
    abFree(&ab);
}

int main() {
    get_terminal_size();
    load_files(".");
    
    // IDE தொடங்கும்போதே டேட்டாபேஸுடன் கனெக்ட் செய்கிறோம்
    db_conn = db_connect("dbname=postgres user=postgres");

    enable_raw_mode();
    draw_ui();

    while (1) {
        char c;
        if (read(STDIN_FILENO, &c, 1) == 1) {
            
            // Pane Switching Logic (TAB பட்டன்)
            if (c == 9) { 
                focus = (focus == 0) ? 1 : 0; // Tree மற்றும் Editor-க்கு இடையே மட்டும் சுவிட்ச் செய்ய
                draw_ui();
                continue;
            }
            
            // Ctrl + P (ASCII 16) -> DB Console-ஐ திறக்க
            if (c == 16) {
                focus = 2;
                draw_ui();
                continue;
            }

            // Tree Active (Left)
            if (focus == 0) {
                int status = handle_tree_input(c);
                if (status == 0) { // Quit
                    break;
                } else if (status == 2) { // Open File
                    load_file_to_editor(files[selected_idx].name);
                    focus = 1; 
                }
            } 
            // Editor Active (Right)
            else if (focus == 1) {
                int status = handle_editor_input(c);
                if (status == 0) { // ESC 
                    focus = 0; 
                }
            }
            // DB Console Active (Right Pane)
            else if (focus == 2) {
                if (c == '\033') { 
                    focus = 0; // ESC அழுத்தினால் Tree-க்கு திரும்பும்
                }
                else if (c == 127 || c == 8) { // Backspace
                    if (db_query_len > 0) {
                        db_query_len--;
                        db_query_buf[db_query_len] = '\0';
                    }
                }
                else if (c >= 32 && c <= 126) { // டைப்பிங்
                    if (db_query_len < 1023) {
                        db_query_buf[db_query_len++] = c;
                        db_query_buf[db_query_len] = '\0';
                    }
                }
                else if (c == '\n' || c == '\r') { // Enter அழுத்தினால் குவரியை ரன் செய்ய
                    if (db_query_len > 0) {
                        disable_raw_mode();
                        printf("\033[2J\033[H");
                        printf("\033[1;32m[BDH-DB] Executing Query:\033[0m %s\n\n", db_query_buf);
                        
                        db_execute_query(db_conn, db_query_buf);
                        
                        printf("\n\033[1;33m[Press ENTER to return to IDE Workspace...]\033[0m");
                        getchar(); 
                        
                        enable_raw_mode(); 
                        db_query_len = 0;
                        db_query_buf[0] = '\0';
                    }
                }
            }
            
            get_terminal_size(); 
            draw_ui();
        }
    }
    
    db_close(db_conn);
    printf("\033[2J\033[H");
    return 0;
}
