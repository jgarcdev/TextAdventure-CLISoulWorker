LDFLAGS = -lSDL2 -lGL -lm

INCLUDES = -Iheaders

BIN = creator

SOURCES = main.c editor.c

test:
	gcc $(SOURCES) -o $(BIN) $(LDFLAGS) $(INCLUDES)

WIN_CFLAGS = -I/usr/x86_64-w64-mingw32/include -I/usr/x86_64-w64-mingw32/include/SDL2
WIN_LDFLAGS = -L/usr/x86_64-w64-mingw32/lib -lSDL2main -lSDL2 -lopengl32 -lgdi32 -lwinmm -lsetupapi -lole32 -loleaut32 -limm32 -lversion

test-win:
	zig cc -target x86_64-windows-gnu main.c -o $(BIN).exe $(WIN_CFLAGS) $(WIN_LDFLAGS) $(INCLUDES)