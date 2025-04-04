#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#define MAX_TXT_SIZE 256

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

void add(char* hunt_id) {
  printf("add test\n");
}

void list(char* hunt_id) {
  printf("list test\n");
}

void view(char* hunt_id, char* treasure_id) {
  printf("view test\n");
}

void remove_treasure(char* hunt_id, char* treasure_id) {
  printf("rm treasure test\n");
}

void remove_hunt(char* hunt_id) {
  printf("rm hunt test\n");
}

// hunt_id - numele directorului

int main(int argc, char** argv) {

  // 0 - numele programului - treasure_manager
  
  if (argc < 2) {
    printf("Usage error: treasure_hunt <operation> <values>\n.");
    exit(-1);
  }

  if (!strcmp(argv[1], "--add")) {
    if (argc < 3) {
      printf("Usage error: treasure_manager add <hunt_id>\n.");
      exit(-1);
    }
    add(argv[2]);
  }
  else if (!strcmp(argv[1], "--list")) {
    if (argc < 3) {
      printf("Usage error: treasure_manager list <hunt_id>\n.");
      exit(-1);
    }
    list(argv[2]);
  }
  else if (!strcmp(argv[1], "--view")) {
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
  }
  else {
    printf("Usage error:\nadd <hunt_id>\nlist <hunt_id>\nview <hunt_id> <id>\nremove_treasure <hunt_id> <id>\nremove_hunt <hunt_id>\n");
    exit(-1);
  }

  return 0;
}
