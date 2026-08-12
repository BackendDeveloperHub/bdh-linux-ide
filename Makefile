CC = gcc
CFLAGS = -Iinclude -Wall -Wextra -O3
LDFLAGS = -lpq

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# ப்ராஜெக்ட்டில் உள்ள அனைத்து கோர் மாட்யூல்களும்
OBJS = $(OBJ_DIR)/bdh-tree.o \
       $(OBJ_DIR)/bdh-edit.o \
       $(OBJ_DIR)/bdh-db.o \
       $(OBJ_DIR)/main.o

TARGET = $(BIN_DIR)/bdh-linux-ide

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "🔥 BDH-IDE Build Successful! Run with: ./bin/bdh-linux-ide"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "🧹 Workspace cleaned."

.PHONY: all clean
