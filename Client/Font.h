#pragma once

#include <ft2build.h>
#include <freetype/freetype.h>
#include "Shader.hpp"
#include <memory>
#include "Camera.hpp"

struct Char {
    GLuint textureID;   // ID handle of the glyph texture
    glm::ivec2 size;    // Size of glyph
    glm::ivec2 bearing;  // Offset from baseline to left/top of glyph
    GLuint advancex;    // Horizontal offset to advance to next glyph
    GLuint advancey;    // Vertical offset to advance to next glyph
};

class Font {
    FT_Face face;
    FT_Library ft;

    GLuint tex;
    GLuint vao;
    GLuint vbo;
    // Shader program for object
    std::unique_ptr<Shader> _objectShader;
    std::unordered_map<char, Char> _textures;

    glm::vec2 screenSize;

    
public:
    
    glm::vec4 _textColor;
    glm::vec4 _backgroundColor;
    bool center = true;

    Font();

    void setScreenSize(glm::vec2 s);

    void render_text(const char* text, float x, float y, float sx, float sy);

    void display(
        bool depth, std::unique_ptr<Camera> const &camera, glm::mat4 toWorld, std::string str, float size = 1.0f,
        float xcoord = -1, float ycoord = -1);

    void display(std::string str, float xcoord = -1, float ycoord = -1);
};
