#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pthread.h>

#define MAX_TXT_SIZE 256
#define TMP_FOLDER "tmp"
#define CMD_FILE_PATH "tmp/command.txt"

pid_t monitor_pid = -1;
int monitor_shutting_down = 0;
int pipe_fds[2] = { -1, -1 };  // [0] read, [1] write
pthread_t reader_thread;

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

// Citire din pipe si print la stdout
void* reader_routine(void *arg) {
    char buf[512];
    ssize_t n;
    while ((n = read(pipe_fds[0], buf, sizeof(buf)-1)) > 0) {
        buf[n] = '\0';
        printf("%s", buf);
        fflush(stdout);
    }
    return NULL;
}

int start_monitor() {
    if (pipe(pipe_fds) < 0) {
        perror("pipe"); exit(1);
    }

    monitor_pid = fork();

    if (monitor_pid == 0) {
        // Monitor
        close(pipe_fds[0]); // Inchide capatul de citire
        dup2(pipe_fds[1], STDOUT_FILENO); // Redirectare stdout spre pipe
        dup2(pipe_fds[1], STDERR_FILENO); // Redirectare stderr spre pipe
        close(pipe_fds[1]);
        execl("./monitor", "monitor", NULL);
        perror("execl failed");
        exit(1);
    } else if (monitor_pid > 0) {
        // Hub
        close(pipe_fds[1]); // Inchide capatul de scriere
        // Citire
        if (pthread_create(&reader_thread, NULL, reader_routine, NULL) != 0) {
            perror("pthread_create");
            exit(1);
        }
        printf("\nMonitor started (PID %d)\n", monitor_pid);
        fflush(stdout);
    } else {
        perror("Fork failed.");
        exit(1);
    }

    return 1;
}

void stop_monitor() {
    if (monitor_pid <= 0) {
        printf("[Treasure Hub] No monitor is running.\n");
        return;
    }

    printf("[Treasure Hub] Stopping monitor (PID %d)...\n", monitor_pid);
    kill(monitor_pid, SIGUSR2);
    monitor_shutting_down = 1;

    int status;
    waitpid(monitor_pid, &status, 0);
    // Inchide capatul de citire
    close(pipe_fds[0]);
    pthread_join(reader_thread, NULL);

    printf("[Treasure Hub] Monitor (PID %d) exited with code %d.\n", monitor_pid, WEXITSTATUS(status));
    monitor_pid = -1;
    monitor_shutting_down = 0;
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
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (pid == monitor_pid) {
            printf("[Treasure Hub] Monitor (PID %d) exited with code %d.\n", pid, WEXITSTATUS(status));
            monitor_pid = -1;
            monitor_shutting_down = 0;
        } else {
            printf("[Treasure Hub] Child process (PID %d) exited.\n", pid);
        }
    }
}

void setup_sigchld_handler() {
    struct sigaction sa_chld;
    sa_chld.sa_handler = handle_sigchld;
    sigemptyset(&sa_chld.sa_mask);
    sa_chld.sa_flags = SA_RESTART;

    if (sigaction(SIGCHLD, &sa_chld, NULL) == -1) {
        perror("sigaction SIGCHLD");
    }
}

int main() {
    char input[128];

    setup_tmp_folder();

    setup_sigchld_handler();

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