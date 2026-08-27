#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../include/bdh-ui.h"
#include "../include/bdh-tree.h"
#include "../include/bdh-edit.h"

int term_height = 10;          
float tree_width_ratio = 0.33; 

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

void draw_ui() {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) strcpy(cwd, "Unknown");

    struct abuf ab = ABUF_INIT;
    char buf[2048];
    int len;

    abAppend(&ab, "\033[?25l", 6); 
    // 🔥 GLITCH FIX 2: Disable Line Wrap (Prevents auto-scrolling bouncing effect)
    abAppend(&ab, "\033[?7l", 5);  
    abAppend(&ab, "\033[H", 3); 

    for (int i = 1; i <= total_rows - term_height; i++) {
        len = snprintf(buf, sizeof(buf), "\033[%d;%dH\033[1;30m│\033[0m", i, divider_col);
        abAppend(&ab, buf, len);
    }
    len = snprintf(buf, sizeof(buf), "\033[%d;%dH\033[1;36m[<]\033[0m", (total_rows - term_height) / 2 - 1, divider_col - 1);
    abAppend(&ab, buf, len);
    len = snprintf(buf, sizeof(buf), "\033[%d;%dH\033[1;36m[>]\033[0m", (total_rows - term_height) / 2 + 1, divider_col - 1);
    abAppend(&ab, buf, len);

    for (int i = 1; i <= total_cols; i++) {
        len = snprintf(buf, sizeof(buf), "\033[%d;%dH\033[1;30m─\033[0m", total_rows - term_height + 1, i);
        abAppend(&ab, buf, len);
    }
    len = snprintf(buf, sizeof(buf), "\033[%d;%dH\033[1;36m[ - ]   [ + ]\033[0m", total_rows - term_height + 1, (total_cols / 2) - 6);
    abAppend(&ab, buf, len);

    len = snprintf(buf, sizeof(buf), "\033[1;1H%s\033[K\r\n", 
                   focus == 0 ? "\033[1;42m[ BDH-TREE (ACTIVE) ]\033[0m" : "\033[1;37m[ BDH-TREE ]\033[0m");
    abAppend(&ab, buf, len);
    
    char clipped_cwd[256];
    int max_pwd_width = divider_col - 8;
    if (max_pwd_width < 5) max_pwd_width = 5;
    strncpy(clipped_cwd, cwd, max_pwd_width);
    clipped_cwd[max_pwd_width] = '\0';
    
    len = snprintf(buf, sizeof(buf), "\033[1;33m PWD: %s \033[0m\033[K\r\n\r\n", clipped_cwd);
    abAppend(&ab, buf, len);
    
    int max_display = total_rows - term_height - 4; 
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
        strcat(row_buf, "\033[K\r\n");
        abAppend(&ab, row_buf, strlen(row_buf));
    }

    if (focus == 0 || focus == 1 || focus == 3) {
        len = snprintf(buf, sizeof(buf), "\033[1;%dH%s%s ]\033[0m\033[K", divider_col + 2, 
                       focus == 1 ? "\033[1;44m[ BDH-EDIT (ACTIVE) - File: " : "\033[1;37m[ BDH-EDIT - File: ", current_file);
        abAppend(&ab, buf, len);

        int row = 3;
        int line_num = 1;
        int offset = 6; 
        int max_editor_width = total_cols - divider_col - 2 - offset;
        if (max_editor_width < 10) max_editor_width = 10;

        int line_start = 0;
        
        while (row <= total_rows - term_height) {
            int line_end = line_start;
            while (line_end < editor_len && editor_buf[line_end] != '\n') {
                line_end++;
            }
            
            int len_to_print = line_end - line_start;
            if (len_to_print > max_editor_width) len_to_print = max_editor_width; 
            
            char line_str[1024] = {0};
            if (len_to_print > 0) {
                strncpy(line_str, &editor_buf[line_start], len_to_print);
            }
            
            len = snprintf(buf, sizeof(buf), "\033[%d;%dH\033[1;30m%3d │\033[0m %s\033[K", 
                           row, divider_col + 2, line_num, line_str);
            abAppend(&ab, buf, len);
            
            row++;
            line_num++;
            
            if (line_end >= editor_len) {
                while(row <= total_rows - term_height) {
                    len = snprintf(buf, sizeof(buf), "\033[%d;%dH\033[1;30m%3d │\033[0m \033[K", row, divider_col + 2, line_num);
                    abAppend(&ab, buf, len);
                    row++;
                    line_num++;
                }
                break; 
            }
            line_start = line_end + 1; 
        }
    } 
    else if (focus == 2) {
        len = snprintf(buf, sizeof(buf), "\033[1;%dH\033[1;45m[ BDH-DB CONSOLE (ACTIVE) - PostgreSQL ]\033[0m\033[K", divider_col + 2);
        abAppend(&ab, buf, len);
        
        len = snprintf(buf, sizeof(buf), "\033[3;%dH\033[1;36mSQL > \033[0m%s\033[K", divider_col + 2, db_query_buf);
        abAppend(&ab, buf, len);
        
        len = snprintf(buf, sizeof(buf), "\033[5;%dH\033[1;30mStatus: %s\033[0m\033[K", divider_col + 2, 
                       (db_conn != NULL) ? "\033[1;32mConnected\033[0m" : "\033[1;31mDisconnected\033[0m");
        abAppend(&ab, buf, len);
    }

    len = snprintf(buf, sizeof(buf), "\033[%d;1H%s PWD: %s\033[K", total_rows - term_height + 2,
                   focus == 3 ? "\033[1;46m[ BDH-TERMINAL (ACTIVE) ]\033[0m" : "\033[1;37m[ BDH-TERMINAL ]\033[0m", cwd);
    abAppend(&ab, buf, len);

    len = snprintf(buf, sizeof(buf), "\033[%d;1H\033[1;32m $\033[0m %s\033[K", total_rows - term_height + 3, term_cmd_buf);
    abAppend(&ab, buf, len);

    int max_out_lines = term_height - 4;
    if (max_out_lines > 50) max_out_lines = 50;
    
    for (int i = 0; i < max_out_lines; i++) {
        len = snprintf(buf, sizeof(buf), "\033[%d;1H\033[1;30m > \033[0m%s\033[K", 
                       total_rows - term_height + 4 + i, term_output[50 - max_out_lines + i]);
        abAppend(&ab, buf, len);
    }

    len = snprintf(buf, sizeof(buf), "\033[%d;1H\033[1;33m [TAB]: Switch Pane | [Ctrl+T]: Terminal | [Ctrl+S]: Save | [Ctrl+P]: DB | [Q]/[ESC]: Quit \033[0m\033[K", total_rows);
    abAppend(&ab, buf, len);

    // 🔥 GLITCH FIX 2: Re-enable Line Wrap
    abAppend(&ab, "\033[?7h", 5); 
    abAppend(&ab, "\033[?25h", 6); 

    // 🔥 EDITOR CURSOR FIX: கர்சர் எல்லையை மீறாமல் பார்த்துக் கொள்ளும் லாஜிக்
    if (focus == 0) {
        len = snprintf(buf, sizeof(buf), "\033[%d;6H", (selected_idx - window_start) + 4);
    } else if (focus == 1) {
        int cursor_row = 3 + editor_cy;
        if (cursor_row > total_rows - term_height) {
            cursor_row = total_rows - term_height;
        }
        len = snprintf(buf, sizeof(buf), "\033[%d;%dH", cursor_row, divider_col + 8 + editor_cx); 
    } else if (focus == 2) {
        len = snprintf(buf, sizeof(buf), "\033[3;%dH", divider_col + 8 + db_query_len);
    } else if (focus == 3) {
        len = snprintf(buf, sizeof(buf), "\033[%d;%dH", total_rows - term_height + 3, 4 + term_cmd_len);
    }
    
    abAppend(&ab, buf, len);

    write(STDOUT_FILENO, ab.b, ab.len);
    abFree(&ab);
}
