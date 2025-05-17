#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>

#define MAX_TXT_SIZE 256
#define TMP_FOLDER "tmp"
#define CMD_FILE_PATH "tmp/command.txt"

pid_t monitor_pid = -1;
int monitor_shutting_down = 0;

void setup_tmp_folder() {
    struct stat st;

    if (stat(TMP_FOLDER, &st) == 0 && S_ISDIR(st.st_mode)) {
        DIR *dir = opendir(TMP_FOLDER);
        struct dirent *entry;
        char path[MAX_TXT_SIZE];

        if (dir) {
            while ((entry = readdir(dir)) != NULL) {
                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                    continue;

                snprintf(path, sizeof(path), "%s/%s", TMP_FOLDER, entry->d_name);
                remove(path);
            }
            closedir(dir);
        }

        rmdir(TMP_FOLDER);
    }

    mkdir(TMP_FOLDER, 0777);
}

int start_monitor() {
    monitor_pid = fork();

    if (monitor_pid == 0) {
        execl("./monitor", "monitor", NULL);
        perror("execl failed");
        exit(1);
    } else if (monitor_pid > 0) {
        printf("\nMonitor started (PID %d)\n", monitor_pid);
        fflush(stdout);
    } else {
        perror("Fork failed.");
        exit(1);
    }

    return 1;
}

int stop_monitor() {

    if (monitor_shutting_down) {
        printf("Monitor is already shutting down.\n");
        return 0;
    }

    if (kill(monitor_pid, SIGUSR2) < 0) {
        perror("Error sending SIGUSR2 to monitor");
        return 0;
    }

    monitor_shutting_down = 1;
    printf("Stop signal sent to monitor (PID %d). Waiting for it to exit...\n", monitor_pid);
    return 1;
}

int list_hunts() {

    if (monitor_pid <= 0) {
        printf("Monitor is not running.\n");
        return 0;
    }

    FILE *cmd_file = fopen(CMD_FILE_PATH, "w");
    if (!cmd_file) {
        perror("Failed to write command");
        return 0;
    }

    fprintf(cmd_file, "list_hunts");
    fclose(cmd_file);

    kill(monitor_pid, SIGUSR1);
    return 1;
}

int list_treasures(char* game_name) {
    if (monitor_pid <= 0) {
        printf("Monitor is not running.\n");
        return 0;
    }

    FILE *cmd_file = fopen(CMD_FILE_PATH, "w");
    if (!cmd_file) {
        perror("Failed to write command");
        return 0;
    }

    fprintf(cmd_file, "list_treasures %s", game_name);
    fclose(cmd_file);

    kill(monitor_pid, SIGUSR1);
    return 1;
}

int view_treasure(char* game, char* treasure) {
    if (monitor_pid <= 0) {
        printf("Monitor is not running.\n");
        return 0;
    }

    FILE *cmd_file = fopen(CMD_FILE_PATH, "w");
    if (!cmd_file) {
        perror("Failed to write command");
        return 0;
    }

    fprintf(cmd_file, "view_treasure %s %s", game, treasure);
    fclose(cmd_file);

    kill(monitor_pid, SIGUSR1);
    return 1;
}

void handle_sigchld(int sig) {
    int status;
    pid_t pid = waitpid(-1, &status, WNOHANG);
    if (pid == monitor_pid) {
        printf("[Treasure Hub] Monitor (PID %d) exited with code %d.\n", pid, WEXITSTATUS(status));
        monitor_pid = -1;
        monitor_shutting_down = 0;
    }
}

int main() {
    char input[128];

    setup_tmp_folder();

    struct sigaction sa_chld;
    memset(&sa_chld, 0, sizeof(sa_chld));
    sa_chld.sa_handler = handle_sigchld;
    sigaction(SIGCHLD, &sa_chld, NULL);


    printf("Treasure Hub:\n");

    while (1) {
        printf("\n> ");
        fflush(stdout);

        if (!fgets(input, sizeof(input), stdin))
            break;

        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "start_monitor") == 0) {

            if (monitor_shutting_down) {
                printf("Monitor is shutting down. Please wait...\n");
                continue;
            }

            start_monitor();

        } else if (strcmp(input, "stop_monitor") == 0) {

            if (monitor_shutting_down) {
                printf("Monitor is shutting down. Please wait...\n");
                continue;
            }

            stop_monitor();

        } else if (strcmp(input, "list_hunts") == 0) {

            if (monitor_shutting_down) {
                printf("Monitor is shutting down. Please wait...\n");
                continue;
            }

            list_hunts();

        } else if (strncmp(input, "list_treasures ", 15) == 0) {

            if (monitor_shutting_down) {
                printf("Monitor is shutting down. Please wait...\n");
                continue;
            }

            char game_name[MAX_TXT_SIZE];
            sscanf(input + 15, "%63s", game_name);
            list_treasures(game_name);

        } else if (strncmp(input, "view_treasure ", 14) == 0) {

            if (monitor_shutting_down) {
                printf("Monitor is shutting down. Please wait...\n");
                continue;
            }

            char game[MAX_TXT_SIZE], treasure[MAX_TXT_SIZE];
            sscanf(input + 14, "%s %s", game, treasure);
            view_treasure(game, treasure);

        } else if (strcmp(input, "exit") == 0) {
            break;

        } else {
            printf("Unknown command: %s", input);
        }
    }

    return 0;
}