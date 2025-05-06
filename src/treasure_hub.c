#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#define MAX_TXT_SIZE 256

int shutting_down = 0; // flag pentru testarea introducerii comenzilor in monitor dupa ce a primit semnalul de terminare 

void write_command(const char *command) {
    FILE *fp = fopen("command.txt", "w");
    if (!fp) {
        perror("Failed to write to command.txt");
        return;
    }
    fprintf(fp, "%s\n", command);
    fclose(fp);
}

void read_result() {
    FILE *fp = fopen("result.txt", "r");
    if (!fp) {
        perror("Failed to read result.txt");
        return;
    }

    char line[MAX_TXT_SIZE];
    while (fgets(line, sizeof(line), fp)) {
        printf("%s", line);
    }
    fclose(fp);
}


int check_monitor_running(int monitor_running) {
    if (shutting_down) {
        printf("Monitor is shutting down. Please wait...\n");
     
        return 0;
    }

    if (!monitor_running) {
        printf("Start the monitor first using start_monitor.\n");
        return 0;
    }
    return 1;
}

void handle_command_signal(int sig) {
    FILE *cmd_fp = fopen("command.txt", "r");
    if (!cmd_fp) {
        perror("Monitor: Failed to read command.txt");
        return;
    }

    char command[MAX_TXT_SIZE];
    fgets(command, sizeof(command), cmd_fp);
    fclose(cmd_fp);

    command[strcspn(command, "\n")] = '\0';

    char system_cmd[MAX_TXT_SIZE];

    if (strncmp(command, "list_hunts", 10) == 0) {
        snprintf(system_cmd, sizeof(system_cmd), "./treasure_manager --list_hunts > result.txt");

    } else if (strncmp(command, "list_treasures", 14) == 0) {
        char game[MAX_TXT_SIZE];
        sscanf(command, "list_treasures %s", game);
        snprintf(system_cmd, sizeof(system_cmd), "./treasure_manager --list %s > result.txt", game);

    } else if (strncmp(command, "view_treasure", 13) == 0) {
        char game[MAX_TXT_SIZE], treasure[MAX_TXT_SIZE];
        sscanf(command, "view_treasure %s %s", game, treasure);
        snprintf(system_cmd, sizeof(system_cmd), "./treasure_manager --view %s %s > result.txt", game, treasure);

    } else {
        snprintf(system_cmd, sizeof(system_cmd), "echo Invalid command > result.txt");
    }

    system(system_cmd);
}

void handle_terminate_signal(int sig) {
    printf("\n[Monitor] Received termination signal. Exiting...\n");
    exit(0);
}

void start_monitor(pid_t* monitor_pid, int* monitor_running) {
    if (*monitor_running) {
        printf("Monitor already running.\n");
        return;
    }

    *monitor_pid = fork();

    if (*monitor_pid == 0) {
        signal(SIGUSR1, handle_command_signal);
        signal(SIGUSR2, handle_terminate_signal);

        printf("[Monitor] Running. PID: %d\n", getpid());
        fflush(stdout);

        while (1) {
            pause();
        }

        exit(0);
    } else if (*monitor_pid > 0) {
        *monitor_running = 1;
        printf("\nMonitor started (PID %d)\n", *monitor_pid);
        fflush(stdout);
    } else {
        perror("fork failed");
    }
}

void stop_monitor(pid_t* monitor_pid, int* monitor_running) {
    if (!check_monitor_running(*monitor_running))
        return;

    shutting_down = 1;

    kill(*monitor_pid, SIGUSR2);
    waitpid(*monitor_pid, NULL, 0);
    printf("Monitor stopped.\n");

    *monitor_running = 0;
    *monitor_pid = -1;
}

void list_hunts(pid_t* monitor_pid, int* monitor_running) {
    if (!check_monitor_running(*monitor_running))
        return;

    write_command("list_hunts");
    kill(*monitor_pid, SIGUSR1);
    sleep(1);
    read_result();
}

void list_treasures(pid_t* monitor_pid, int* monitor_running, char *game_name) {
    if (!check_monitor_running(*monitor_running))
        return;

    char command[MAX_TXT_SIZE];
    snprintf(command, sizeof(command), "list_treasures %s", game_name);
    write_command(command);
    kill(*monitor_pid, SIGUSR1);
    sleep(1);
    read_result();
}

void view_treasure(pid_t* monitor_pid, int* monitor_running, char *game, char *treasure) {
    if (!check_monitor_running(*monitor_running))
        return;

    char command[MAX_TXT_SIZE];
    snprintf(command, sizeof(command), "view_treasure %s %s", game, treasure);
    write_command(command);
    kill(*monitor_pid, SIGUSR1);
    sleep(1);
    read_result();
}

int main() {
    char input[128];

    pid_t monitor_pid = -1;
    int monitor_running = 0;

    printf("Treasure Hub:\n");

    while (1) {
        printf("\n> ");
        fflush(stdout);

        if (!fgets(input, sizeof(input), stdin))
            break;

        input[strcspn(input, "\n")] = '\0'; // stergere caracterul \n de la urma stringului de input

        if (strcmp(input, "start_monitor") == 0) {
            start_monitor(&monitor_pid, &monitor_running);
            usleep(100000); // ca sa evit cazul in care ">" se printeaza mai repede decat printul din procesul copil
        } else if (strcmp(input, "stop_monitor") == 0) {
            stop_monitor(&monitor_pid, &monitor_running);

        } else if (strcmp(input, "list_hunts") == 0) {
            list_hunts(&monitor_pid, &monitor_running);

        } else if (strncmp(input, "list_treasures ", 15) == 0) {
            char game_name[MAX_TXT_SIZE];
            sscanf(input + 15, "%63s", game_name);
            list_treasures(&monitor_pid, &monitor_running, game_name);

        } else if (strncmp(input, "view_treasure ", 14) == 0) {
            char game[MAX_TXT_SIZE], treasure[MAX_TXT_SIZE];
            sscanf(input + 14, "%s %s", game, treasure);
            view_treasure(&monitor_pid, &monitor_running, game, treasure);

        } else if (strcmp(input, "exit") == 0) {
            if (monitor_running) {
                printf("Please stop the monitor before exiting.\n");
            } else {
                remove("command.txt");
                remove("result.txt");
                break;
            }

        } else {
            printf("Unknown command: %s\n", input);
        }
    }

    return 0;
}