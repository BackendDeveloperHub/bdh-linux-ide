#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

// ஃபைல்களை ட்ரீ வடிவத்தில் பிரிண்ட் செய்யும் ஃபங்ஷன்
void print_tree(const char *path, int level) {
    DIR *dir = opendir(path);
    if (!dir) return;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // மறைக்கப்பட்ட ஃபைல்கள் (. மற்றும் ..) வேண்டாம் என்றால்:
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        // Tree கிளைகளை (Branches) வரைதல்
        for (int i = 0; i < level; i++) {
            printf("│   ");
        }
        printf("├── ");

        // ஃபைலா அல்லது போல்டரா என்று சரிபார்த்தல் (stat)
        struct stat statbuf;
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        
        if (stat(full_path, &statbuf) == 0) {
            if (S_ISDIR(statbuf.st_mode)) {
                // போல்டராக இருந்தால் Cyberpunk Blue நிறத்தில் காட்டவும்
                printf("\033[1;36m%s/\033[0m\n", entry->d_name);
                // போல்டருக்குள் உள்ளே சென்று படிக்க வேண்டுமானால் இங்கே Recursive Call செய்யலாம்
                // print_tree(full_path, level + 1); 
            } else {
                // சாதாரண ஃபைல்
                printf("%s\n", entry->d_name);
            }
        }
    }
    closedir(dir);
}

int main(int argc, char *argv[]) {
    const char *start_path = (argc > 1) ? argv[1] : ".";
    
    printf("\n\033[1;36m[ BDH Workspace: %s ]\033[0m\n", start_path);
    printf("│\n");
    
    print_tree(start_path, 0);
    
    printf("\n");
    return 0;
}
