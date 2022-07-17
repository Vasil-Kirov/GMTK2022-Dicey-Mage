
#ifndef RENDERER_H
#define RENDERER_H

#include <ft2build.h>
#include FT_FREETYPE_H


#pragma pack(push, 1)

typedef struct { f32 x; f32 y; } v2;
typedef struct { f32 x; f32 y; f32 z; f32 w; } v4;
typedef struct { v2 pos; v2 tex_coord; } Render_Target;

#pragma pack(pop)

inline v2 V2(f32 x, f32 y)
{
	v2 result = {x, y};
	return result;
}

inline v4 V4(f32 x, f32 y, f32 z, f32 w)
{
	v4 result = {x, y, z, w};
	return result;
}


typedef struct {int width; int height;} Init_Info;
typedef struct {
	GLuint vao;
	GLuint vbo;
	GLuint ebo;
	GLuint shader_program;
	int width;
	int height;
} Renderer_State;

typedef struct {
	FT_Library library;
	FT_Face face;
} Font_Info;

typedef struct {
    unsigned int texture_id; // ID handle of the glyph texture
    v2           size;       // Size of glyph
    v2           bearing;    // Offset from baseline to left/top of glyph
    f32          advance;    // Offset to advance to next glyph
} Character;

typedef struct {
	unsigned int texture;
	f32          width_over_height;
} Texture_Desc;

typedef struct {
	char *key;
	Texture_Desc value;
} Texture_Hash_Map;


// NOTE(Vasko): the best function
f32 normalize_between(f32 x, f32 min_x, f32 max_x, f32 from, f32 to);


/*
static const char *vertex_shader_src = {
#include "shaders/shader.vert"
};

static const char *fragment_shader_src = {
#include "shaders/shader.frag"
};
*/


/*****************************************************************************
 * text:   the text to draw                                                  *
 * x, y:   the position, x and y are floats between -1 and 1                 *
 * color:  the color of the text, 0xRRGGBBAA                                 *
 * scaler: the size of the text will be scaled by this value, 1 is default   *
 *****************************************************************************/

void r_draw_text(char *text, float x, float y, uint32_t color, float scaler);


/******************************************************************
* Takes a color represented as a 32 bit number                    *
* Outputs a 4 component vector with values between 0 and 1        *
*******************************************************************/

v4 extract_color_v4_from_u32(uint32_t in);


Texture_Desc get_texture_by_name(const char *name);

void load_font_glyphs(int w, int h);
void load_data_file();

#endif //RENDERER_H
