CC = gcc
CFLAGS = -Wall -Wextra -g
SRC_DIR = ./src

TMGR_LIB_OBJS = $(SRC_DIR)/treasure_mgr_lib.c
TMGR_OBJS = $(SRC_DIR)/treasure_manager.c $(TMGR_LIB_OBJS)
THUB_OBJS = $(SRC_DIR)/treasure_hub.c
TMON_OBJS = $(SRC_DIR)/monitor.c
CALC_OBJS = $(SRC_DIR)/calculate_score.c $(TMGR_LIB_OBJS)

all: $(TMGR_OBJS) $(THUB_OBJS) $(TMON_OBJS) $(CALC_OBJS)
	$(CC) $(CFLAGS) -o treasure_manager $(TMGR_OBJS)
	$(CC) $(CFLAGS) -o treasure_hub $(THUB_OBJS)
	$(CC) $(CFLAGS) -o monitor $(TMON_OBJS)
	$(CC) $(CFLAGS) -o calculate_score $(CALC_OBJS)

tmgr: $(TMGR_OBJS)
	$(CC) $(CFLAGS) -o treasure_manager $(TMGR_OBJS)

thub: $(THUB_OBJS)
	$(CC) $(CFLAGS) -o treasure_hub $(THUB_OBJS)

tmon: $(TMON_OBJS)
	$(CC) $(CFLAGS) -o monitor $(TMON_OBJS)

calc: $(CALC_OBJS)
	$(CC) $(CFLAGS) -o calculate_score $(CALC_OBJS)

clean:
	rm -f treasure_manager treasure_hub monitor calculate_score
