
#include <stdio.h>
#include <string.h>
#include "../include/bdh-edit.h"

// மெமரி பஃபர் மற்றும் ஸ்டேட்கள்
char editor_buf[EDITOR_BUF_SIZE] = "";
int editor_len = 0;
char current_file[256] = "untitled.txt";

// 🔥 UI Cursor Tracking Variables
int editor_cx = 0;
int editor_cy = 0;

// கர்சர் இடத்தை அப்டேட் செய்யும் ஃபங்ஷன்
void update_cursor_position() {
    editor_cx = 0;
    editor_cy = 0;
    for (int i = 0; i < editor_len; i++) {
        if (editor_buf[i] == '\n') {
            editor_cy++;
            editor_cx = 0;
        } else {
            editor_cx++;
        }
    }
}

// 1. ஃபைலை டிஸ்க்கில் இருந்து பஃபருக்கு ஏற்றும் லாஜிக்
void load_file_to_editor(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        snprintf(editor_buf, EDITOR_BUF_SIZE, "// New File: %s\n", filename);
        editor_len = strlen(editor_buf);
        strncpy(current_file, filename, sizeof(current_file) - 1);
        update_cursor_position();
        return;
    }
    editor_len = fread(editor_buf, 1, EDITOR_BUF_SIZE - 1, f);
    editor_buf[editor_len] = '\0';
    fclose(f);
    strncpy(current_file, filename, sizeof(current_file) - 1);
    update_cursor_position();
}

// 2. பஃபரில் உள்ளதை டிஸ்க்கில் சேவ் செய்யும் லாஜிக்
void save_editor_file(void) {
    FILE *f = fopen(current_file, "w");
    if (!f) return;
    fwrite(editor_buf, 1, editor_len, f);
    fclose(f);
}

// 3. யூசர் டைப் செய்வதை பஃபரில் சேர்க்கும் லாஜிக்
int handle_editor_input(char c) {
    if (c == '\033') { 
        // ESC அழுத்தினால் Focus-ஐ Tree-க்கு மாற்ற main.c-க்கு 0 அனுப்புகிறோம்
        return 0; 
    }
    else if (c == 19) { // Ctrl+S (ASCII 19)
        save_editor_file();
    }
    else if (c >= 32 && c <= 126) { // சாதாரண எழுத்துக்கள் (Printable chars)
        if (editor_len < EDITOR_BUF_SIZE - 1) {
            editor_buf[editor_len++] = c;
            editor_buf[editor_len] = '\0';
            editor_cx++; // வலது பக்கம் கர்சரை நகர்த்துகிறோம்
        }
    }
    else if (c == 127 || c == 8) { // Backspace
        if (editor_len > 0) {
            editor_len--;
            editor_buf[editor_len] = '\0';
            update_cursor_position(); // பேக்ஸ்பேஸ் அடித்தால் கர்சரை ரீ-கால்குலேட் செய்கிறோம்
        }
    }
    else if (c == '\n' || c == '\r') { // Enter பட்டன்
        if (editor_len < EDITOR_BUF_SIZE - 1) {
            editor_buf[editor_len++] = '\n';
            editor_buf[editor_len] = '\0';
            editor_cy++; // கர்சரை அடுத்த வரிக்குக் கொண்டு செல்கிறோம்
            editor_cx = 0;
        }
    }
    
    // எடிட்டரிலேயே தொடர 1 ரிட்டர்ன் செய்கிறோம்
    return 1; 
}
