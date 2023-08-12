#define GS_IMPL
#include <gs/gs.h>

#define GS_IMMEDIATE_DRAW_IMPL
#include <gs/util/gs_idraw.h>

#define GS_ASSET_IMPL
#include <gs/util/gs_asset.h>

#include <time.h>

/*===========================
// Constantes e Defines
===========================*/
#define GAME_FIELDX     10.f    // Padding entre janela e linha
#define GAME_FIELDY     10.f
#define TMPSTRSZ        1024

#define RABBIT_WIDTH    20.f
#define RABBIT_HEIGHT   20.f
#define RABBIT_SPEED    5.f

#define WOLF_SPEED      5.f
#define WOLF_WIDTH      10.f
#define WOLF_HEIGHT     10.f

#define SPACES_GAME     5

#define window_size(...)    gs_platform_window_sizev(gs_platform_main_window())
#define rabbit_dims(...)    gs_v2(RABBIT_WIDTH, RABBIT_HEIGHT)
#define wolf_dims(...)      gs_v2(WOLF_WIDTH, WOLF_HEIGHT)

/*==========
// Rabbit
===========*/

typedef struct rabbit_t {
    gs_vec2 position;
    gs_vec2 velocity;
} rabbit_t;

/*======
// Bola
======*/

typedef enum wolf_level {
    WOLF1,
    WOLF2,
    WOLF3,
    WOLF4,
    WOLF5,
    WOLF6,
    WOLF7,
    WOLF8,
    WOLF9,
    WOLF10,
    WOLF_COUNT,
} wolf_level;

typedef struct wolf_t {
    gs_vec2 position;
    gs_vec2 velocity;
} wolf_t;

/*============
// Game Data
=============*/

typedef struct game_data_t
{
    gs_command_buffer_t gcb;
    gs_immediate_draw_t gsi;
    gs_asset_manager_t  gsa;
    rabbit_t            rabbit;
    wolf_t              wolf[WOLF_COUNT];
} game_data_t;

// Forward declares
void draw_game(game_data_t* gd);
void init_rabbit(game_data_t* gd);
void update_rabbit(game_data_t* gd);
void init_wolf(game_data_t* gd);
void update_wolf(game_data_t* gd);

int32_t random_val_int(int32_t lower, int32_t upper)
{ 
    int32_t n = (rand() % (upper - lower + 1)) + lower;
    if (n == 0) return(random_val_int(lower, upper));
    return (n);
} 

void app_init()
{
    // Pegue o ponteiro de dados do usuário da estrutura
    game_data_t* gd = gs_user_data(game_data_t);

    // Inicializar utilitários
    gd->gcb = gs_command_buffer_new();
    gd->gsi = gs_immediate_draw_new(gs_platform_main_window());
    gd->gsa = gs_asset_manager_new();

    // Inicializa rabbit
    init_rabbit(gd);
    
    // Initialize wolf
    init_wolf(gd);

}

void app_update()
{
    // Obter dados do jogo
    game_data_t* gd = gs_user_data(game_data_t);
    
    // Verifique a entrada para fechar o aplicativo
    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) {
        gs_quit();
    }

    // Updates
    update_rabbit(gd);
    for (uint32_t i = 0; i < WOLF_COUNT; ++i) update_wolf(gd, i);

    // Desenhar jogos
    draw_game(gd);
}

void app_shutdown()
{
}

gs_aabb_t rabbit_aabb(rabbit_t rabbit)  // colisão
{
    gs_aabb_t aabb = {0};
    aabb.min = rabbit.position;
    aabb.max = gs_vec2_add(aabb.min, rabbit_dims());
    return aabb;
}

gs_aabb_t wolf_aabb(wolf_t wolf)
{
    gs_aabb_t aabb = {0};
    aabb.min = wolf.position;
    aabb.max = gs_vec2_add(aabb.min, wolf_dims());
    return aabb;
}

void init_rabbit(game_data_t* gd)
{
    gs_vec2 pd = rabbit_dims();
    gs_vec2 ws = window_size();
    gd->rabbit.position = gs_v2((ws.x * 0.5f) - (RABBIT_WIDTH * 0.5f), (ws.y - (pd.x * 2.f)));
}

void init_wolf(game_data_t* gd)
{
    gs_vec2 wf = wolf_dims();
    gs_vec2 ws = window_size();
    
    const float y_offset = (ws.y - GAME_FIELDY) / SPACES_GAME;    // espaço entre a linhas
    const float pos_y = y_offset + ((GAME_FIELDY + WOLF_HEIGHT + y_offset) * 0.5f);
    uint32_t adiciona_wolf[] = {5,3,2};
    uint32_t indice = 0;
    for (uint32_t i = 0; i < 3; ++i)
    {   
        const float x_offset = ((ws.x - GAME_FIELDX - wf.x) / (adiciona_wolf[i] + 1));  // Divide o espaço para os lobos
        
        for (uint32_t j = 0; j < adiciona_wolf[i]; ++j)
        {
            int dir = indice%2 == 0 ? -1 : 1;
            gd->wolf[indice].position = gs_v2(x_offset * (j+1), pos_y + (i * y_offset) + (random_val_int(10,50)*(dir)));
            gd->wolf[indice].velocity = gs_v2((f32)random_val_int(-1,1), (f32)random_val_int(-1,1));
            indice++;
        }
    }
}

void update_rabbit(game_data_t* gd)
{
    gs_vec2 ws = window_size();
    float* y = NULL;
    float* x = NULL;
    float miny = GAME_FIELDY;
    float minx = GAME_FIELDX;
    float maxy = ws.y - RABBIT_HEIGHT - miny;
    float maxx = ws.x - RABBIT_WIDTH - minx;
    const float y_offset = (ws.y - GAME_FIELDY) / SPACES_GAME;    // espaço entre a linhas

    // Left rabbit movement y 
    y = &gd->rabbit.position.y;
    if (gs_platform_key_down(GS_KEYCODE_W)) {
        *y = gs_clamp(*y - RABBIT_SPEED, miny, maxy);
    }
    if (gs_platform_key_down(GS_KEYCODE_S)) {
        *y = gs_clamp(*y + RABBIT_SPEED, miny, maxy);
    }

    // Left rabbit movement x 
    x = &gd->rabbit.position.x;
    if (gs_platform_key_down(GS_KEYCODE_A)) {
        *x = gs_clamp(*x - RABBIT_SPEED, minx, maxx);
    }
    if (gs_platform_key_down(GS_KEYCODE_D)) {
        *x = gs_clamp(*x + RABBIT_SPEED, minx, maxx);
    }

    // Rabbit ganha
    if (*y < (y_offset))
    {
        gs_quit();
    }
}

void update_wolf(game_data_t* gd, uint32_t i)
{
    gs_vec2 ws = window_size();
    const float y_offset = (ws.y - GAME_FIELDY) / SPACES_GAME;    // espaço entre a linhas

    // Move wolf based on its previous velocity
    {
        gd->wolf[i].position.x += gd->wolf[i].velocity.x * WOLF_SPEED;
        gd->wolf[i].position.y += gd->wolf[i].velocity.y * WOLF_SPEED;
    }

    bool need_pos_reset = false;
    bool need_ball_reset = false;

    // Verifique contra as paredes teto e piso correspondente<<<<
    if (
        (gd->wolf[i].position.y > ws.y - GAME_FIELDY - WOLF_HEIGHT - ((3-i) * y_offset)) ||
        (gd->wolf[i].position.y < GAME_FIELDY + ((1+i) * y_offset))
    )
    {
        gd->wolf[i].velocity.y *= -1.f;
        need_pos_reset = true;
    }

    // Verifica contra parede da direita
    if (gd->wolf[i].position.x > ws.x - GAME_FIELDX - WOLF_WIDTH) {
        gd->wolf[i].velocity.x *= -1.f;
        need_pos_reset = true;
    }

    // Verifica contra parede da esquerda
    if (gd->wolf[i].position.x < GAME_FIELDX) {
        gd->wolf[i].velocity.x *= -1.f;
        need_pos_reset = true;
    }

    // Check for collision against lobo
    gs_aabb_t raabb = rabbit_aabb(gd->rabbit);
    gs_aabb_t waabb = wolf_aabb(gd->wolf[i]);
    if (
        gs_aabb_vs_aabb(&raabb, &waabb)
    )
    {
        gd->wolf[i].velocity.x *= -1.f;
        need_pos_reset = true;
        init_rabbit(gd);
    }

    // Reset position
    if (need_pos_reset) {
        gd->wolf[i].position.y += gd->wolf[i].velocity.y * WOLF_SPEED;
        gd->wolf[i].position.x += gd->wolf[i].velocity.x * WOLF_SPEED;
    }

}

void draw_game(game_data_t* gd)
{
    // Cache pointers
    gs_command_buffer_t* gcb = &gd->gcb;
    gs_immediate_draw_t* gsi = &gd->gsi;
    gs_asset_manager_t*  gsa = &gd->gsa;

    // Window Size
    gs_vec2 ws = window_size();

    // 2D Camera (for screen coordinates)
    gsi_camera2D(gsi, ws.x, ws.y);

    // Game Field
    gsi_rect(gsi, GAME_FIELDX, GAME_FIELDY, ws.x - GAME_FIELDX, ws.y - GAME_FIELDY, 255, 255, 255, 255, GS_GRAPHICS_PRIMITIVE_LINES);

    // Game Field linhas divisoras
    const float y_offset = (ws.y - GAME_FIELDY) / SPACES_GAME;         // espaço entre a linhas
    gs_vec2 div_dim = gs_v2(1.f, 1.f); // Tamanho das linhas Widht x Height
    int32_t num_steps = (ws.y - GAME_FIELDY * 2.f) / (div_dim.y + y_offset);    // Quantidade de linhas
    for (uint32_t i = 1; i <= num_steps; ++i)
    {
        gs_vec2 a = gs_v2(GAME_FIELDX, GAME_FIELDY + i * (div_dim.y + y_offset));
        gs_vec2 b = gs_v2(ws.x - GAME_FIELDX, GAME_FIELDY + i * (div_dim.y + y_offset));
        gsi_linev(gsi, a, b, gs_color(255, 255, 255, 255));
    }

    // Rabbit
    {
        gs_vec2 a = gd->rabbit.position;
        gs_vec2 b = gs_v2(a.x + RABBIT_WIDTH, a.y + RABBIT_HEIGHT);
        gsi_rectv(gsi, a, b, GS_COLOR_WHITE, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
    }
    
    // wolf
    for (uint32_t i = 0; i < WOLF_COUNT; ++i)
    {
        gs_vec2 a = gd->wolf[i].position;
        gs_vec2 b = gs_v2(a.x + WOLF_WIDTH, a.y + WOLF_HEIGHT);
        gsi_rectv(gsi, a, b, gs_color_ctor(255, 0, 0, 255), GS_GRAPHICS_PRIMITIVE_TRIANGLES);
    }

    // Envio de sorteio imediato final e passagem de renderização
    gsi_renderpass_submit(gsi, gcb, gs_v4(0.f, 0.f, ws.x, ws.y), gs_color(20, 20, 20, 255));
    // Envio de buffer de comando de back-end gráfico final
    gs_graphics_command_buffer_submit(gcb);
}

// Globals
game_data_t         gdata = {0};
gs_immediate_draw_t gsi   = {0};

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
    srand(time(NULL));
    return (gs_app_desc_t){
        .user_data = gs_malloc_init(game_data_t),
        .init = app_init,
		.update = app_update,
        .window = {
            .width = 600,
            .height = 900,
            .title = "Rabbit to the Hole",
        }
    };
}