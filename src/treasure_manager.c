#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <errno.h>

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

void open_binary_dir(char* hunt_id, char* new_path) {

  char dir_path[MAX_TXT_SIZE];
  snprintf(dir_path, sizeof(dir_path), "%s/%s", BASE_DIR, hunt_id);

  DIR *dir = opendir(dir_path);

  if (!dir)
  {
    perror("opendir");
    exit(EXIT_FAILURE);
  }

  struct dirent* dr;

  struct stat buffer;
  int status;

  char bin_path[MAX_TXT_SIZE];

  while ((dr = readdir(dir)))
  {
    snprintf(bin_path, sizeof(bin_path), "%s/%s", dir_path, dr->d_name);
    status = stat(bin_path, &buffer);

    if (S_ISREG(buffer.st_mode)) {
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

  dprintf(fd, "%s\n", message);
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
  snprintf(log_msg, sizeof(log_msg), "ADD Treasure ID: %s, User: %s, Value: %d", t.TreasureID, t.Username, t.Value);
  log_operation(hunt_id, log_msg);
}

void list(char* hunt_id) {

  char bin_path[MAX_TXT_SIZE];
  open_binary_dir(hunt_id, bin_path);

  int fd = open(bin_path, O_RDONLY);
    
  if (fd < 0) {
    perror("open list");
    return;
  }

  printf("Hunt name: %s\nTotal file size: %d\nLast modification time: not yet implemented\n", hunt_id, fd);

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

  int i = 0;

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
  int found = 0;
  int count_remaining = 0;

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
      unlink(tmp_bin_path);
      return;
  }

  if (count_remaining == 0) {
      unlink(tmp_bin_path);
      remove_hunt(hunt_id);
      return;
  }

  // Overwrite fis. original
  if (rename(tmp_bin_path, bin_path) != 0) {
      perror("Error replacing original file with updated file");
      unlink(tmp_bin_path);
      return;
  }

  char log_msg[MAX_TXT_SIZE];
  snprintf(log_msg, sizeof(log_msg), "REMOVE treasure %s in hunt %s", treasure_id, hunt_id);
  log_operation(hunt_id, log_msg);
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
