#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include "../include/bdh-tree.h"

FileEntry files[MAX_FILES];
int file_count = 0;
int selected_idx = 0;
int window_start = 0;

void load_files(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) return;
    struct dirent *entry;
    file_count = 0;
    window_start = 0; // புதிய ஃபோல்டருக்குப் போகும்போது வியூபோர்ட்டை ரீசெட் செய்ய
    
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

int handle_tree_input(char c) {
    if (c == '\033') { 
        char seq[3];
        if (read(STDIN_FILENO, &seq[0], 1) == 0) return 1;
        if (read(STDIN_FILENO, &seq[1], 1) == 0) return 1;

        if (seq[0] == '[') {
            if (seq[1] == 'A') { // Up Arrow
                if (selected_idx > 0) selected_idx--;
            } 
            else if (seq[1] == 'B') { // Down Arrow
                if (selected_idx < file_count - 1) selected_idx++;
            }
            else if (seq[1] == 'C') { // Right Arrow (உள்ளே செல்ல)
                if (files[selected_idx].is_dir) {
                    chdir(files[selected_idx].name);
                    load_files(".");
                    selected_idx = 0;
                }
            }
            else if (seq[1] == 'D') { // Left Arrow (வெளியே வர)
                chdir(".."); 
                load_files(".");
                selected_idx = 0;
            }
        }
    } 
    else {
        if (c == 'q') {
            return 0; // Q அழுத்தினால் வெளியேற சிக்னல்
        } 
        else if (c == 'w' || c == 'k') { 
            if (selected_idx > 0) selected_idx--;
        } 
        else if (c == 's' || c == 'j') { 
            if (selected_idx < file_count - 1) selected_idx++;
        } 
        else if (c == 127 || c == 8 || c == 'b') { // Backspace
            chdir("..");
            load_files(".");
            selected_idx = 0;
        }
        else if (c == '\n' || c == '\r') { // Enter
            if (files[selected_idx].is_dir) {
                chdir(files[selected_idx].name);
                load_files(".");
                selected_idx = 0; 
            } else {
                return 2; // ஃபைலை எடிட்டரில் திறக்க சிக்னல்
            }
        }
    }
    return 1; // தொடர்ந்து Tree-ல் இயங்க சிக்னல்
}
