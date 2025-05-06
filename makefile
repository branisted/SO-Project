CC = gcc
CFLAGS = -Wall -Wextra -g
SRC_DIR = ./src

TMGR_OBJS = $(SRC_DIR)/treasure_manager.c $(SRC_DIR)/treasure_mgr_lib.c
THUB_OBJS = $(SRC_DIR)/treasure_hub.c

tmgr: $(TMGR_OBJS)
	$(CC) $(CFLAGS) -o treasure_manager $(TMGR_OBJS)

thub: $(THUB_OBJS)
	$(CC) $(CFLAGS) -o treasure_hub $(THUB_OBJS)

clean:
	rm -f treasure_manager treasure_hub
