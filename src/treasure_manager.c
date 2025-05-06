#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "treasure_mgr_lib.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage error: treasure_manager <operation> <values>\n.");
        exit(-1);
    }

    if (!strcmp(argv[1], "--add")) {
        if (argc < 3) {
            printf("Usage error: treasure_manager add <hunt_id>\n.");
            exit(-1);
        }
        add(argv[2]);
    } else if (!strcmp(argv[1], "--list")) {
        if (argc < 3) {
            printf("Usage error: treasure_manager list <hunt_id>\n.");
            exit(-1);
        }
        list(argv[2]);
    } else if (!strcmp(argv[1], "--view")) {
        if (argc < 4) {
            printf("Usage error: treasure_manager view <hunt_id> <treasure_id>\n.");
            exit(-1);
        }
        view(argv[2], argv[3]);
    } else if (strstr(argv[1], "--remove_treasure")) {
        if (argc < 4) {
            printf("Usage error: treasure_manager remove_treasure <hunt_id> <treasure_id>\n.");
            exit(-1);
        }
        remove_treasure(argv[2], argv[3]);
    } else if (!strcmp(argv[1], "--remove_hunt")) {
        if (argc < 3) {
            printf("Usage error: treasure_manager remove_hunt <hunt_id>\n.");
            exit(-1);
        }
        remove_hunt(argv[2]);
    } else if (!strcmp(argv[1], "--list_hunts")) {
        list_hunts();
    } else {
        printf("Usage error:\nadd <hunt_id>\nlist <hunt_id>\nview <hunt_id> <id>\nremove_treasure <hunt_id> <id>\nremove_hunt <hunt_id>\nlist_hunts\n");
        exit(-1);
    }

    return 0;
}
