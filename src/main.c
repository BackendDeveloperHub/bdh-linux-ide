#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "../include/bdh-tree.h"
#include "../include/bdh-edit.h"
#include "../include/bdh-db.h"

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

int focus = 0; // 0 = Tree, 1 = Editor, 2 = DB Console, 3 = Terminal
int total_rows, total_cols, divider_col;
struct termios orig_termios;

// DB Console வேரியபிள்கள்
char db_query_buf[1024] = "";
int db_query_len = 0;
PGconn *db_conn = NULL; 

// Integrated Terminal வேரியபிள்கள்
char term_cmd_buf[1024] = "";
int term_cmd_len = 0;
char term_output[2][1024] = {"", ""};

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
    divider_col = total_cols / 3; 
}

// கமாண்டை ரன் செய்து அவுட்புட்டைப் பிடிக்கும் லாஜிக்
void run_terminal_command() {
    char sys_cmd[1200];
    snprintf(sys_cmd, sizeof(sys_cmd), "%s > .bdh_term_out 2>&1", term_cmd_buf);
    system(sys_cmd);

    FILE *fp = fopen(".bdh_term_out", "r");
    if (fp) {
        char line[1024];
        strcpy(term_output[0], ""); strcpy(term_output[1], "");
        while (fgets(line, sizeof(line), fp)) {
            line[strcspn(line, "\n")] = 0;
            strcpy(term_output[0], term_output[1]);
            strncpy(term_output[1], line, 1023);
        }
        fclose(fp);
    }
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

    // 1. செங்குத்து பார்டர் (Tree-க்கும் Editor-க்கும் நடுவில், Terminal-க்கு மேலே வரை)
    for (int i = 1; i <= total_rows - 6; i++) {
        len = snprintf(buf, sizeof(buf), "\033[%d;%dH\033[1;30m│\033[0m", i, divider_col);
        abAppend(&ab, buf, len);
    }

    // 2. கிடைமட்ட பார்டர் (Terminal-ஐப் பிரிக்க)
    for (int i = 1; i <= total_cols; i++) {
        len = snprintf(buf, sizeof(buf), "\033[%d;%dH\033[1;30m─\033[0m", total_rows - 5, i);
        abAppend(&ab, buf, len);
    }

    // 3. Left Pane (TREE)
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
    
    int max_display = total_rows - 10; 
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

    // 4. Right Pane (EDITOR or DB CONSOLE)
    if (focus == 0 || focus == 1 || focus == 3) {
        len = snprintf(buf, sizeof(buf), "\033[1;%dH%s%s ]\033[0m", divider_col + 2, 
                       focus == 1 ? "\033[1;44m[ BDH-EDIT (ACTIVE) - File: " : "\033[1;37m[ BDH-EDIT - File: ", current_file);
        abAppend(&ab, buf, len);

        int row = 3;
        int max_editor_width = total_cols - divider_col - 4;
        if (max_editor_width < 10) max_editor_width = 10;

        char *buf_copy = strdup(editor_buf);
        char *line = strtok(buf_copy, "\n");
        while (line != NULL && row <= total_rows - 6) {
            char clipped_line[1024];
            strncpy(clipped_line, line, max_editor_width);
            clipped_line[max_editor_width] = '\0';
            
            len = snprintf(buf, sizeof(buf), "\033[%d;%dH\033[0m%s\033[K", row, divider_col + 2, clipped_line);
            abAppend(&ab, buf, len);
            row++;
            line = strtok(NULL, "\n");
        }
        free(buf_copy);
    } 
    else if (focus == 2) {
        len = snprintf(buf, sizeof(buf), "\033[1;%dH\033[1;45m[ BDH-DB CONSOLE (ACTIVE) ]\033[0m", divider_col + 2);
        abAppend(&ab, buf, len);
        
        len = snprintf(buf, sizeof(buf), "\033[3;%dH\033[1;36mSQL > \033[0m%s", divider_col + 2, db_query_buf);
        abAppend(&ab, buf, len);
        
        len = snprintf(buf, sizeof(buf), "\033[5;%dH\033[1;30mStatus: %s\033[0m", divider_col + 2, 
                       (db_conn != NULL) ? "\033[1;32mConnected\033[0m" : "\033[1;31mDisconnected\033[0m");
        abAppend(&ab, buf, len);
    }

    // 5. Bottom Pane (INTEGRATED TERMINAL - 5 Lines Space)
    len = snprintf(buf, sizeof(buf), "\033[%d;1H%s PWD: %s\033[K", total_rows - 4,
                   focus == 3 ? "\033[1;46m[ BDH-TERMINAL (ACTIVE) ]\033[0m" : "\033[1;37m[ BDH-TERMINAL ]\033[0m", cwd);
    abAppend(&ab, buf, len);

    len = snprintf(buf, sizeof(buf), "\033[%d;1H\033[1;32m $\033[0m %s\033[K", total_rows - 3, term_cmd_buf);
    abAppend(&ab, buf, len);

    len = snprintf(buf, sizeof(buf), "\033[%d;1H\033[1;30m > \033[0m%s\033[K", total_rows - 2, term_output[0]);
    abAppend(&ab, buf, len);

    len = snprintf(buf, sizeof(buf), "\033[%d;1H\033[1;30m > \033[0m%s\033[K", total_rows - 1, term_output[1]);
    abAppend(&ab, buf, len);

    // 6. Footer
    len = snprintf(buf, sizeof(buf), "\033[%d;1H\033[1;33m [TAB]: Switch | [Ctrl+T]: Terminal | [Ctrl+S]: Save | [Ctrl+P]: DB | [Q]/[ESC]: Quit \033[0m\033[K", total_rows);
    abAppend(&ab, buf, len);

    // Cursor Positioning
    if (focus == 0) len = snprintf(buf, sizeof(buf), "\033[%d;6H", (selected_idx - window_start) + 4);
    else if (focus == 1) len = snprintf(buf, sizeof(buf), "\033[3;%dH", divider_col + 2);
    else if (focus == 2) len = snprintf(buf, sizeof(buf), "\033[3;%dH", divider_col + 8 + db_query_len);
    else if (focus == 3) len = snprintf(buf, sizeof(buf), "\033[%d;%dH", total_rows - 3, 4 + term_cmd_len);
    abAppend(&ab, buf, len);

    // கர்சரை மீண்டும் காண்பித்து மொத்தமாக ஸ்க்ரீனில் எழுதுகிறோம்!
    abAppend(&ab, "\033[?25h", 6);
    write(STDOUT_FILENO, ab.b, ab.len);
    abFree(&ab);
}

int main() {
    get_terminal_size();
    load_files(".");
    db_conn = db_connect("dbname=postgres user=postgres");
    enable_raw_mode();
    draw_ui();

    while (1) {
        char c;
        if (read(STDIN_FILENO, &c, 1) == 1) {
            
            if (c == 9) { focus = (focus == 0) ? 1 : 0; draw_ui(); continue; } // TAB
            if (c == 16) { focus = 2; draw_ui(); continue; } // Ctrl+P
            if (c == 20) { focus = 3; draw_ui(); continue; } // Ctrl+T

            if (focus == 0) {
                int status = handle_tree_input(c);
                if (status == 0) break;
                else if (status == 2) { load_file_to_editor(files[selected_idx].name); focus = 1; }
            } 
            else if (focus == 1) {
                if (handle_editor_input(c) == 0) focus = 0; 
            }
            else if (focus == 2) {
                if (c == '\033') focus = 0; 
                else if (c == 127 || c == 8) { if (db_query_len > 0) db_query_buf[--db_query_len] = '\0'; }
                else if (c >= 32 && c <= 126 && db_query_len < 1023) { db_query_buf[db_query_len++] = c; db_query_buf[db_query_len] = '\0'; }
                else if (c == '\n' || c == '\r') {
                    if (db_query_len > 0) {
                        disable_raw_mode(); printf("\033[2J\033[H");
                        db_execute_query(db_conn, db_query_buf);
                        printf("\n\033[1;33m[Press ENTER to return...]\033[0m"); getchar();
                        enable_raw_mode(); db_query_len = 0; db_query_buf[0] = '\0';
                    }
                }
            }
            else if (focus == 3) {
                if (c == '\033') focus = 0; 
                else if (c == 127 || c == 8) { if (term_cmd_len > 0) term_cmd_buf[--term_cmd_len] = '\0'; }
                else if (c >= 32 && c <= 126 && term_cmd_len < 1023) { term_cmd_buf[term_cmd_len++] = c; term_cmd_buf[term_cmd_len] = '\0'; }
                else if (c == '\n' || c == '\r') {
                    if (term_cmd_len > 0) {
                        run_terminal_command(); 
                        term_cmd_len = 0; term_cmd_buf[0] = '\0'; 
                    }
                }
            }
            
            get_terminal_size(); 
            draw_ui();
        }
    }
    
    db_close(db_conn);
    remove(".bdh_term_out");
    printf("\033[2J\033[H");
    return 0;
}
