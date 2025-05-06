#ifndef TREASURE_MGR_LIB_H
#define TREASURE_MGR_LIB_H

#define MAX_TXT_SIZE 256
#define BASE_DIR "hunts"

typedef struct COORDS {
    double x;
    double y;
} COORDS_T;

typedef struct TREASURE {
    char TreasureID[MAX_TXT_SIZE];
    char Username[MAX_TXT_SIZE];
    COORDS_T GPScoords;
    char Clue[MAX_TXT_SIZE];
    int Value;
} TREASURE_T;

void add(char* hunt_id);
void list(char* hunt_id);
void view(char* hunt_id, char* treasure_id);
void remove_treasure(char* hunt_id, char* treasure_id);
void remove_hunt(char* hunt_id);
void list_hunts();

#endif
