CC = gcc
CFLAGS = -Wall -lpthread

INCLUDES = -I. -Iheaders

SRCS = external/cJSON.c main.c structures/RoomTable.c control/Setup.c structures/SoulWorker.c \
		structures/Maze.c helpers/Error.c control/Keyboard.c \
		control/SaveLoad.c helpers/itoa.s structures/DArray.c helpers/Misc.c control/Battle.c

HEADERS = headers/cJSON.h headers/Setup.h headers/SoulWorker.h headers/Maze.h headers/Error.h \
		headers/Keyboard.h headers/SaveLoad.h headers/LoadJSON.h headers/DArray.h headers/Misc.h \
		headers/Battle.h headers/Colors.h

OBJS = $(SRCS:.c=.o)
OBJS := $(OBJS:.s=.o)

TARGET = clisw
PACKAGE_DIR = CLISW
PACKAGE_NAME = $(TARGET)_build.zip
INSTALLER = clisw-installer
LAUNCHER = clisw-launcher

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) -lm

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

%.o: %.c
	$(CC) $< -o $@

debug: CFLAGS += -g -O0
debug: $(TARGET)

launcher:
	$(CC) $(CFLAGS) launcher.c -o $(LAUNCHER) -lcurl

installer:
	$(CC) $(CFLAGS) installer.c -o $(INSTALLER)

clean:
	rm -f $(OBJS) $(TARGET)
	rm -rf $(PACKAGE_DIR)
	rm -f $(PACKAGE_NAME)
	rm -f $(LAUNCHER)
	rm -f $(INSTALLER)

	rm -rf CLISW_GAME

package: all installer launcher
	mkdir -p $(PACKAGE_DIR)
	cp $(TARGET) $(PACKAGE_DIR)/
	cp $(LAUNCHER) $(PACKAGE_DIR)/
	cp version $(PACKAGE_DIR)/
	mkdir -p $(PACKAGE_DIR)/data/maps
	mkdir -p $(PACKAGE_DIR)/data/misc
	mkdir -p $(PACKAGE_DIR)/data/story
	mkdir -p $(PACKAGE_DIR)/data/story/best_showtime
	mkdir -p $(PACKAGE_DIR)/data/story/control_zone
	mkdir -p $(PACKAGE_DIR)/data/story/r_square
	mkdir -p $(PACKAGE_DIR)/data/story/tower_of_greed
	
	rsync -av --include='*.json' --exclude='*' data/maps/ $(PACKAGE_DIR)/data/maps/
	rsync -av --include='*.dat' --exclude='*' data/misc/ $(PACKAGE_DIR)/data/misc/
	rsync -av --include='intro.story' --exclude='*' data/story/ $(PACKAGE_DIR)/data/story/
	rsync -av --include='*.story' --exclude='*' data/story/best_showtime/ $(PACKAGE_DIR)/data/story/best_showtime/
	rsync -av --include='*.story' --exclude='*' data/story/control_zone/ $(PACKAGE_DIR)/data/story/control_zone/
	rsync -av --include='*.story' --exclude='*' data/story/r_square/ $(PACKAGE_DIR)/data/story/r_square/
	rsync -av --include='*.story' --exclude='*' data/story/tower_of_greed/ $(PACKAGE_DIR)/data/story/tower_of_greed/

	zip -r $(PACKAGE_NAME) $(PACKAGE_DIR)

.PHONY: all debug clean package