#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include "treasure_mgr_lib.h"

typedef struct USERSCORE {

    char username[MAX_TXT_SIZE];
    int score;
    struct USERCORE* next;

} USERSCORE_T;

void update_user_score(USERSCORE_T** head, const char* username, int value) {
    USERSCORE_T* current = *head;
    
    while (current != NULL) {
        if (strcmp(current->username, username) == 0) {
            current->score += value;
            return;
        }
        current = current->next;
    }
    
    USERSCORE_T* new_user = (USERSCORE_T*)malloc(sizeof(USERSCORE_T));
    strcpy(new_user->username, username);
    new_user->score = value;
    new_user->next = *head;
    *head = new_user;
}

void free_user_scores(USERSCORE_T* head) {
    USERSCORE_T* temp;
    while (head != NULL) {
        temp = head;
        head = head->next;
        free(temp);
    }
}

void process_hunt(const char* hunt_id) {
    char bin_path[MAX_TXT_SIZE];
    snprintf(bin_path, sizeof(bin_path), "%s/%s/treasures.bin", BASE_DIR, hunt_id);
    
    int fd = open(bin_path, O_RDONLY);
    if (fd < 0) {
        printf("Could not open hunt '%s' file\n", hunt_id);
        return;
    }
    
    USERSCORE_T* users = NULL;
    TREASURE_T t;
    
    while (read(fd, &t, sizeof(TREASURE_T)) == sizeof(TREASURE_T)) {
        update_user_score(&users, t.Username, t.Value);
    }
    
    close(fd);
    
    printf("Scores for hunt '%s':\n", hunt_id);
    USERSCORE_T* current = users;
    while (current != NULL) {
        printf("  %s: %d points\n", current->username, current->score);
        current = current->next;
    }

    free_user_scores(users);
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        process_hunt(argv[1]);
        return 0;
    }

    DIR* dir = opendir(BASE_DIR);
    if (!dir) {
        printf("Could not open hunts directory\n");
        return 1;
    }
    
    struct dirent* entry;
    struct stat st;
    char path[MAX_TXT_SIZE];
    int hunt_count = 0;
    
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        snprintf(path, sizeof(path), "%s/%s", BASE_DIR, entry->d_name);
        if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) {
            pid_t pid = fork();
            
            if (pid == 0) {
                closedir(dir);
                process_hunt(entry->d_name);
                exit(0);
            } else if (pid < 0) {
                perror("Fork failed");
            } else {
                hunt_count++;
            }
        }
    }
    
    closedir(dir);
    
    int status;
    for (int i = 0; i < hunt_count; i++) {
        wait(&status);
    }
    
    if (hunt_count == 0) {
        printf("No hunts found\n");
    }
    
    return 0;
}