// main.c now forwards to the editor implementation in editor.c
#include "editor.h"

int main(int argc, char *argv[]) {
	return editor_run(argc, argv);
}