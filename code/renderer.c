#include "renderer.h"

static SDL_Window *window;
static Renderer_State renderer;
static Font_Info font;
static Character char_map[128] = {0};
static char *base_path;
static Texture_Hash_Map *texture_map;


f32 normalize_between(f32 x, f32 min_x, f32 max_x, f32 from, f32 to)
{
	return (to - from) * ((x - min_x) / (max_x - min_x)) + from;
}


void r_resize(int width, int height)
{
	glViewport(0, 0, width, height);
	renderer.width = width;
	renderer.height = height;
	load_font_glyphs(width, height);
}

v4 extract_color_v4_from_u32(u32 in)
{
	v4 result;
	result.x = ((in >> 24) & 0xFF) / 255.0f;
	result.y = ((in >> 16) & 0xFF) / 255.0f;
	result.z = ((in >> 8)  & 0xFF) / 255.0f;
	result.w = ((in >> 0)  & 0xFF) / 255.0f;
	return result;
}

u32 convert_v4_to_u32(v4 in)
{
	u32 result = 0;
	result |= (((u32)normalize_between(in.x, 0, 1, 0, 255)) << 24) & 0xFF;
	result |= (((u32)normalize_between(in.y, 0, 1, 0, 255)) << 16) & 0xFF;
	result |= (((u32)normalize_between(in.z, 0, 1, 0, 255)) << 8)  & 0xFF;
	result |= (((u32)normalize_between(in.w, 0, 1, 0, 255)) << 0)  & 0xFF;
	return result;	
}

void r_set_uniform_int(const char *uniform_name, int value)
{
	GLint location = glGetUniformLocation(renderer.shader_program, uniform_name);
	glUniform1i(location, value);
}

void r_set_uniform_v4(const char *uniform_name, v4 value)
{
	GLint location = glGetUniformLocation(renderer.shader_program, uniform_name);
	glUniform4f(location, value.x, value.y, value.z, value.w);
}

void r_make_draw_call(int num_of_elements, uint32_t color, int texture_index)
{	
	v4 uniform_color = extract_color_v4_from_u32(color);
	
	r_set_uniform_int("texture1",      texture_index);
	r_set_uniform_int("texture_index", texture_index);
	r_set_uniform_v4("color", uniform_color);
	
	i32 stride = sizeof(Render_Target);
	
	glBindVertexArray(renderer.vao);
	
	glBindBuffer(GL_ARRAY_BUFFER, renderer.vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer.ebo);
	
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void *)(0));
	glEnableVertexAttribArray(0);
	
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void *)(sizeof(v2)));
	glEnableVertexAttribArray(1);
	
	glUseProgram(renderer.shader_program);

	glBindVertexArray(renderer.vao);
	
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawElements(GL_TRIANGLES, num_of_elements, GL_UNSIGNED_INT, 0);
}

void r_draw_text(char *text, float x, float y, uint32_t color, float scaler)
{
	v2 pos = { .x = x, .y = y };
	size_t size = strlen(text);

	//size_t vert_memory_size = sizeof(Render_Target) * size * 4;
	//size_t indx_memory_size = sizeof(int) * size * 6;

	//Render_Target *verts  = alloca( vert_memory_size );
	//unsigned int *indexes = alloca( indx_memory_size );
	

    glActiveTexture(GL_TEXTURE0);
	#define ASCII_HEIGHT 10
	for (int index = 0; index < size; ++index)
	{
		char c = text[index];
		Character char_info = char_map[c];
		
		if(c == '\n')
		{
			float bottom = pos.y - (char_info.size.y - char_info.bearing.y) * scaler;
			
			pos.x = x;
			pos.y = bottom - (char_info.size.y * scaler) * 1.35f;
			continue;
		}
		
		float x_pos = pos.x + char_info.bearing.x * scaler;
		float y_pos = pos.y - (char_info.size.y - char_info.bearing.y) * scaler;
		
		float c_width  = char_info.size.x * scaler;
		float c_height = char_info.size.y * scaler;

		v2 t1 = { .x = 0, .y = 0 };
		v2 t2 = { .x = 1, .y = 1 };

		v2 p1 = { .x = x_pos, .y = y_pos };
		v2 p2 = { .x = x_pos + c_width, .y =  y_pos + c_height };

		//p1.y += ((25*scaler) / ASCII_HEIGHT) - (c_height + char_info.y_offset * scaler);
		//p2.y += ((25*scaler) / ASCII_HEIGHT) - (c_height + char_info.y_offset * scaler);

		//int vert_offset = last_vert;
		
		Render_Target verts[] = {
			(Render_Target){ p1.x, p2.y, t1.x, t1.y },
			(Render_Target){ p1.x, p1.y, t1.x, t2.y },
			(Render_Target){ p2.x, p1.y, t2.x, t2.y },
			(Render_Target){ p2.x, p2.y, t2.x, t1.y },
		};
		
		unsigned int indexes[] = {
			0, 1, 2, 0, 2, 3,
		};
		
		glBindTexture(GL_TEXTURE_2D, char_info.texture_id);
		
		glBindBuffer(GL_ARRAY_BUFFER, renderer.vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer.ebo);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(indexes), indexes);
		
		r_make_draw_call(6, color, 0);
		
        pos.x += char_info.advance * scaler;
	}
}

void r_present(void)
{
	SDL_GL_SwapWindow(window);
}

void r_compile_shaders(void)
{
	/*
	SDL_RWops *vert_file = SDL_RWFromFile("code/shaders/shader.vert", "r");
	SDL_RWops *frag_file = SDL_RWFromFile("code/shaders/shader.frag", "r");
	if(!vert_file)
		V_FATAL("Vertex shader file is missing.");
	if(!frag_file)
		V_FATAL("Fragment shader file is missing");
	
	size_t vert_src_size = vert_file->size(vert_file);
	size_t frag_src_size = frag_file->size(frag_file);
	if(vert_src_size == -1 || frag_src_size == -1)
		V_FATAL("Couldn't determine shader file size: %d", SDL_GetError());
	*/
	
	const char *vertex_shader_src = "#version 330 core\n"
		"\n"
		"layout (location = 0) in vec2 position;\n"
		"layout (location = 1) in vec2 texture;\n"
		"\n"
		"out vec2 texture_coords;\n"
		"\n"
		"void main()\n"
		"{\n"
		"    gl_Position = vec4(position.x, position.y, 1, 1);\n"
		"    texture_coords = texture;\n"
		"}";
	const char *fragment_shader_src = "#version 330 core\n"
		"out vec4 frag_color;\n"
		"in  vec2  texture_coords;\n"
		"\n"
		"uniform int       texture_index = 0;\n"
		"uniform sampler2D texture1;\n"
		"uniform vec4      color;\n"
		"\n"
		"void main()\n"
		"{\n"
		"    vec4 texture_result = texture(texture1, texture_coords);\n"
		"    if(texture_index == 0)\n"
		"        frag_color = texture_result.r * color;\n"
		"    else\n"
		"        frag_color = texture_result * color;\n"
		"}";
	/*
if(vert_file->read(vert_file, vertex_shader_src, vert_src_size, 1) == 0)
		V_FATAL("Failed to read vertex shader source file: %d", SDL_GetError());
	if(frag_file->read(frag_file, fragment_shader_src, frag_src_size, 1) == 0)
		V_FATAL("Failed to read fragment shader source file: %d", SDL_GetError());
	
	vert_file->close(vert_file);
	frag_file->close(frag_file);
	*/
	
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	
	glShaderSource(vertex_shader, 1, &vertex_shader_src, 0);
	glShaderSource(fragment_shader, 1, &fragment_shader_src, 0);
	
	glCompileShader(vertex_shader);
	glCompileShader(fragment_shader);
	
	int success;
	char info_log[512] = {0};
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
	if(!success)
	{
		glGetShaderInfoLog(vertex_shader, 512, NULL, info_log);
		V_FATAL(info_log);
	}
	
	
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
	if(!success)
	{
		glGetShaderInfoLog(fragment_shader, 512, NULL, info_log);
		V_FATAL(info_log);
	}
	
	renderer.shader_program = glCreateProgram();
	glAttachShader(renderer.shader_program, vertex_shader);
	glAttachShader(renderer.shader_program, fragment_shader);
	glLinkProgram(renderer.shader_program);
	glUseProgram(renderer.shader_program);
	
	glGetProgramiv(renderer.shader_program, GL_LINK_STATUS, &success);
	if(!success)
	{
		glGetProgramInfoLog(renderer.shader_program, 512, NULL, info_log);
		fprintf(stderr, info_log);
		exit(1);
	}
}

void r_draw_v4(v4 location, unsigned int texture, u32 color)
{
	
	v2 t1 = { .x = 0, .y = 0 };
	v2 t2 = { .x = 1, .y = 1 };
	
	v2 p1 = { .x = location.x, .y = location.y };
	v2 p2 = { .x = location.z, .y =  location.w };
	
	Render_Target verts[] = {
		(Render_Target){ p1.x, p2.y, t1.x, t1.y },
		(Render_Target){ p1.x, p1.y, t1.x, t2.y },
		(Render_Target){ p2.x, p1.y, t2.x, t2.y },
		(Render_Target){ p2.x, p2.y, t2.x, t1.y },
	};
	
	unsigned int indexes[] = {
		0, 1, 2, 0, 2, 3,
	};
	
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture);
	
	glBindBuffer(GL_ARRAY_BUFFER, renderer.vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer.ebo);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(indexes), indexes);
	
	r_make_draw_call(6, color, 1);
	
}


void r_init(Init_Info info)
{
	
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	
	/* Create window and context */
	window = SDL_CreateWindow("Template Window",
							  SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
							  info.width, info.height,
							  SDL_WINDOW_OPENGL);
	
	SDL_GL_CreateContext(window);
	assert(gladLoadGLLoader(SDL_GL_GetProcAddress));
	V_INFO("Loaded GL Version %s", glGetString(GL_VERSION));
	SDL_GL_SetSwapInterval(1);
	
	
	/* Initialize OpenGL */
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_SCISSOR_TEST);
	
	r_compile_shaders();
	
	/* Set-up GL buffers */
	glGenVertexArrays(1, &renderer.vao);
	glGenBuffers(1, &renderer.vbo);
	glGenBuffers(1, &renderer.ebo);
	
	glBindVertexArray(renderer.vao);
	
	// 10 MB
	glBindBuffer(GL_ARRAY_BUFFER, renderer.vbo);
	glBufferData(GL_ARRAY_BUFFER, 10*1024*1024, NULL, GL_STREAM_DRAW);
	
	// 10 MB
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 10*1024*1024, NULL, GL_STREAM_DRAW);
	
	assert(glGetError() == 0);
	
	int error_code = FT_Init_FreeType(&font.library);
	if (error_code != 0)
	{
		V_FATAL("Failed to load the FreeType library: %d", error_code);
	}
	
	error_code = FT_New_Face(font.library, "assets/Gidole-Regular.ttf", 0, &font.face);
	if (error_code != 0)
	{
		V_FATAL("Failed to load font: %d", error_code);
	}
	
	FT_Set_Pixel_Sizes(font.face, 0, 48);
	
	r_resize(info.width, info.height);
	
	base_path = SDL_GetBasePath();
	load_data_file();
}

#define CONSUME(type) (type *)_consume(&data, start, file_size, sizeof(type))

void *_consume(void **data, void * start, size_t file_size, size_t size)
{
	void *result = *data;
	*data = (u8 *)*data + size;
	assert((size_t)((u8 *)*data - (u8 *)start) <= (size_t)file_size);
	return result;
}

void load_data_file()
{
	char asset_file_path[4096] = {0};
	strcat(asset_file_path, base_path);
	strcat(asset_file_path, "assets/output.vdata");
	SDL_RWops *asset_file = SDL_RWFromFile(asset_file_path, "rb");
	if(!asset_file)
		V_FATAL("Couldn't find data file");
	size_t file_size = asset_file->size(asset_file);
	u8 *data = (u8 *)SDL_malloc(file_size);
	if(!data)
		V_FATAL("Couldn't allocate memory for data file");
	
	u8 *start = data;
	
	asset_file->read(asset_file, data, file_size, 1);
	asset_file->close(asset_file);
	while((size_t)(data - start) < file_size)
	{
		assert(*CONSUME(u8) == 'i');
		assert(*CONSUME(u8) == 'e');
		assert(*CONSUME(u8) == 'h');
		assert(*CONSUME(u8) == '\0');
		size_t name_len = strlen((char *)data) + 1;
		u8 *name = data;
		_consume(&data, start, file_size, name_len);
		u32 img_size = *CONSUME(u32);
		u32 width  = *CONSUME(u32);
		u32 height = *CONSUME(u32); 
		u8 *pixels = data;
		_consume(&data, start, file_size, img_size);
		
		unsigned int texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D,
					 0,
					 GL_RGBA,
					 width,
					 height,
					 0,
					 GL_RGBA,
					 GL_UNSIGNED_BYTE,
					 pixels
					 );
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
		Texture_Desc desc = {
			.texture = texture,
			.width_over_height = (f32)width/(f32)height
		};
		shput(texture_map, name, desc);
		
	}
	SDL_free(data);
}

Texture_Desc get_texture_by_name(const char *name)
{
	return shget(texture_map, name);
}

void load_font_glyphs(int w, int h)
{
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte allignment restrection
	
	
	for(unsigned char c = 0; c < 128; c++)
	{
		int error_code = FT_Load_Char(font.face, c, FT_LOAD_RENDER);
		if (error_code != 0)
		{
			V_FAIL("Failed to load glyph for character %c, font may be corrupt: %d", c, error_code);
			continue;
		}
		
		// Generate texture
		unsigned int texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D,
					 0,
					 GL_RED,
					 font.face->glyph->bitmap.width,
					 font.face->glyph->bitmap.rows,
					 0,
					 GL_RED,
					 GL_UNSIGNED_BYTE,
					 font.face->glyph->bitmap.buffer
					 );
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
		v2 size    = V2(
						normalize_between((f32)font.face->glyph->bitmap.width, 0, (f32)w, 0, 1.5),
						normalize_between((f32)font.face->glyph->bitmap.rows, 0, (f32)h, 0, 1.5));
		v2 bearing = V2(
						normalize_between((f32)font.face->glyph->bitmap_left, 0, (f32)w, 0, 1.5),
						normalize_between((f32)font.face->glyph->bitmap_top, 0, (f32)h, 0, 1.5));
		
		f32 advance = normalize_between((f32)(font.face->glyph->advance.x >> 6), 0, (f32)w, 0, 1.5);
		Character character = {
			.texture_id = texture,
			.size       = size,
			.bearing    = bearing,
			.advance    = advance
		};
		char_map[c] = character;
	}
	
}



