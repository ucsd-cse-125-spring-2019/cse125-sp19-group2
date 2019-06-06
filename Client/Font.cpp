#include "Font.h"
#include "Font.h"
#include <iostream>
#include <algorithm>

Font::Font() {
    if (FT_Init_FreeType(&ft)) {
        fprintf(stderr, "Could not init freetype library\n");
    }

    if (FT_New_Face(ft, "./Resources/Font/ComicSansBold.ttf", 0, &face)) {
        fprintf(stderr, "Could not open font\n");
    }
    FT_Set_Pixel_Sizes(face, 0, 52);
    
    FT_GlyphSlot g = face->glyph;

    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    //glUniform1i(uniform_tex, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Load first 128 characters of ASCII set
    for (GLubyte c = 0; c < 128; c++)
    {
        // Load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            fprintf(stderr, "ERROR::FREETYTPE: Failed to load Glyph\n");
            continue;
        }
        // Generate texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            g->bitmap.width,
            g->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            g->bitmap.buffer
        );
        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Now store character for later use
        Char character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x,
            face->glyph->advance.y
        };
        _textures.insert(std::pair<char, Char>(c, character));
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    // Destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    // Allocate member variables
    _objectShader = std::make_unique<Shader>();

    _objectShader->LoadFromFile(GL_VERTEX_SHADER, "./Resources/Shaders/font.vert");
    _objectShader->LoadFromFile(GL_FRAGMENT_SHADER, "./Resources/Shaders/font.frag");
    _objectShader->CreateProgram();
    _textColor = glm::vec4(1,1,1,1);
    _backgroundColor = glm::vec4(0,0,0,0);
    screenSize = glm::ivec2(1280, 720);
    _fb = std::make_unique<FrameBuffer>(1280, 720);
}

void Font::setScreenSize(glm::ivec2 s) {
    screenSize = s;
    _fb->resize(s.x, s.y);
}
#pragma optimize("", off)
void Font::render_text(const char* text, float x, float y, float sx, float sy) {
    
    const char* p;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if(center) {
        bool yModified = false;
        // calculate the length first
        for (p = text; *p; p++) {
            auto res = _textures.find(*p);

            if(res == _textures.end()) {
                continue;
            }
            auto c = res->second;
            x -= (c.advancex / 128) * sx;
            if(!yModified) {
                y -= (c.advancey / 128) * sy;
                yModified = true;
            }
		}
    }
    
    GLfloat box[4];
    GLfloat viewport[4];
    glGetFloatv(GL_SCISSOR_BOX, box);
    glGetFloatv(GL_VIEWPORT, viewport);

	if(cut) {
	    glEnable(GL_SCISSOR_TEST);
        float cx = x;
        float cy = y;
        float w = 0;
        float h = 0;
        bool init = false;
        for (p = text; *p; p++) {
            auto res = _textures.find(*p);
            if(res == _textures.end()) {
                continue;
            }
            auto c = res->second;
            if(!init) {
                cx += c.bearing.x * sx;
                cy -= (c.size.y - c.bearing.y) * sy;
                init = true;
            }
            w += (c.advancex / 64) * sx;
            h = std::max<float>(h, c.size.y * sy);
		}
        cx = ((cx + 1) / 2.0) - edge.x;
        cy = ((cy + 1) / 2.0) - edge.y;
        cx *= viewport[2];
        cy *= viewport[3];

        w /= 2.0;
        h /= 2.0;
        w += edge.x * 2;
        h += edge.y * 2;
        w *= viewport[2];
        h *= viewport[3];
        if(h > 0 && w > 0){
            glScissor(cx, cy, w, h);
		}
	}

    glClearColor(_backgroundColor.r, _backgroundColor.g,_backgroundColor.b,_backgroundColor.a);
    glClear(GL_COLOR_BUFFER_BIT);

    for (p = text; *p; p++) {

        auto res = _textures.find(*p);

        if(res == _textures.end()) {
            continue;
        }
        auto c = res->second;

        GLfloat xpos = x + c.bearing.x * sx;
        GLfloat ypos = y - (c.size.y - c.bearing.y) * sy;

        GLfloat w = c.size.x * sx;
        GLfloat h = c.size.y * sy;
        // Update VBO for each character
        GLfloat vertices[6][4] = {
            { xpos,     ypos + h,   0.0, 0.0 },            
            { xpos,     ypos,       0.0, 1.0 },
            { xpos + w, ypos,       1.0, 1.0 },

            { xpos,     ypos + h,   0.0, 0.0 },
            { xpos + w, ypos,       1.0, 1.0 },
            { xpos + w, ypos + h,   1.0, 0.0 }           
        };
        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, c.textureID);
        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);

        x += (c.advancex / 64) * sx;
        y += (c.advancey / 64) * sy; 
    }

    if(cut) {
        glScissor(box[0], box[1], box[2], box[3]);
	    glDisable(GL_SCISSOR_TEST);
    }
}
#pragma optimize("", on)
void Font::display(
    bool depth, std::unique_ptr<Camera> const& camera, glm::mat4 toWorld, const std::string & str, float size,
    float xcoord, float ycoord) {
    _objectShader->Use();
    glBindVertexArray(vao);
    glDepthMask(GL_FALSE);
    if (!depth) {
        //glClear(GL_DEPTH_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);
    }

    _objectShader->set_uniform("textColor", _textColor);
    glActiveTexture(GL_TEXTURE0);
    _objectShader->set_uniform("tex", 0);

    glm::mat4 projection = camera->projection_matrix();
    glm::mat4 mv = camera->view_matrix() * toWorld;
    _objectShader->set_uniform("ModelView", mv);
    _objectShader->set_uniform("ProjectionMatrix", projection);

    float sx = 2.0 * size / screenSize.x;
    float sy = 2.0 * size / screenSize.y;

    // (screenSize / size) / 2.0
    // sx = 2.0 * size / screenSize;
    // () = 2.0 / sx;
    if(xcoord < 0) {
        xcoord = 1.0 / sx;
    }
    if(ycoord < 0) {
        ycoord = 1.0 / sy;
    }

    render_text(
        str.c_str(),
        -1 + xcoord * sx, 1 - ycoord * sy, sx, sy);
    if (!depth) {
        glEnable(GL_DEPTH_TEST);
    }
    glUseProgram(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glDepthMask(GL_TRUE);
}

void Font::display(const std::string & str, float size, float xcoord, float ycoord) {
    _objectShader->Use();
    glDepthMask(GL_FALSE);
    glBindVertexArray(vao);
    //glClear(GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    
    glm::mat4 identity = glm::mat4(1.0f);
    _objectShader->set_uniform("ModelView", identity);
    _objectShader->set_uniform("ProjectionMatrix", identity);

    _objectShader->set_uniform("textColor", _textColor);
    glActiveTexture(GL_TEXTURE0);
    _objectShader->set_uniform("tex", 0);
    float sx = 2.0 * size / screenSize.x;
    float sy = 2.0 * size / screenSize.y;

    if(xcoord < 0) {
        xcoord = 1.0 / sx;
    }
    if(ycoord < 0) {
        ycoord = 1.0 / sy;
    }

    render_text(
        str.c_str(),
        -1 + xcoord * sx, 1 - ycoord * sy, sx, sy);
    glEnable(GL_DEPTH_TEST);
    glUseProgram(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glDepthMask(GL_TRUE);
}

void Font::renderToTexture(const std::string & str, float size, float xcoord, float ycoord) {
    _fb->renderScene([&]() {
        GLfloat bkColor[4];
        glGetFloatv(GL_COLOR_CLEAR_VALUE, bkColor);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT |GL_DEPTH_BUFFER_BIT);
        display(str, size, xcoord, ycoord);
        glClearColor(bkColor[0], bkColor[1],bkColor[2],bkColor[3]);
    });
}

GLuint Font::getTexture() const {
    return _fb->getRGBA();
}
