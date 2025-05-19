#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <dirent.h>
#include "treasure_mgr_lib.h"

#define MAX_TXT_SIZE 256
#define CMD_FILE_PATH "tmp/command.txt"

void handle_terminate_signal(int sig) {
    printf("[Monitor] Received termination signal. Exiting...\n");
    fflush(stdout);
    exit(0);
}

void handle_command_input(int sig) {
    FILE *cmd_file = fopen(CMD_FILE_PATH, "r");
    if (!cmd_file) {
        perror("[Monitor] Failed to open command file");
        return;
    }

    char cmd[MAX_TXT_SIZE];
    if (fgets(cmd, MAX_TXT_SIZE, cmd_file)) {
        cmd[strcspn(cmd, "\n")] = '\0';  // remove newline

        printf("\n[Monitor] Executing command: %s\n\n", cmd);
        fflush(stdout);

        pid_t pid = fork();
        if (pid == 0) {

            if (strstr(cmd, "list_hunts")) {

                execlp("./treasure_manager", "treasure_manager", "--list_hunts", NULL);
                perror("execlp failed");
                exit(1);

            } else if (strncmp(cmd, "list_treasures", 14) == 0) {

                char game_name[MAX_TXT_SIZE];
                sscanf(cmd + 15, "%s", game_name);
                execlp("./treasure_manager", "treasure_manager", "--list", game_name, NULL);
                perror("execlp failed");
                exit(1);

            } else if (strncmp(cmd, "view_treasure", 13) == 0) {

                char game[MAX_TXT_SIZE], treasure[MAX_TXT_SIZE];
                sscanf(cmd + 14, "%s %s", game, treasure);
                execlp("./treasure_manager", "treasure_manager", "--view", game, treasure, NULL);
                perror("execlp failed");
                exit(1);

            } else if (strcmp(cmd, "calculate_score") == 0) {

                execlp("./calculate_score", "calculate_score", NULL);
                perror("execlp failed");
                exit(1);
                        
            } else {

                printf("[Monitor] Unknown command: %s\n", cmd);
                exit(1);

            }
        } else if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
        } else {
            perror("fork failed");
        }
    }
    fclose(cmd_file);
}

int main() {
    struct sigaction sa_term, sa_exec;

    memset(&sa_term, 0, sizeof(sa_term));
    sa_term.sa_handler = handle_terminate_signal;
    sigaction(SIGUSR2, &sa_term, NULL);

    memset(&sa_exec, 0, sizeof(sa_exec));
    sa_exec.sa_handler = handle_command_input;
    sigaction(SIGUSR1, &sa_exec, NULL);

    printf("[Monitor] Running. PID: %d\n", getpid());
    fflush(stdout);

    while (1) {
        pause();
    }

    return 0;
}