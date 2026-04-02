#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL_GL2_IMPLEMENTATION
#include "nuklear.h"
#include "nuklear_sdl_gl2.h"


int main(int argc, char *argv[]) {
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window* window = SDL_CreateWindow("Create Map",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		800, 600,
		SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
	);

	SDL_GLContext glCtx = SDL_GL_CreateContext(window);
	SDL_GL_SetSwapInterval(1);

	struct nk_context* ctx = nk_sdl_init(window);
	struct nk_font_atlas* atlas;
	nk_sdl_font_stash_begin(&atlas);
	nk_sdl_font_stash_end();

	struct nk_colorf bg = {0.10f, 0.18f, 0.24f, 1.0f};

	int running = 1;
	while (running) {
		SDL_Event evt;
		nk_input_begin(ctx);

		while (SDL_PollEvent(&evt)) {
			if (evt.type == SDL_QUIT) running = 0;
			nk_sdl_handle_event(&evt);
		}

		nk_input_end(ctx);

		if (nk_begin(ctx, "Demo", nk_rect(50, 50, 230, 250), 
				NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE| NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE)) {
			
			nk_layout_row_dynamic(ctx, 30, 1);
			nk_label(ctx, "Hello, world!", NK_TEXT_LEFT);

			if (nk_button_label(ctx, "Quit")) running = 0;
		}
		nk_end(ctx);

		glViewport(0, 0, 800, 600);
		glClearColor(bg.r, bg.g, bg.b, bg.a);
		glClear(GL_COLOR_BUFFER_BIT);
		nk_sdl_render(NK_ANTI_ALIASING_ON);
		SDL_GL_SwapWindow(window);
	}

	nk_sdl_shutdown();
	SDL_GL_DeleteContext(glCtx);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}