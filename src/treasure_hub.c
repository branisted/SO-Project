#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    char input[128];

    printf("Treasure Hub:\n");

    while (1) {
        printf("\n> ");
        fflush(stdout);

        if (!fgets(input, sizeof(input), stdin))
            break;

        input[strcspn(input, "\n")] = '\0'; // stergere caracterul \n de la urma stringului de input

        if (strcmp(input, "exit") == 0) {
            {
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