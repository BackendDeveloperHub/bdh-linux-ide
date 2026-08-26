#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "../include/bdh-tree.h"
#include "../include/bdh-edit.h"
#include "../include/bdh-db.h"
#include "../include/bdh-ui.h" // UI மாட்யூல் இணைக்கப்பட்டுள்ளது

int focus = 0; 
int total_rows, total_cols, divider_col;
struct termios orig_termios;

char db_query_buf[1024] = "";
int db_query_len = 0;
PGconn *db_conn = NULL; 

char term_cmd_buf[1024] = "";
int term_cmd_len = 0;
char term_output[7][1024] = {"", "", "", "", "", "", ""};

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
    divider_col = (int)(total_cols * tree_width_ratio); 
}

void run_terminal_command() {
    char sys_cmd[1200];
    snprintf(sys_cmd, sizeof(sys_cmd), "%s > .bdh_term_out 2>&1", term_cmd_buf);
    system(sys_cmd);

    FILE *fp = fopen(".bdh_term_out", "r");
    if (fp) {
        char line[1024];
        for (int i = 0; i < 7; i++) strcpy(term_output[i], ""); 
        while (fgets(line, sizeof(line), fp)) {
            line[strcspn(line, "\n")] = 0;
            for (int i = 0; i < 6; i++) strcpy(term_output[i], term_output[i+1]);
            strncpy(term_output[6], line, 1023);
        }
        fclose(fp);
    }
}

int main() {
    get_terminal_size();
    load_files(".");
    db_conn = db_connect("dbname=postgres user=postgres");
    
    printf("\033[?1000h\033[?1006h"); // மவுஸ் டிராக்கிங் ON
    enable_raw_mode();
    draw_ui();

    while (1) {
        char c;
        if (read(STDIN_FILENO, &c, 1) == 1) {
            
            if (c == 9) { 
                if (focus == 0) focus = 1;
                else if (focus == 1) focus = 3;
                else focus = 0; 
                draw_ui(); continue; 
            }
            if (c == 16) { focus = 2; draw_ui(); continue; } // Ctrl+P
            if (c == 20) { focus = 3; draw_ui(); continue; } // Ctrl+T

            if (focus == 0) {
                // --- 🔥 Dynamic Resize Shortcuts 🔥 ---
                if (c == '+') { if (term_height < total_rows - 8) term_height++; draw_ui(); continue; }
                if (c == '-') { if (term_height > 5) term_height--; draw_ui(); continue; }
                if (c == '>') { if (tree_width_ratio < 0.8) tree_width_ratio += 0.05; get_terminal_size(); draw_ui(); continue; }
                if (c == '<') { if (tree_width_ratio > 0.15) tree_width_ratio -= 0.05; get_terminal_size(); draw_ui(); continue; }
                // ---------------------------------------

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
            // --- 3. TERMINAL ---
            else if (focus == 3) {
                if (c == '\033') focus = 0; 
                else if (c == 127 || c == 8) { if (term_cmd_len > 0) term_cmd_buf[--term_cmd_len] = '\0'; }
                else if (c >= 32 && c <= 126 && term_cmd_len < 1023) { term_cmd_buf[term_cmd_len++] = c; term_cmd_buf[term_cmd_len] = '\0'; }
                else if (c == '\n' || c == '\r') {
                    if (term_cmd_len > 0) {
                        
                        // --- 🔥 CD Command Handle 🔥 ---
                        if (strncmp(term_cmd_buf, "cd ", 3) == 0) {
                            if (chdir(term_cmd_buf + 3) != 0) {
                                // Directory மாறவில்லை என்றால் எரர் மெசேஜைக் காட்ட
                                strcpy(term_output[6], "Directory not found!");
                            } else {
                                strcpy(term_output[6], "Changed directory successfully.");
                            }
                        } else {
                            run_terminal_command(); // மற்ற எல்லா கமாண்டுகளுக்கும்
                        }
                        
                        term_cmd_len = 0; term_cmd_buf[0] = '\0'; 
                        
                        // --- 🔥 AUTO-REFRESH TREE 🔥 ---
                        // புது ஃபைல்/ஃபோல்டர் உருவாக்கினால் உடனே Tree-யில் அப்டேட் ஆக
                        load_files("."); 
                    }
                }
            }
            
            get_terminal_size(); 
            draw_ui();
        }
    }
    
    db_close(db_conn);
    remove(".bdh_term_out");
    printf("\033[?1000l\033[?1006l"); // மவுஸ் டிராக்கிங் OFF
    printf("\033[2J\033[H");
    return 0;
}
