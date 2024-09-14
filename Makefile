# Variablen für Kompilierungsflags und Dateinamen
CC = emcc
FLAGS = -Os -Wall -DPLATFORM_WEB
LIBRARY = ./lib/libraylib.a
INCLUDE_DIR = ./include
SHELL_FILE = ./minshell.html
GLFW = -s USE_GLFW=3
OUTPUT_DIR = build
TARGET = game
SRC = ./src/main.c

# Ziel: HTML-Datei erstellen
$(OUTPUT_DIR)/$(TARGET).html: $(OUTPUT_DIR) $(SRC) $(SHELL_FILE)
	$(CC) -o $(OUTPUT_DIR)/$(TARGET).html $(SRC) $(FLAGS) $(LIBRARY) -I$(INCLUDE_DIR) $(GLFW) -s --shell-file $(SHELL_FILE) -s TOTAL_MEMORY=67108864 --preload-file ./resources --profiling

$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)

# Ziel: Bereinigung der erstellten Dateien
clean:
	rm -rf $(OUTPUT_DIR)

