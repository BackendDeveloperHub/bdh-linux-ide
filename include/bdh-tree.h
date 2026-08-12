#ifndef BDH_TREE_H
#define BDH_TREE_H

#define MAX_FILES 1024

typedef struct {
    char name[256];
    int is_dir;
} FileEntry;

// குளோபல் வேரியபிள்கள் (extern மூலமாக)
extern FileEntry files[MAX_FILES];
extern int file_count;
extern int selected_idx;
extern int window_start;

// ஃபைல் மற்றும் நேவிகேஷன் ஃபங்ஷன்கள்
void load_files(const char *path);

// கீபோர்டு பட்டன்களைக் கையாளும் ஃபங்ஷன்
// ரிட்டர்ன் வேல்யூஸ்: 0 = Quit, 1 = Continue, 2 = Open File in Editor
int handle_tree_input(char c);

#endif // BDH_TREE_H
