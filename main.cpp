#include "allegro5/allegro.h"
#include "allegro5/allegro_primitives.h"
#include "allegro5/allegro_color.h"
#include "allegro5/allegro_image.h"
#include "allegro5/allegro_font.h"
#include "allegro5/allegro_ttf.h"

#include "math.h"
#include "common.c"

#include <vector>
#include <iostream>

#define pi ALLEGRO_PI

/*
a transform for individual vertices?
a transform looks like it gets created
at 0,0,0
and fits to the size of the vertices
that will be drawn by prim
*/

typedef struct
{
	float x, y, z;
} Vector;

typedef struct
{
	/* the vertex data */
	int n, v_size;
	ALLEGRO_VERTEX *v;
	ALLEGRO_BITMAP *skybox = NULL;
	
	double mouse_look_speed = 0.03;
	double movement_speed = 0.05;
} Verts;
// maybe cpp hates it

Verts ex;
Verts moving;
// the demo
Verts squares;
Verts squares2;
Verts squares3;

typedef struct
{
	Vector position;
	Vector xaxis;				   // right
	Vector yaxis;				   // up
	Vector zaxis;				   // back
	double vertical_field_of_view; /* In radians. */
} Camera;

Camera camera;


//
// adding
//

static void add_vertex(double x, double y, double z, double u, double v, ALLEGRO_COLOR color)
{
	// 1 - size 4
	// 4 - size 10
	int i = ex.n++;
	if (i >= ex.v_size)
	{
		ex.v_size += 1;
		ex.v_size *= 2;
		// void*
		// basically what you want
		ex.v = (ALLEGRO_VERTEX*)realloc(ex.v, ex.v_size * sizeof *ex.v);
	}
	ex.v[i].x = x;
	ex.v[i].y = y;
	ex.v[i].z = z;
	ex.v[i].u = u;
	ex.v[i].v = v;
	ex.v[i].color = color;
}

/* Adds two triangles (6 vertices) to the scene. */
static void add_quad(
	double x, double y, double z,
	double u, double v,
	double ux, double uy, double uz,
	double uu, double uv,
	double vx, double vy, double vz,
	double vu, double vv,
	ALLEGRO_COLOR c1, ALLEGRO_COLOR c2)
{
	add_vertex(x, y, z, u, v, c1);
	add_vertex(x + ux, y + uy, z + uz, u + uu, v + uv, c1);
	add_vertex(x + vx, y + vy, z + vz, u + vu, v + vv, c2);
	
	add_vertex(x + vx, y + vy, z + vz, u + vu, v + vv, c2);
	add_vertex(x + ux, y + uy, z + uz, u + uu, v + uv, c1);
	add_vertex(x + ux + vx, y + uy + vy, z + uz + vz, u + uu + vu, v + uv + vv, c2);
}

//
// triangle
//

static void add_tri()
{
	ALLEGRO_COLOR col =
		al_color_name("red");
	float size = 10;
	float dist = 0.0f;
	int points = 3;
	int mem = 4;
		
	moving.v = (ALLEGRO_VERTEX*) realloc(moving.v, mem*sizeof*moving.v);
	
	moving.v[0] = {
		.x = -15.f,
		 .y = -15.f,
		  .z = dist,
		.u = size, .v = size,
		.color = col };
		
	moving.v[1] = {
		.x = 15.f,
		.y = -15.f,
		.z = dist,
		.u = size, .v = size,
		.color = col };
		
	moving.v[2] = {
		.x = 15.f,
		.y = 15.f,
		.z = dist,
		.u = size, .v = size };
		
	moving.n = points;
}

static void move_try()
{
	float x, y, z;
	x = 0.f;
	y = 0.f;
	z = -80.f;
	
	double time = al_get_time();
	float my_t = sin(2.0f * time);
	
	ALLEGRO_TRANSFORM mt;
	al_identity_transform(&mt);
    al_rotate_transform_3d(&mt, 0, 0, 1, -my_t);
    al_translate_transform_3d(&mt, x, y, z);
	al_use_transform(&mt);
	
	al_draw_prim(moving.v, NULL, NULL, 0, moving.n, ALLEGRO_PRIM_TRIANGLE_LIST);
}

static void add_skybox()
{
	Vector p = camera.position;
	ALLEGRO_COLOR c1 = al_color_name("black");
	ALLEGRO_COLOR c2 = al_color_name("blue");
	ALLEGRO_COLOR c3 = al_color_name("white");
	
	ALLEGRO_COLOR c4 = al_color_name("green");
	ALLEGRO_COLOR c5 = al_color_name("black");
	double a = 0, b = 0;
	a = 128;
	b = 128;
	
	int far = 100;
	int small = 20;
	int uv_scale = far*2;

	/* Back skybox wall. */
	add_quad(
			p.x - far, p.y - far, p.z - far,
			a * 4, b * 2,
			 uv_scale, 0, 0,
			 -a, 0,
			 0, uv_scale, 0,
			 0, -b,
			 c5, c3);
	/* Front skybox wall. */
	add_quad(
		p.x - far, p.y - far, p.z + far,
		a, b * 2,
			 uv_scale, 0, 0,
			 a, 0,
			 0, uv_scale, 0,
			 0, -b,
			 c5,c3);
	/* Left skybox wall. */
	add_quad(
		p.x - 50, p.y - 50, p.z - 50,
		0, b * 2,
		0, 0, 100,
			 a, 0,
			 0, 100, 0,
			 0, -b,
			 c1, c2);
	/* Right skybox wall. */
	add_quad(p.x + 50, p.y - 50, p.z - 50, a * 3, b * 2,
			 0, 0, 100, -a, 0,
			 0, 100, 0, 0, -b,
			 c1, c2);

	/* Top of skybox. */
	// 1
	add_vertex(p.x - 50, p.y + 50, p.z - 50, a, 0, c2);
	add_vertex(p.x + 50, p.y + 50, p.z - 50, a * 2, 0, c2);
	
	add_vertex(p.x, p.y + 50, p.z, a * 1.5, b * 0.5, c4);

	// 2
	add_vertex(p.x + 50, p.y + 50, p.z - 50, a * 2, 0, c2);
	add_vertex(p.x + 50, p.y + 50, p.z + 50, a * 2, b, c2);
	
	add_vertex(p.x, p.y + 50, p.z, a * 1.5, b * 0.5, c4);

	// 3
	add_vertex(p.x + 50, p.y + 50, p.z + 50, a * 2, b, c2);
	add_vertex(p.x - 50, p.y + 50, p.z + 50, a, b, c2);
	
	add_vertex(p.x, p.y + 50, p.z, a * 1.5, b * 0.5, c4);

	// 4
	add_vertex(p.x - 50, p.y + 50, p.z + 50, a, b, c2);
	add_vertex(p.x - 50, p.y + 50, p.z - 50, a, 0, c2);
	
	add_vertex(p.x, p.y + 50, p.z, a * 1.5, b * 0.5, c4);
}

//
// setup
//

static void setup_3d_projection(void)
{
	ALLEGRO_TRANSFORM projection;
	ALLEGRO_DISPLAY *display = al_get_current_display();

	double dw = al_get_display_width(display);
	double dh = al_get_display_height(display);
	double f;

	al_identity_transform(&projection);
	al_translate_transform_3d(&projection, 0, 0, -1);
	f = tan(camera.vertical_field_of_view / 2);

	al_perspective_transform(&projection, -1 * dw / dh * f, f, 1, f * dw / dh, -f, 1000);

	al_use_projection_transform(&projection);
}

static void setup_camera(void)
{
	camera.xaxis.x = 1;
	camera.yaxis.y = 1;
	camera.zaxis.z = 1;
	camera.position.y = 2;
	camera.vertical_field_of_view = 60 * pi / 180;
}

//
// squares
//

static void add_squares(void)
{
	ALLEGRO_COLOR col =
		al_color_name("red");
	float size = 30.f;
	float dist = 0.0f;
	int square_points = 6;
	int number_of_squares = 1;
	int mem = square_points*number_of_squares+1;
		
	squares.v = (ALLEGRO_VERTEX*) realloc(squares.v, mem*sizeof*squares.v);
	
	// 1
	squares.v[0] = {
		.x = -15.f,
		 .y = -15.f,
		  .z = dist,
		.u = size, .v = size,
		.color = col };
		
	squares.v[1] = {
		.x = -15.f,
		.y = 15.f,
		.z = dist,
		.u = size, .v = size,
		.color = col };
		
	squares.v[2] = {
		.x = 15.f,
		.y = 15.f,
		.z = dist,
		.u = size, .v = size };
	
	// 2
	squares.v[3] = {
		.x = 15.f,
		.y = 15.f,
		.z = dist,
		.u = size, .v = size,
		.color = col };
		
	squares.v[4] = {
		.x = 15.f,
		.y = -15.f,
		.z = dist,
		.u = size, .v = size,
		.color = col };
		
	squares.v[5] = {
		.x = -15.f,
		.y = -15.f,
		.z = dist,
		.u = size, .v = size };
		
	// im not adding to n++
	squares.n = square_points;
	// copy it
	squares2 = squares;
	squares2.n = square_points;
	// copy
	squares3 = squares;
	squares3.n = square_points;
}

static void square_transform(void)
{
	float dist = -300.f;
	float max = 40.f;
	float three_tenths = 33.f;
	float speed = al_get_time() / pi;
	
	ALLEGRO_TRANSFORM st;
	al_identity_transform(&st);

	// rotate square first
	al_rotate_transform_3d(
		&st,
		0.f, 0.f, 1.f, -speed*8.f);
	
	// rotate axis and -z
	ALLEGRO_TRANSFORM at;
	al_identity_transform(&at);
	al_translate_transform_3d(
		&at,
		0.f, max, dist);
	al_rotate_transform_3d(
		&at,
		0.f, 0.f, 1.f, speed);
		
	al_compose_transform(&st, &at);
	al_use_transform(&st);

	// draw first
	al_draw_prim(squares.v, NULL, NULL, 0, squares.n, ALLEGRO_PRIM_TRIANGLE_LIST);
	
	// draw second
	al_identity_transform(&st);
	al_rotate_transform_3d(
		&st,
		0.f, 0.f, 1.f, -speed*8.f);
	
	al_identity_transform(&at);
	al_translate_transform_3d(
		&at,
		-three_tenths, -three_tenths, dist);
	al_rotate_transform_3d(
		&at,
		0.f, 0.f, 1.f, speed);
		
	al_compose_transform(&st, &at);
		
	al_use_transform(&st);
	al_draw_prim(squares2.v, NULL, NULL, 0, squares2.n, ALLEGRO_PRIM_TRIANGLE_LIST);
	
	//
	// draw third
	al_identity_transform(&st);
	al_rotate_transform_3d(
		&st,
		0.f, 0.f, 1.f, -speed*8.f);
	al_identity_transform(&at);
	al_translate_transform_3d(
		&at,
		three_tenths, -three_tenths, dist);
	al_rotate_transform_3d(
		&at,
		0.f, 0.f, 1.f, speed);
	al_compose_transform(&st, &at);
	al_use_transform(&st);
	al_draw_prim(squares3.v, NULL, NULL, 0, squares.n, ALLEGRO_PRIM_TRIANGLE_LIST);
	// done
}


//
// scene
//

static void draw_scene(
ALLEGRO_FONT *font)
{
	/* We save Allegro's projection so we can restore it for drawing text. */
	ALLEGRO_TRANSFORM projection = *al_get_current_projection_transform();
	ALLEGRO_TRANSFORM t;
	
	ALLEGRO_COLOR back = al_color_name("black");

	setup_3d_projection();
	al_clear_to_color(back);

	/* We use a depth buffer. */
	al_set_render_state(ALLEGRO_DEPTH_TEST, 1);
	al_clear_depth_buffer(1);

	/* Recreate the entire scene geometry */
    // sky box stuff
	ex.n = 0;
	add_skybox();

	/* Construct cam */
	al_build_camera_transform(
		&t,
		camera.position.x, camera.position.y, camera.position.z,
		camera.position.x - camera.zaxis.x,
		camera.position.y - camera.zaxis.y,
		camera.position.z - camera.zaxis.z,
		camera.yaxis.x, camera.yaxis.y, camera.yaxis.z);
	al_use_transform(&t);
	
	al_draw_prim(ex.v, NULL, ex.skybox, 0, ex.n, ALLEGRO_PRIM_TRIANGLE_LIST);
	
	// why have to draw prim here?
	square_transform();
	

	// probably for text
	al_identity_transform(&t);
	al_use_transform(&t);
	al_use_projection_transform(&projection);
	al_set_render_state(ALLEGRO_DEPTH_TEST, 0);

	/* Draw some text. */
	// 1080, 2400
	ALLEGRO_DISPLAY *display =
		al_get_current_display();
	double sw = al_get_display_width(display);
	double sh = al_get_display_height(display);
	ALLEGRO_COLOR front = al_color_name("white");

	al_draw_textf(font, front, sw/2.f, sh/2.f,
		ALLEGRO_ALIGN_CENTER,
				  "Classic");
}


//
// main
//

int main(int argc, char **argv)
{
	ALLEGRO_DISPLAY *display;
	ALLEGRO_EVENT_QUEUE *queue;
	ALLEGRO_FONT *font;
	ALLEGRO_TIMER *timer;

	if (!al_init())
	{
		std::cerr << "Not initialised.";
		return 1;
	}

	display = al_create_display(720, 1600);
	if (!display)
	{
		std::cerr << "No display.";
		return 1;
	}

	al_init_font_addon();
	al_init_ttf_addon();
	al_init_primitives_addon();
	init_platform_specific();
	
	setup_camera();
	add_squares();

	font = al_load_font("/system/fonts/Roboto-Regular.ttf", 30, 0);

	if (!font)
		return 2;

	queue = al_create_event_queue();

	al_register_event_source(queue, al_get_display_event_source(display));

	timer = al_create_timer(1 / 60.0);
	al_register_event_source(queue, al_get_timer_event_source(timer));

	al_start_timer(timer);
	int redraw = 1;
	
	//tri_make_transform();

	while (true)
	{
		ALLEGRO_EVENT event;

		al_wait_for_event(queue, &event);

		if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
			break;
			
//
// movement
//

		else if (event.type == ALLEGRO_EVENT_TIMER)
		{
			//handle_movement(timer);
			redraw = 1;
		}

		else if (event.type == ALLEGRO_EVENT_DISPLAY_HALT_DRAWING)
		{
			al_stop_timer(timer);
			redraw = 0;
			al_acknowledge_drawing_halt(display);
		}

		else if (event.type == ALLEGRO_EVENT_DISPLAY_RESUME_DRAWING)
		{
			al_acknowledge_drawing_resume(display);
			al_start_timer(timer);
		}
		

		if (redraw && al_is_event_queue_empty(queue))
		{
			draw_scene(font);
			
			al_flip_display();
			redraw = 0;
		}
	}

	al_destroy_font(font);
	al_destroy_display(display);

	return 0;
}