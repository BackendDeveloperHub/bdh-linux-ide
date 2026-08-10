#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>

// UI States
int focus = 0; // 0 என்றால் Tree (Left), 1 என்றால் Editor (Right)
int total_rows, total_cols, divider_col;

// டெர்மினலை Raw Mode-க்கு மாற்றும் ஃபங்ஷன்
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

// திரையின் அளவைக் கண்டுபிடிக்கும் ஃபங்ஷன்
void get_terminal_size() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    total_rows = w.ws_row;
    total_cols = w.ws_col;
    divider_col = total_cols / 3; // திரையில் 1/3 பங்கு Tree-க்கு, மீதம் Editor-க்கு
}

// UI-ஐ வரையும் ஃபங்ஷன்
void draw_ui() {
    printf("\033[2J\033[H"); // திரையை க்ளீன் செய்

    // 1. செங்குத்து பார்டர் (Vertical Divider) வரைகிறோம்
    for (int i = 1; i < total_rows; i++) {
        printf("\033[%d;%dH\033[1;30m│\033[0m", i, divider_col);
    }

    // 2. Left Pane (BDH Tree Header)
    if (focus == 0) {
        printf("\033[1;2H\033[1;42m[ 🌳 BDH-TREE (ACTIVE) ]\033[0m"); // Focus இருந்தால் பச்சை நிறம்
    } else {
        printf("\033[1;2H\033[1;37m[ 🌳 BDH-TREE ]\033[0m");
    }

    // 3. Right Pane (BDH Edit Header)
    if (focus == 1) {
        printf("\033[1;%dH\033[1;44m[ 📝 BDH-EDIT (ACTIVE) ]\033[0m", divider_col + 2); // Focus இருந்தால் நீல நிறம்
    } else {
        printf("\033[1;%dH\033[1;37m[ 📝 BDH-EDIT ]\033[0m", divider_col + 2);
    }

    // 4. Footer (கீழே உள்ள இன்பர்மேஷன்)
    printf("\033[%d;1H\033[1;33m [TAB]: Switch Pane | [Q]: Quit IDE \033[0m", total_rows);

    // 5. கர்சரை சரியான இடத்தில் வைப்பது
    if (focus == 0) {
        printf("\033[3;2H"); // கர்சர் Tree-ல் இருக்கும்
    } else {
        printf("\033[3;%dH", divider_col + 2); // கர்சர் Editor-ல் இருக்கும்
    }

    fflush(stdout);
}

int main() {
    get_terminal_size();
    enable_raw_mode();
    draw_ui();

    while (1) {
        char c;
        if (read(STDIN_FILENO, &c, 1) == 1) {
            
            // 'q' அழுத்தினால் வெளியேற
            if (c == 'q') {
                break;
            }
            // TAB பட்டன் (ASCII 9) அழுத்தினால் Focus-ஐ மாற்ற
            else if (c == 9) {
                focus = !focus; // 0 இருந்தால் 1, 1 இருந்தால் 0 என மாறும்
                draw_ui();
            }
        }
    }

    printf("\033[2J\033[H");
    return 0;
}
