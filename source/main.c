#define GS_IMPL
#include <gs/gs.h>

#define GS_IMMEDIATE_DRAW_IMPL
#include <gs/util/gs_idraw.h>

#define GS_ASSET_IMPL
#include <gs/util/gs_asset.h>

#define GS_META_IMPL
#include <gs/util/gs_meta.h>

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

// Declare custom property type info
#define GS_META_PROPERTY_TYPE_CUSTOM        (GS_META_PROPERTY_TYPE_COUNT + 1)
#define GS_META_PROPERTY_TYPE_INFO_CUSTOM   _gs_meta_property_type_decl(custom_struct_t, GS_META_PROPERTY_TYPE_CUSTOM)

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
// Print meta
============*/

typedef struct custom_struct_t 
{
    gs_vec2 v2val;
    uint64_t u64val;
} custom_struct_t;

// Type to reflect
typedef struct thing_t
{
    float fval1;
    float fval2;
    uint32_t uval1;
    uint32_t uval2;
    gs_vec2 v2val1;
    gs_vec2 v2val2;
    gs_quat qval;
    custom_struct_t csval;
} thing_t;

/*============
// Game Data
=============*/

typedef struct game_data_t
{
    gs_command_buffer_t gcb;
    gs_immediate_draw_t gsi;
    gs_asset_manager_t  gsa;
    gs_meta_registry_t  gmr;
    thing_t             thg;
    rabbit_t            rabbit;
    ball_t              ball;
} game_data_t;

// Forward declares
void draw_game(game_data_t* gd);
void init_rabbit(game_data_t* gd);
void update_rabbit(game_data_t* gd);
void init_ball(game_data_t* gd);
void update_ball(game_data_t* gd);

void print_object(void* obj, const gs_meta_class_t* cls, gs_vec2* pos, game_data_t* gd)
{
    char buf[TMPSTRSZ] = {0};

    #define DO_TEXT(TXT)\
        do {\
            gsi_text(&gd->gsi, pos->x, pos->y, buf, NULL, false, 255, 255, 255, 255);\
            pos->y += 20.f;\
            memset(buf, 0, TMPSTRSZ);\
        } while (0)

    gs_snprintf(buf, TMPSTRSZ, "cls: %s", cls->name);
    DO_TEXT(TMPSTRSZ);

    // Iterate through property count of class
    for (uint32_t i = 0; i < cls->property_count; ++i)
    {
        // Get property at i
        gs_meta_property_t* prop = &cls->properties[i];

        // Depending on property, we'll handle the debug text differently
        switch (prop->type.id)
        {
            case GS_META_PROPERTY_TYPE_U8: 
            {
                gs_snprintf(buf, TMPSTRSZ, "%s (%s): %zu", prop->type.name, prop->name, gs_meta_getv(obj, uint8_t, prop));
            } break;

            case GS_META_PROPERTY_TYPE_S8: 
            {
                gs_snprintf(buf, TMPSTRSZ, "%s (%s): %d", prop->type.name, prop->name, gs_meta_getv(obj, int8_t, prop));
            } break;

            case GS_META_PROPERTY_TYPE_U16: 
            {
                gs_snprintf(buf, TMPSTRSZ, "%s (%s): %zu", prop->type.name, prop->name, gs_meta_getv(obj, uint16_t, prop));
            } break;

            case GS_META_PROPERTY_TYPE_S16: 
            {
                gs_snprintf(buf, TMPSTRSZ, "%s (%s): %d", prop->type.name, prop->name, gs_meta_getv(obj, int16_t, prop));
            } break;

            case GS_META_PROPERTY_TYPE_U32: 
            {
                gs_snprintf(buf, TMPSTRSZ, "%s (%s): %zu", prop->type.name, prop->name, gs_meta_getv(obj, uint32_t, prop));
            } break;

            case GS_META_PROPERTY_TYPE_S32: 
            {
                gs_snprintf(buf, TMPSTRSZ, "%s (%s): %d", prop->type.name, prop->name, gs_meta_getv(obj, int32_t, prop));
            } break;

            case GS_META_PROPERTY_TYPE_U64:
            {
                gs_snprintf(buf, TMPSTRSZ, "%s (%s): %zu", prop->type.name, prop->name, gs_meta_getv(obj, uint64_t, prop));
            } break;

            case GS_META_PROPERTY_TYPE_S64: 
            {
                gs_snprintf(buf, TMPSTRSZ, "%s (%s): %d", prop->type.name, prop->name, gs_meta_getv(obj, int64_t, prop));
            } break;

            case GS_META_PROPERTY_TYPE_F32: 
            {
                gs_snprintf(buf, TMPSTRSZ, "%s (%s): %.2f", prop->type.name, prop->name, gs_meta_getv(obj, float, prop));
            } break;

            case GS_META_PROPERTY_TYPE_F64: 
            {
                gs_snprintf(buf, TMPSTRSZ, "%s (%s): %.2f", prop->type.name, prop->name, gs_meta_getv(obj, double, prop));
            } break;

            case GS_META_PROPERTY_TYPE_VEC2: 
            {
                gs_vec2* v = gs_meta_getvp(obj, gs_vec2, prop);
                gs_snprintf(buf, TMPSTRSZ, "%s (%s): <%.2f, %.2f>", prop->type.name, prop->name, v->x, v->y);
            } break;

            case GS_META_PROPERTY_TYPE_VEC3: 
            {
                gs_vec3* v = gs_meta_getvp(obj, gs_vec3, prop);
                gs_snprintf(buf, TMPSTRSZ, "%s (%s): <%.2f, %.2f, %.2f>", prop->type.name, prop->name, v->x, v->y, v->z);
            } break;

            case GS_META_PROPERTY_TYPE_VEC4: 
            {
                gs_vec4* v = gs_meta_getvp(obj, gs_vec4, prop);
                gs_snprintf(buf, TMPSTRSZ, "%s (%s): <%.2f, %.2f, %.2f, %.2f>", prop->type.name, prop->name, v->x, v->y, v->z, v->w);
            } break;

            case GS_META_PROPERTY_TYPE_QUAT: 
            {
                gs_quat* v = gs_meta_getvp(obj, gs_quat, prop);
                gs_snprintf(buf, TMPSTRSZ, "%s (%s): <%.2f, %.2f, %.2f, %.2f>", prop->type.name, prop->name, v->x, v->y, v->z, v->w);
            } break;

            case GS_META_PROPERTY_TYPE_MAT4:
            {
                gs_mat4* m = gs_meta_getvp(obj, gs_mat4, prop);
                gs_snprintf(buf, TMPSTRSZ, "%s (%s): \n\t<%.2f, %.2f, %.2f, %.2f>\
                    \n\t<%.2f, %.2f, %.2f, %.2f>\
                    \n\t<%.2f, %.2f, %.2f, %.2f>\
                    \n\t<%.2f, %.2f, %.2f, %.2f>", 
                    prop->type.name, prop->name, 
                    m->elements[0], m->elements[1], m->elements[2], m->elements[3],
                    m->elements[4], m->elements[5], m->elements[6], m->elements[7],
                    m->elements[8], m->elements[9], m->elements[10], m->elements[11],
                    m->elements[12], m->elements[13], m->elements[14], m->elements[15]
                );
            } break;

            case GS_META_PROPERTY_TYPE_VQS: 
            {
                gs_vqs* v = gs_meta_getvp(obj, gs_vqs, prop);
                gs_snprintf(buf, TMPSTRSZ, "%s (%s): pos: <%.2f, %.2f, %.2f>, rot: <%.2f, %.2f, %.2f, %.2f>, scale: <%.2f, %.2f, %.2f>", 
                    prop->type.name, prop->name, 
                    v->position.x, v->position.y, v->position.z,
                    v->rotation.x, v->rotation.y, v->rotation.z, v->rotation.w,
                    v->scale.x, v->scale.y, v->scale.z
                );
            } break;

            case GS_META_PROPERTY_TYPE_CUSTOM:
            {
                // Get other class for this type, then pass in to this function recursively to display
                gs_meta_class_t* clz = gs_meta_class_get(&gd->gmr, custom_struct_t);
                if (clz)
                {
                    // Get value at this property to pass into print function
                    custom_struct_t* cs = gs_meta_getvp(obj, custom_struct_t, prop);
                    // Do temp print for this property name and type
                    gs_snprintf(buf, TMPSTRSZ, "%s (%s):", prop->type.name, prop->name);
                    DO_TEXT(TMPSTRSZ);
                    // Tab x value
                    pos->x += 10.f;
                    // Print object
                    print_object(cs, clz, pos, gd);
                    // Return x value to original
                    pos->x -= 10.f;
                }

            } break;
        }

        DO_TEXT(TMPSTRSZ);
    }
}

void app_init()
{
    // Pegue o ponteiro de dados do usuário da estrutura
    game_data_t* gd = gs_user_data(game_data_t);

    // Inicializar utilitários
    gd->gcb = gs_command_buffer_new();
    gd->gsi = gs_immediate_draw_new();
    gd->gsa = gs_asset_manager_new();
    gd->gmr = gs_meta_registry_new();

    // Inicializa rabbit
    init_rabbit(gd);
    
    // Initialize ball
    init_ball(gd);
    
    // Register meta class information for thing (returns id, if needed)
    uint64_t thing_cls_id = gs_meta_class_register(&gd->gmr, (&(gs_meta_class_decl_t){
        .name = gs_to_str(thing_t),
        .properties = (gs_meta_property_t[]) {
            gs_meta_property(thing_t, float, fval1, GS_META_PROPERTY_TYPE_INFO_F32),        // Default provided types
            gs_meta_property(thing_t, float, fval2, GS_META_PROPERTY_TYPE_INFO_F32),        // Default provided types
            gs_meta_property(thing_t, uint32_t, uval1, GS_META_PROPERTY_TYPE_INFO_U32),
            gs_meta_property(thing_t, uint32_t, uval2, GS_META_PROPERTY_TYPE_INFO_U32),
            gs_meta_property(thing_t, gs_vec2, v2val1, GS_META_PROPERTY_TYPE_INFO_VEC2),
            gs_meta_property(thing_t, gs_vec2, v2val2, GS_META_PROPERTY_TYPE_INFO_VEC2),
            gs_meta_property(thing_t, gs_quat, qval, GS_META_PROPERTY_TYPE_INFO_QUAT),
            gs_meta_property(thing_t, custom_struct_t, csval, GS_META_PROPERTY_TYPE_INFO_CUSTOM)     // Custom property type info declared above
        },
        .size = 8 * sizeof(gs_meta_property_t)
    }));

    // Register meta class information for custom struct (returns id, if needed)
    uint64_t cs_cls_id = gs_meta_class_register(&gd->gmr, (&(gs_meta_class_decl_t){
        .name = gs_to_str(custom_struct_t),
        .properties = (gs_meta_property_t[]) {
            gs_meta_property(custom_struct_t, gs_vec2, v2val, GS_META_PROPERTY_TYPE_INFO_VEC2),
            gs_meta_property(custom_struct_t, uint64_t, u64val, GS_META_PROPERTY_TYPE_INFO_U64)
        },
        .size = 2 * sizeof(gs_meta_property_t)
    }));

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
        gs_println("scores: %f, %f", gd->ball.position.x, gd->ball.position.y);
        gd->ball.velocity.y *= -1.f;
        need_pos_reset = true;
    }

    // Verifica contra parede da direita
    if (gd->ball.position.x > ws.x - GAME_FIELDX - BALL_WIDTH) {
        gs_println("scores: %f, %f", gd->ball.position.x, gd->ball.position.y);
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

    // Construct instance of thing struct
    gd->thg = (thing_t) {
        .fval1 = GAME_FIELDX,
        .fval2 = GAME_FIELDY,
        .uval1 = 12,
        .uval2 = 34,
        .v2val1 = gd->ball.position,
        .v2val2 = gd->rabbit.position,
        .qval = gs_quat_default(),
        .csval = (custom_struct_t){
            .v2val = gs_v2(2, 4),
            .u64val = 123
        }
    };

    // Present property information (debug text)
    gs_meta_class_t* cls = gs_meta_class_get(&gd->gmr, thing_t);

    // Do print
    gs_vec2 pos = gs_v2(100.f, 100.f);
    print_object(&gd->thg, cls, &pos, gd);

    // Envio de sorteio imediato final e passagem de renderização
    gsi_renderpass_submit(gsi, gcb, gs_v4(0.f, 0.f, ws.x, ws.y), gs_color(20, 20, 20, 255));
    // Envio de buffer de comando de back-end gráfico final
    gs_graphics_command_buffer_submit(gcb);
}

// Globals
game_data_t         gdata = {0};
thing_t             thing = {0};
gs_immediate_draw_t gsi   = {0};
gs_meta_registry_t  gmr   = {0};

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