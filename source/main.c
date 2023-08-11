#define GS_IMPL
#include <gs/gs.h>

#define GS_IMMEDIATE_DRAW_IMPL
#include <gs/util/gs_idraw.h>

#define GS_ASSET_IMPL
#include <gs/util/gs_asset.h>

/*===========================
// Constantes e Defines
===========================*/
#define GAME_FIELDX     10.f    // Padding entre janela e linha
#define GAME_FIELDY     10.f
#define TMPSTRSZ        1024

#define RABBIT_WIDTH    20.f
#define RABBIT_HEIGHT   20.f
#define RABBIT_SPEED    10.f

#define BALL_SPEED      5.f
#define BALL_WIDTH      10.f
#define BALL_HEIGHT     10.f

#define window_size(...)    gs_platform_window_sizev(gs_platform_main_window())
#define rabbit_dims(...)    gs_v2(RABBIT_WIDTH, RABBIT_HEIGHT)
#define ball_dims(...)      gs_v2(BALL_WIDTH, BALL_HEIGHT)

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

typedef struct ball_t {
    gs_vec2 position;
    gs_vec2 velocity;
} ball_t;


/*============
// Game Data
=============*/

typedef struct game_data_t
{
    gs_command_buffer_t gcb;
    gs_immediate_draw_t gsi;
    gs_asset_manager_t  gsa;
    rabbit_t            rabbit;
    ball_t              ball;
} game_data_t;

// Forward declares
void draw_game(game_data_t* gd);
void init_rabbit(game_data_t* gd);
void update_rabbit(game_data_t* gd);
void init_ball(game_data_t* gd);
void update_ball(game_data_t* gd);

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
    
    // Initialize ball
    init_ball(gd);

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
    update_ball(gd);

    // Desenhar jogos
    draw_game(gd);
}

void app_shutdown()
{
}

gs_aabb_t paddle_aabb(rabbit_t rabbit)  // colisão
{
    gs_aabb_t aabb = {0};
    aabb.min = rabbit.position;
    aabb.max = gs_vec2_add(aabb.min, rabbit_dims());
    return aabb;
}

gs_aabb_t ball_aabb(ball_t ball)
{
    gs_aabb_t aabb = {0};
    aabb.min = ball.position;
    aabb.max = gs_vec2_add(aabb.min, ball_dims());
    return aabb;
}

void init_rabbit(game_data_t* gd)
{
    gs_vec2 pd = rabbit_dims();
    gs_vec2 ws = window_size();
    gd->rabbit.position = gs_v2((ws.x * 0.5f) - (RABBIT_WIDTH * 0.5f), (ws.y - (pd.x * 2.f)));
}

void init_ball(game_data_t* gd)
{
    gs_vec2 ws = window_size();
    gd->ball.position = gs_v2((ws.x - BALL_WIDTH) * 0.5f, (ws.y - BALL_HEIGHT) * 0.5f);
    gd->ball.velocity = gs_v2(-1.f, -1.f);
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
}

void update_ball(game_data_t* gd)
{
    gs_vec2 ws = window_size();

    // Move ball based on its previous velocity
    gd->ball.position.x += gd->ball.velocity.x * BALL_SPEED;
    gd->ball.position.y += gd->ball.velocity.y * BALL_SPEED;

    bool need_pos_reset = false;
    bool need_ball_reset = false;

    // Verifique contra as paredes teto e piso <<<<
    if (
        (gd->ball.position.y > ws.y - GAME_FIELDY - BALL_HEIGHT) ||
        (gd->ball.position.y < GAME_FIELDY)
    )
    {
        gd->ball.velocity.y *= -1.f;
        need_pos_reset = true;
    }

    // Verifica contra parede da direita
    if (gd->ball.position.x > ws.x - GAME_FIELDX - BALL_WIDTH) {
        gd->ball.velocity.x *= -1.f;
        need_pos_reset = true;
    }

    // Verifica contra parede da esquerda
    if (gd->ball.position.x < GAME_FIELDX) {
        // gd->score[PADDLE_RIGHT]++;
        gd->ball.velocity.x *= -1.f;
        need_pos_reset = true;
    }

    // Check for collision against paddle
    gs_aabb_t laabb = paddle_aabb(gd->rabbit);
    gs_aabb_t raabb = paddle_aabb(gd->rabbit);
    gs_aabb_t baabb = ball_aabb(gd->ball);
    if (
        gs_aabb_vs_aabb(&laabb, &baabb) ||
        gs_aabb_vs_aabb(&raabb, &baabb)
    )
    {
        gd->ball.velocity.x *= -1.f;
        need_pos_reset = true;
        init_rabbit(gd);
    }

    // Reset position
    if (need_pos_reset) {
        gd->ball.position.y += gd->ball.velocity.y * BALL_SPEED;
        gd->ball.position.x += gd->ball.velocity.x * BALL_SPEED;
        // play_sound(&gd->gsa, gd->ball_hit_audio, 05.f);
    }

    // Reset ball if scored
    if (need_ball_reset) {
        init_ball(gd);
        // gs_println("scores: %zu, %zu", gd->score[PADDLE_LEFT], gd->score[PADDLE_RIGHT]);
        // play_sound(&gd->gsa, gd->score_audio, 0.5f);
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
    const float y_offset = (ws.y - GAME_FIELDY) / 5;         // espaço entre a linhas
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
    
    // Ball
    {
        gs_vec2 a = gd->ball.position;
        gs_vec2 b = gs_v2(a.x + BALL_WIDTH, a.y + BALL_HEIGHT);
        gsi_rectv(gsi, a, b, GS_COLOR_WHITE, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
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