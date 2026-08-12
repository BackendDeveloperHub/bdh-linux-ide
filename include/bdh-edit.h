#ifndef BDH_EDIT_H
#define BDH_EDIT_H

#define EDITOR_BUF_SIZE 4096

// எடிட்டர் வேரியபிள்களை குளோபலாக அறிவிக்கிறோம்
extern char editor_buf[EDITOR_BUF_SIZE];
extern int editor_len;
extern char current_file[256];

// ஃபைல் ஆபரேஷன்கள்
void load_file_to_editor(const char *filename);
void save_editor_file(void);

// கீபோர்டு பட்டன்களைக் கையாளும் ஃபங்ஷன்
// (ESC அழுத்தினால் 0 ரிட்டர்ன் செய்யும், மற்றவற்றுக்கு 1)
int handle_editor_input(char c);

#endif // BDH_EDIT_H
