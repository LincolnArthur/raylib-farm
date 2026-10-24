# raylib-farm
#
#   make          compila o jogo (Linux, ou Windows com MinGW/w64devkit)
#   make run      compila e roda
#   make web      gera a versão web na pasta docs/ (precisa do Emscripten)
#   make clean    apaga o que foi compilado
#
# Se a raylib foi instalada pelo gerenciador de pacotes (pacman, apt, brew),
# não precisa mexer em nada. Se foi compilada à mão, informe a pasta:
#   make RAYLIB_PATH=~/raylib

CC   = gcc
SRC  = src/farmville.c
CFLAGS = -std=gnu99 -O2 -Wall -Wextra

RAYLIB_PATH ?=

ifeq ($(OS),Windows_NT)
    TARGET   = farmville.exe
    SYS_LIBS = -lopengl32 -lgdi32 -lwinmm
    # Caminho padrão do instalador oficial da raylib para Windows
    ifeq ($(RAYLIB_PATH),)
        RAYLIB_PATH = C:/raylib/raylib
    endif
else
    TARGET   = farmville
    SYS_LIBS = -lGL -lm -lpthread -ldl -lrt -lX11
endif

ifeq ($(RAYLIB_PATH),)
    RAYLIB_CFLAGS := $(shell pkg-config --cflags raylib 2>/dev/null)
    RAYLIB_LIBS   := $(shell pkg-config --libs raylib 2>/dev/null)
    ifeq ($(strip $(RAYLIB_LIBS)),)
        RAYLIB_LIBS := -lraylib
    endif
else
    RAYLIB_CFLAGS := -I$(RAYLIB_PATH)/src
    RAYLIB_LIBS   := -L$(RAYLIB_PATH)/src -lraylib
endif

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(RAYLIB_CFLAGS) $(RAYLIB_LIBS) $(SYS_LIBS)

run: $(TARGET)
	./$(TARGET)

# ---------------------------------------------------------------------------
# Versão web (WebAssembly)
#   RAYLIB_PATH     = pasta da raylib (para os headers)
#   RAYLIB_WEB_LIB  = pasta com a libraylib.a compilada com PLATFORM=PLATFORM_WEB
# Exemplo: make web RAYLIB_PATH=~/raylib RAYLIB_WEB_LIB=~/raylib/src
# ---------------------------------------------------------------------------
RAYLIB_WEB_LIB ?= $(RAYLIB_PATH)/src

web: $(SRC) web/shell.html
	mkdir -p docs
	emcc $(SRC) -o docs/index.js -Os -Wall -std=gnu99 -DPLATFORM_WEB \
		-I$(RAYLIB_PATH)/src -L$(RAYLIB_WEB_LIB) -lraylib \
		-s USE_GLFW=3 -s ASYNCIFY -s TOTAL_MEMORY=67108864 -s FORCE_FILESYSTEM=1 \
		--preload-file resources
	sed 's|{{{ SCRIPT }}}|<script async src="index.js"></script>|' web/shell.html > docs/index.html
	touch docs/.nojekyll

clean:
	rm -f farmville farmville.exe

.PHONY: all run web clean
