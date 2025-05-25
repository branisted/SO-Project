#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <time.h>
#include "treasure_mgr_lib.h"

void open_binary_dir(char* hunt_id, char* new_path) {
    char dir_path[MAX_TXT_SIZE];
    snprintf(dir_path, sizeof(dir_path), "%s/%s", BASE_DIR, hunt_id);

    DIR *dir = opendir(dir_path);
    if (!dir) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    struct dirent* dr;
    struct stat buffer;
    char bin_path[MAX_TXT_SIZE];

    while ((dr = readdir(dir))) {
        snprintf(bin_path, sizeof(bin_path), "%s/%s", dir_path, dr->d_name);
        if (stat(bin_path, &buffer) == 0 && S_ISREG(buffer.st_mode)) {
            break;
        }
    }

    closedir(dir);
    strcpy(new_path, bin_path);
}

void log_operation(const char* hunt_id, const char* message) {
    char log_path[MAX_TXT_SIZE];
    snprintf(log_path, sizeof(log_path), "%s/%s/logged_hunt", BASE_DIR, hunt_id);

    int fd = open(log_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0) {
        perror("log open");
        return;
    }

    time_t rawtime;
    struct tm* timeinfo;
    char time_str[64];

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(time_str, sizeof(time_str), "[%Y-%m-%d %H:%M:%S]", timeinfo);

    dprintf(fd, "%s %s\n", time_str, message);
    close(fd);
}

void exists_hunt_dir(const char* hunt_id) {
    char hunt_path[MAX_TXT_SIZE];
    snprintf(hunt_path, sizeof(hunt_path), "%s/%s", BASE_DIR, hunt_id);

    mkdir(BASE_DIR, 0755);
    mkdir(hunt_path, 0755);

    char linkname[MAX_TXT_SIZE];
    snprintf(linkname, sizeof(linkname), "logged_hunt-%s", hunt_id);

    char target[MAX_TXT_SIZE];
    snprintf(target, sizeof(target), "%s/%s/logged_hunt", BASE_DIR, hunt_id);

    if (access(linkname, F_OK) == -1) {
        symlink(target, linkname);
    }
}

void add(char* hunt_id) {
    exists_hunt_dir(hunt_id);

    char bin_path[MAX_TXT_SIZE];
    snprintf(bin_path, sizeof(bin_path), "%s/%s/treasures.bin", BASE_DIR, hunt_id);

    TREASURE_T t;
    printf("Enter Treasure ID: ");
    fgets(t.TreasureID, MAX_TXT_SIZE, stdin); 
    t.TreasureID[strcspn(t.TreasureID, "\n")] = 0;
    printf("Enter Username: ");
    fgets(t.Username, MAX_TXT_SIZE, stdin); 
    t.Username[strcspn(t.Username, "\n")] = 0;
    printf("Enter Coordinates (x y): ");
    scanf("%lf %lf", &t.GPScoords.x, &t.GPScoords.y);
    getchar();
    printf("Enter Clue: ");
    fgets(t.Clue, MAX_TXT_SIZE, stdin); 
    t.Clue[strcspn(t.Clue, "\n")] = 0;
    printf("Enter Value: ");
    scanf("%d", &t.Value);
    getchar();

    int fd = open(bin_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0) {
        perror("Eroare open fisier binar.");
        return;
    }

    write(fd, &t, sizeof(TREASURE_T));
    close(fd);

    char log_msg[512];
    if (snprintf(log_msg, sizeof(log_msg), "ADD Treasure ID: %s, User: %s, Value: %d", t.TreasureID, t.Username, t.Value) >= sizeof(log_msg)) {
        fprintf(stderr, "Log message too long in add().");
    }
    log_operation(hunt_id, log_msg);
}

void list(char* hunt_id) {
    char bin_path[MAX_TXT_SIZE];
    open_binary_dir(hunt_id, bin_path);

    int fd = open(bin_path, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "Error: Treasure file for hunt '%s' does not exist or cannot be opened.\n", hunt_id);
        perror("open");
        log_operation(hunt_id, "LIST treasures failed - file not found");
        return;
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        perror("fstat");
        close(fd);
        return;
    }

    char mod_time[64];
    strftime(mod_time, sizeof(mod_time), "%Y-%m-%d %H:%M:%S", localtime(&st.st_mtime));

    printf("Hunt name: %s\nTotal file size: %ld bytes\nLast modification time: %s\n", hunt_id, st.st_size, mod_time);

    TREASURE_T t;
    int i = 0;

    while (read(fd, &t, sizeof(TREASURE_T)) == sizeof(TREASURE_T)) {
        printf("\nTreasure %d:\n", ++i);
        printf("Treasure ID: %s\n", t.TreasureID);
        printf("User name: %s\n", t.Username);
        printf("GPS coordinates (x y): %lf %lf\n", t.GPScoords.x, t.GPScoords.y);
        printf("Clue text: %s\n", t.Clue);
        printf("Value: %d\n", t.Value);
    }

    close(fd);
    log_operation(hunt_id, "LIST treasures");
}

void view(char* hunt_id, char* treasure_id) {
    char bin_path[MAX_TXT_SIZE];
    open_binary_dir(hunt_id, bin_path);

    int fd = open(bin_path, O_RDONLY);
    if (fd < 0) {
        perror("open view");
        return;
    }

    TREASURE_T t;
    while (read(fd, &t, sizeof(TREASURE_T)) == sizeof(TREASURE_T)) {
        if (!strcmp(t.TreasureID, treasure_id)) {
            printf("Treasure ID: %s\n", t.TreasureID);
            printf("User name: %s\n", t.Username);
            printf("GPS coordinates (x y): %lf %lf\n", t.GPScoords.x, t.GPScoords.y);
            printf("Clue text: %s\n", t.Clue);
            printf("Value: %d\n", t.Value);
            close(fd);
            char log_msg[MAX_TXT_SIZE];
            snprintf(log_msg, sizeof(log_msg), "VIEW treasure %s in hunt %s", treasure_id, hunt_id);
            log_operation(hunt_id, log_msg);
            return;
        }
    }

    printf("Treasure not found!\n");
    close(fd);
    char log_msg[MAX_TXT_SIZE];
    snprintf(log_msg, sizeof(log_msg), "VIEW treasure %s in hunt %s - TREASURE NOT FOUND", treasure_id, hunt_id);
    log_operation(hunt_id, log_msg);
}

void remove_hunt(char* hunt_id) {
    char hunt_path[MAX_TXT_SIZE];
    snprintf(hunt_path, sizeof(hunt_path), "%s/%s", BASE_DIR, hunt_id);

    DIR* dir = opendir(hunt_path);
    if (!dir) {
        perror("Failed to open hunt directory");
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;

        char file_path[MAX_TXT_SIZE];
        if (snprintf(file_path, sizeof(file_path), "%s/%s", hunt_path, entry->d_name) >= sizeof(file_path)) {
            fprintf(stderr, "File path too long in remove_hunt().");
            continue;
        }

        if (unlink(file_path) != 0) {
            perror("Failed to remove a file inside the hunt directory");
        }
    }
    closedir(dir);

    if (rmdir(hunt_path) != 0) {
        perror("Failed to remove the hunt directory");
    }

    char linkname[MAX_TXT_SIZE];
    snprintf(linkname, sizeof(linkname), "logged_hunt-%s", hunt_id);
    unlink(linkname);

    printf("Hunt '%s' has been completely removed.\n", hunt_id);
}

void remove_treasure(char* hunt_id, char* treasure_id) {
    char bin_path[MAX_TXT_SIZE];
    open_binary_dir(hunt_id, bin_path);

    char tmp_bin_path[MAX_TXT_SIZE];
    snprintf(tmp_bin_path, sizeof(tmp_bin_path), "%s/%s/tmp_treasures.bin", BASE_DIR, hunt_id);

    int fd = open(bin_path, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open original treasure file");
        return;
    }

    int tmp_fd = open(tmp_bin_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (tmp_fd < 0) {
        perror("Failed to open temporary file");
        close(fd);
        return;
    }

    TREASURE_T t;
    int found = 0, count_remaining = 0;

    while (read(fd, &t, sizeof(TREASURE_T)) == sizeof(TREASURE_T)) {
        if (!strcmp(t.TreasureID, treasure_id)) {
            found = 1;
            continue;
        }
        write(tmp_fd, &t, sizeof(TREASURE_T));
        count_remaining++;
    }

    close(fd);
    close(tmp_fd);

    if (!found) {
        printf("Treasure not found!\n");
        char log_msg[MAX_TXT_SIZE];
        snprintf(log_msg, sizeof(log_msg), "REMOVE treasure %s in hunt %s - Treasure not found!", treasure_id, hunt_id);
        log_operation(hunt_id, log_msg);
        unlink(tmp_bin_path);
        return;
    }

    if (count_remaining == 0) {
        unlink(tmp_bin_path);
        remove_hunt(hunt_id);
        return;
    }

    if (rename(tmp_bin_path, bin_path) != 0) {
        perror("Error replacing original file with updated file");
        unlink(tmp_bin_path);
        return;
    }

    char log_msg[MAX_TXT_SIZE];
    snprintf(log_msg, sizeof(log_msg), "REMOVE treasure %s in hunt %s", treasure_id, hunt_id);
    log_operation(hunt_id, log_msg);
}

void list_hunts() {
    DIR *dir = opendir(BASE_DIR);
    if (!dir) {
        perror("Could not open base directory");
        return;
    }

    struct dirent *entry;
    struct stat st;
    char path[MAX_TXT_SIZE];

    printf("Hunts:\n");

    while ((entry = readdir(dir)) != NULL) {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;

        if (snprintf(path, sizeof(path), "%s/%s", BASE_DIR, entry->d_name) >= sizeof(path)) {
            fprintf(stderr, "Path too long in list_hunts().");
            continue;
        }

        if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) {
            printf(" - %s\n", entry->d_name);
        }
    }

    closedir(dir);
}