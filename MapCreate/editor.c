#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL_GL2_IMPLEMENTATION
#include "nuklear.h"
#include "nuklear_sdl_gl2.h"

#include "editor.h"

#include <math.h>

#define MAX_ROOMS 256
#define MAX_EDGES (MAX_ROOMS*2)

typedef struct {
    int id;
    int isEntry;
    int exits[4]; /* computed at save time */
    char info[256];
    int hasBoss;
    float x,y; /* position in canvas coords */
    int loot_count;
    int enemy_count;
} Room;

typedef struct { int a,b; } Edge;

static Room rooms[MAX_ROOMS];
static int room_count = 0;
static Edge edges[MAX_EDGES];
static int edge_count = 0;

static Room* find_room_by_id(int id) {
    if (id<0 || id>=room_count) return NULL;
    return &rooms[id];
}

static int find_edge(int a, int b){
    for (int i=0;i<edge_count;i++){
        if ((edges[i].a==a && edges[i].b==b) || (edges[i].a==b && edges[i].b==a)) return i;
    }
    return -1;
}

static int degree_of(int id){
    int d=0; for (int i=0;i<edge_count;i++) if (edges[i].a==id || edges[i].b==id) d++; return d;
}

static Room* create_room_at(float x, float y) {
    if (room_count>=MAX_ROOMS) return NULL;
    Room *n = &rooms[room_count++];
    memset(n,0,sizeof(Room));
    n->id = room_count-1;
    n->isEntry = 0;
    for (int i=0;i<4;i++) n->exits[i] = -1;
    n->x = x; n->y = y;
    snprintf(n->info,sizeof(n->info),"Room %d", n->id);
    return n;
}

static int create_edge(int a, int b){
    if (a==b) return 0;
    if (find_edge(a,b)>=0) return 0;
    if (degree_of(a)>=4 || degree_of(b)>=4) return 0;
    if (edge_count>=MAX_EDGES) return 0;
    edges[edge_count].a = a; edges[edge_count].b = b; edge_count++; return 1;
}

static void recompute_exits_from_edges(){
    for (int i=0;i<room_count;i++) for (int j=0;j<4;j++) rooms[i].exits[j] = -1;
    for (int i=0;i<edge_count;i++){
        Room *ra = find_room_by_id(edges[i].a);
        Room *rb = find_room_by_id(edges[i].b);
        if (!ra||!rb) continue;
        float dx = rb->x - ra->x; float dy = rb->y - ra->y;
        int dir = -1;
        if (fabsf(dx) > fabsf(dy)){
            dir = dx>0 ? 1 : 3; // east or west
        } else {
            dir = dy>0 ? 2 : 0; // south or north
        }
        if (dir>=0){
            if (ra->exits[dir]==-1) ra->exits[dir] = rb->id;
            int opposite = (dir+2)%4;
            if (rb->exits[opposite]==-1) rb->exits[opposite] = ra->id;
        }
    }
}

static int save_map_as(const char *path, const char *mapname) {
    recompute_exits_from_edges();
    FILE *f = fopen(path,"w");
    if (!f) return 0;
    fprintf(f,"{\n  \"$schema\": \"./schema-map.json\",\n\n");
    for (int i=0;i<room_count;i++){
        Room *r = &rooms[i];
        fprintf(f,"  \"%d\": {\n", r->id);
        fprintf(f,"    \"isEntry\": %d,\n", r->isEntry);
        // escape info simple
        char info_esc[512];
        int p=0;
        for (int j=0; r->info[j] && p< (int)sizeof(info_esc)-2; j++){
            char c = r->info[j];
            if (c=='"' || c=='\\') { info_esc[p++]='\\'; info_esc[p++]=c; }
            else if (c=='\n') { info_esc[p++]='\\'; info_esc[p++]='n'; }
            else info_esc[p++]=c;
        }
        info_esc[p]=0;
        fprintf(f,"    \"info\": \"%s\",\n", info_esc);
        fprintf(f,"    \"exits\": [%d, %d, %d, %d],\n", r->exits[0], r->exits[1], r->exits[2], r->exits[3]);
        fprintf(f,"    \"hasBoss\": %d,\n", r->hasBoss);
        fprintf(f,"    \"enemy\": [],\n");
        fprintf(f,"    \"loot\": []\n");
        fprintf(f,"  }%s\n", (i==room_count-1)?"":" ,");
    }
    fprintf(f, "}\n");
    fclose(f);
    return 1;
}

int editor_run(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Map Editor",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1024, 720,
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
    int selected_id = -1;
    char mapname[64] = "new_map";
    int mouse_buttons_prev = 0;
    int moving_node = -1; /* left-drag to move */
    int edge_src = -1; /* right-drag to create edge */
    int panning = 0;
    int pan_start_x = 0, pan_start_y = 0;
    float pan_x = 0.0f, pan_y = 0.0f;
    int last_mx = 0, last_my = 0;
    float drag_off_x = 0.0f, drag_off_y = 0.0f;
    int show_save_popup = 0;
    char save_path[256] = "";

    while (running) {
        SDL_Event evt;
        nk_input_begin(ctx);

        while (SDL_PollEvent(&evt)) {
            if (evt.type == SDL_QUIT) running = 0;
            nk_sdl_handle_event(&evt);
        }

        nk_input_end(ctx);

        int pressed_on_node = -1;
        int pressed_on_node_right = -1;

        int win_w=1024, win_h=720;
        SDL_GetWindowSize(window, &win_w, &win_h);

        /* compute layout sizes */
        int left_w = 220;
        int right_w = 220;
        int menu_h = 36;
        int pad = 8;
        int canvas_x = left_w + pad;
        int canvas_y = menu_h + pad;
        int canvas_w = win_w - left_w - right_w - pad*3;
        if (canvas_w < 200) canvas_w = 200;
        int canvas_h = win_h - canvas_y - 80;

        if (nk_begin(ctx, "Map Editor", nk_rect(0,0,win_w,win_h), NK_WINDOW_BORDER|NK_WINDOW_TITLE)){
            /* Menu bar */
            nk_menubar_begin(ctx);
            nk_layout_row_begin(ctx, NK_STATIC, 25, 2);
            nk_layout_row_push(ctx, 60);
            if (nk_menu_begin_label(ctx, "File", NK_TEXT_LEFT, nk_vec2(200,200))){
                nk_layout_row_dynamic(ctx, 20, 1);
                if (nk_menu_item_label(ctx, "Save", NK_TEXT_LEFT)){
                    /* open save-as popup */
                    snprintf(save_path, sizeof(save_path), "../data/maps/%s.json", mapname);
                    show_save_popup = 1;
                }
                nk_menu_end(ctx);
            }
            nk_menubar_end(ctx);
            nk_layout_row_begin(ctx, NK_STATIC, canvas_h, 3);
            nk_layout_row_push(ctx, left_w); // left properties
            if (nk_group_begin(ctx, "properties", NK_WINDOW_BORDER)){
                nk_layout_row_dynamic(ctx, 20, 1);
                nk_label(ctx, "Properties", NK_TEXT_LEFT);
                nk_layout_row_dynamic(ctx, 24, 1);
                if (nk_button_label(ctx, "Add Node")){
                    Room *nr = create_room_at(canvas_w/2.0f - pan_x, canvas_h/2.0f - pan_y);
                    if (nr) selected_id = nr->id;
                }
                if (selected_id>=0){
                    Room *r = &rooms[selected_id];
                    nk_label(ctx, "Selected Room:", NK_TEXT_LEFT);
                    nk_edit_string_zero_terminated(ctx, NK_EDIT_SIMPLE, r->info, sizeof(r->info), nk_filter_default);
                    nk_checkbox_label(ctx, "Entry", &r->isEntry);
                    nk_checkbox_label(ctx, "Boss Room", &r->hasBoss);
                    nk_label(ctx, "Exits (N E S W)", NK_TEXT_LEFT);
                    char buf[128];
                    snprintf(buf,sizeof(buf),"[%d, %d, %d, %d]", r->exits[0], r->exits[1], r->exits[2], r->exits[3]);
                    nk_label(ctx, buf, NK_TEXT_LEFT);
                } else {
                    nk_label(ctx, "No room selected", NK_TEXT_LEFT);
                }
                nk_group_end(ctx);
            }

            nk_layout_row_push(ctx, canvas_w); // center canvas
            if (nk_group_begin(ctx, "canvas", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)){
                nk_layout_row_dynamic(ctx, 24, 1);
                nk_label(ctx, "Canvas (left-drag to move node, right-drag from node to node to create edge)", NK_TEXT_LEFT);

                /* Begin absolute space for custom positioned nodes */
                nk_layout_space_begin(ctx, NK_STATIC, canvas_h, room_count + edge_count + 8);

                /* Draw edges as small rectangles along the line */
                for (int ei=0; ei<edge_count; ei++){
                    Room *a = find_room_by_id(edges[ei].a);
                    Room *b = find_room_by_id(edges[ei].b);
                    if (!a || !b) continue;
                    float ax = canvas_x + a->x + pan_x; float ay = canvas_y + a->y + pan_y;
                    float bx = canvas_x + b->x + pan_x; float by = canvas_y + b->y + pan_y;
                    int steps = 12;
                    for (int s=0;s<=steps;s++){
                        float t = (float)s/steps;
                        float px = ax + (bx-ax)*t;
                        float py = ay + (by-ay)*t;
                        nk_layout_space_push(ctx, nk_rect(px-3, py-3, 6, 6));
                        nk_button_label(ctx, "");
                    }
                }

                /* Draw nodes */
                for (int i=0;i<room_count;i++){
                    Room *r = &rooms[i];
                    float nx = canvas_x + r->x + pan_x; float ny = canvas_y + r->y + pan_y;
                    nk_layout_space_push(ctx, nk_rect(nx-12, ny-12, 24, 24));
                    if (nk_button_label(ctx, r->info)){
                        selected_id = r->id;
                    }
                    /* detect if the mouse is pressed on this widget (so we know clicks started on a node) */
                    if (nk_widget_has_mouse_click_down(ctx, NK_BUTTON_LEFT, 1)){
                        pressed_on_node = r->id;
                    }
                    if (nk_widget_has_mouse_click_down(ctx, NK_BUTTON_RIGHT, 1)){
                        pressed_on_node_right = r->id;
                    }
                }

                /* removed canvas-space Add Node (moved to right panel) */

                nk_layout_space_end(ctx);
                nk_group_end(ctx);
            }

            nk_layout_row_push(ctx, right_w); // right items/enemies
            if (nk_group_begin(ctx, "right", NK_WINDOW_BORDER)){
                nk_layout_row_dynamic(ctx, 20, 1);
                nk_label(ctx, "Items / Enemies", NK_TEXT_LEFT);
                nk_layout_row_dynamic(ctx, 24, 1);
                if (nk_button_label(ctx, "Add Simple Enemy")){
                    // attach a simple enemy to selected room
                    if (selected_id>=0){
                        Room *r = &rooms[selected_id];
                        r->enemy_count = 1; // placeholder
                    }
                }
                if (nk_button_label(ctx, "Add Simple Loot")){
                    if (selected_id>=0){
                        Room *r = &rooms[selected_id];
                        r->loot_count = 1;
                    }
                }
                nk_group_end(ctx);
            }

            nk_layout_row_end(ctx);

            /* Debug overlay */
            nk_layout_row_dynamic(ctx, 16, 1);
            {
                char dbg[256];
                snprintf(dbg, sizeof(dbg), "mx=%d,my=%d pan=(%.0f,%.0f) moving=%d pressed=%d edge_src=%d panning=%d", last_mx, last_my, pan_x, pan_y, moving_node, pressed_on_node, edge_src, panning);
                nk_label(ctx, dbg, NK_TEXT_LEFT);
            }

            /* Map name is no longer shown here; saving is via File->Save only */
            /* Save As popup (must be inside the current window) */
            if (show_save_popup){
                struct nk_rect r = nk_rect((win_w/2)-180, (win_h/2)-60, 360, 120);
                if (nk_popup_begin(ctx, NK_POPUP_STATIC, "saveas", NK_WINDOW_TITLE, r)){
                    nk_layout_row_dynamic(ctx, 20, 1);
                    nk_label(ctx, "Save map as:", NK_TEXT_LEFT);
                    nk_layout_row_dynamic(ctx, 24, 1);
                    nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, save_path, sizeof(save_path), nk_filter_default);
                    nk_layout_row_dynamic(ctx, 28, 2);
                    if (nk_button_label(ctx, "OK")){
                        recompute_exits_from_edges();
                        save_map_as(save_path, mapname);
                        show_save_popup = 0;
                    }
                    if (nk_button_label(ctx, "Cancel")){
                        show_save_popup = 0;
                    }
                    nk_popup_end(ctx);
                }
            }
        }
        /* finished window */
        nk_end(ctx);

        /* Handle mouse-driven move/edge creation (use SDL mouse state) */
        int mx, my; unsigned int mmask = SDL_GetMouseState(&mx,&my);
        int left_down = mmask & SDL_BUTTON(SDL_BUTTON_LEFT);
        int right_down = mmask & SDL_BUTTON(SDL_BUTTON_RIGHT);
        last_mx = mx; last_my = my;

        /* detect left button down transition */
            if (left_down && !(mouse_buttons_prev & SDL_BUTTON(SDL_BUTTON_LEFT))){
            /* start move if clicked on a node (prefer Nuklear-detected press) */
            if (pressed_on_node >= 0) {
                moving_node = pressed_on_node;
                /* compute offset from node origin so it doesn't jump on drag start */
                {
                    Room *r = &rooms[moving_node];
                    float nx = canvas_x + r->x + pan_x;
                    float ny = canvas_y + r->y + pan_y;
                    drag_off_x = (float)mx - nx;
                    drag_off_y = (float)my - ny;
                }
            } else {
                /* only consider panning if click started inside the canvas area */
                if (mx >= canvas_x && mx <= canvas_x + canvas_w && my >= canvas_y && my <= canvas_y + canvas_h) {
                    /* check geometry to see if click hit a node (fallback) */
                    int clicked_node = -1;
                    for (int i=0;i<room_count;i++){
                        float nx = canvas_x + rooms[i].x + pan_x; float ny = canvas_y + rooms[i].y + pan_y;
                        if (mx >= nx-12 && mx <= nx+12 && my >= ny-12 && my <= ny+12){
                            clicked_node = i; break;
                        }
                    }
                    if (clicked_node>=0) {
                        moving_node = clicked_node;
                        Room *r = &rooms[moving_node];
                        float nx = canvas_x + r->x + pan_x;
                        float ny = canvas_y + r->y + pan_y;
                        drag_off_x = (float)mx - nx;
                        drag_off_y = (float)my - ny;
                    }
                    else {
                        panning = 1;
                        pan_start_x = mx; pan_start_y = my;
                    }
                }
            }
        }
        /* left release */
        if (!left_down && (mouse_buttons_prev & SDL_BUTTON(SDL_BUTTON_LEFT))){
            moving_node = -1;
            panning = 0;
        }

        /* handle node moving */
        if (moving_node>=0 && left_down){
            rooms[moving_node].x = (float)mx - canvas_x - pan_x - drag_off_x;
            rooms[moving_node].y = (float)my - canvas_y - pan_y - drag_off_y;
        } else if (panning && left_down){
            /* pan by mouse movement delta */
            pan_x += (mx - pan_start_x);
            pan_y += (my - pan_start_y);
            pan_start_x = mx; pan_start_y = my;
        }

        /* right button: start edge drag */
        if (right_down && !(mouse_buttons_prev & SDL_BUTTON(SDL_BUTTON_RIGHT))){
            /* prefer Nuklear-detected right-click on node */
            if (pressed_on_node_right >= 0) {
                edge_src = pressed_on_node_right;
            } else {
                /* only start edge drag if inside canvas */
                if (mx >= canvas_x && mx <= canvas_x + canvas_w && my >= canvas_y && my <= canvas_y + canvas_h) {
                    for (int i=0;i<room_count;i++){
                        float nx = canvas_x + rooms[i].x + pan_x; float ny = canvas_y + rooms[i].y + pan_y;
                        if (mx >= nx-12 && mx <= nx+12 && my >= ny-12 && my <= ny+12){
                            edge_src = i; break;
                        }
                    }
                }
            }
        }
        if (!right_down && (mouse_buttons_prev & SDL_BUTTON(SDL_BUTTON_RIGHT))){
            if (edge_src>=0){
                /* check release over a target node */
                for (int i=0;i<room_count;i++){
                    if (i==edge_src) continue;
                    float nx = canvas_x + rooms[i].x + pan_x; float ny = canvas_y + rooms[i].y + pan_y;
                    if (mx >= nx-12 && mx <= nx+12 && my >= ny-12 && my <= ny+12){
                        create_edge(edge_src, i);
                        break;
                    }
                }
            }
            edge_src = -1;
        }

        mouse_buttons_prev = mmask;

        glViewport(0, 0, win_w, win_h);
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
