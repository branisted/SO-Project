#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void start_monitor() {
    printf("Monitor start test.\n");
}

void stop_monitor() {
    printf("Monitor stop test.\n");
}

void list_hunts() {
    printf("List hunts test.\n");
}

void list_treasures(char *game_name) {
    printf("List treasures test for game: %s\n", game_name);
}

void view_treasure(char *game, char *treasure) {
    printf("View treasure test for game: %s, treasure: %s\n", game, treasure);
}

int main() {
    char input[128];

    printf("Treasure Hub:\n");

    while (1) {
        printf("\n> ");
        fflush(stdout);

        if (!fgets(input, sizeof(input), stdin))
            break;

        input[strcspn(input, "\n")] = '\0'; // stergere caracterul \n de la urma stringului de input

        if (strcmp(input, "start_monitor") == 0) {
            start_monitor();

        } else if (strcmp(input, "stop_monitor") == 0) {
            stop_monitor();

        } else if (strcmp(input, "list_hunts") == 0) {
            list_hunts();

        } else if (strncmp(input, "list_treasures ", 15) == 0) {
            char game_name[64];
            sscanf(input + 15, "%63s", game_name);
            list_treasures(game_name);

        } else if (strncmp(input, "view_treasure ", 14) == 0) {
            char game[64], treasure[64];
            sscanf(input + 14, "%63s %63s", game, treasure);
            view_treasure(game, treasure);

        } else if (strcmp(input, "exit") == 0) {
            if (1 == 0) { // trebuie adaugata integrarea cu start_monitor
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