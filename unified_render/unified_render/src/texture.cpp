// Unified Render - General purpouse game engine
// Copyright (C) 2021, Unified Render contributors
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// ----------------------------------------------------------------------------
// Name:
//      texture.cpp
//
// Abstract:
//      Defines some functions to bind a BinaryImage to an OpenGL object
//      by using the OpenGL API for managing the resources of it.
// ----------------------------------------------------------------------------

#include <algorithm>

#include "unified_render/texture.hpp"
#include "unified_render/path.hpp"
#include "unified_render/print.hpp"
#include "unified_render/framebuffer.hpp"
#include "unified_render/utils.hpp"

//
// Texture
//
UnifiedRender::Texture::Texture(void) {

}

UnifiedRender::Texture::Texture(const std::string& path)
    : BinaryImage(path)
{

}

UnifiedRender::Texture::Texture(const UnifiedRender::IO::Asset::Base* asset)
    : BinaryImage((asset == nullptr) ? "" : asset->abs_path)
{

}

UnifiedRender::Texture::Texture(size_t _width, size_t _height)
    : BinaryImage(_width, _height)
{

}

UnifiedRender::Texture::~Texture(void) {
    if(gl_tex_num) {
        delete_opengl();
    }
}

// This dummy texture helps to avoid crashes due to missing buffers or so, and also gives
// visual aid of errors
void UnifiedRender::Texture::create_dummy() {
    width = 8;
    height = 8;
    buffer = std::make_unique<uint32_t[]>(width * height);
    if(buffer.get() == nullptr) {
        CXX_THROW(TextureException, "Dummy", "Out of memory for dummy texture");
    }

    // Fill in with a permutation pattern of pink and black
    // This should be autovectorized by gcc
    for(size_t i = 0; i < width * height; i++) {
        buffer.get()[i] = 0xff000000 | (i * 16);
    }
}

void UnifiedRender::Texture::to_opengl(TextureOptions options) {
    if(gl_tex_num) {
        delete_opengl();
    }

    glGenTextures(1, &gl_tex_num);
    glBindTexture(GL_TEXTURE_2D, gl_tex_num);
    glTexImage2D(GL_TEXTURE_2D, 0, options.internal_format, width, height, 0, options.format, options.type, buffer.get());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, options.wrap_s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, options.wrap_t);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, options.min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, options.mag_filter);

    // We will free up the texture if we don't plan on editing it since it's on the GPU now
    if(!options.editable) {
        buffer.reset();
    }
}

void UnifiedRender::Texture::gen_mipmaps() const {
    glBindTexture(GL_TEXTURE_2D, gl_tex_num);
    glGenerateMipmap(GL_TEXTURE_2D);
}

// Converts the texture into a OpenGL texture, and assigns it a number
void UnifiedRender::Texture::to_opengl(SDL_Surface* surface) {
    if(surface->w == 0 || surface->h == 0) {
        return;
    }

    int colors = surface->format->BytesPerPixel;
    GLuint texture_format;
    if(colors == 4) {
        // Alpha
        if(surface->format->Rmask == 0x000000ff) {
            texture_format = GL_RGBA;
        }
        else {
            texture_format = GL_BGRA;
        }
    }
    else {
        // No alpha
        if(surface->format->Rmask == 0x000000ff) {
            texture_format = GL_RGB;
        }
        else {
            texture_format = GL_BGR;
        }
    }

    int alignment = 8;
    while(surface->pitch % alignment) {
        alignment >>= 1; // x%1==0 for any x
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);

    int expected_pitch = (surface->w * surface->format->BytesPerPixel + alignment - 1) / alignment * alignment;
    if(surface->pitch - expected_pitch >= alignment) {
        // Alignment alone wont't solve it now
        glPixelStorei(GL_UNPACK_ROW_LENGTH, surface->pitch / surface->format->BytesPerPixel);
    }
    else {
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    }
    
    glGenTextures(1, &gl_tex_num);
    glBindTexture(GL_TEXTURE_2D, gl_tex_num);
    glTexImage2D(GL_TEXTURE_2D, 0, colors, surface->w, surface->h, 0, texture_format, GL_UNSIGNED_BYTE, surface->pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenerateMipmap(GL_TEXTURE_2D);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
}


// Binds the texture to the current OpenGL context
void UnifiedRender::Texture::bind(void) const {
    glBindTexture(GL_TEXTURE_2D, gl_tex_num);
}

// Deletes the OpenGL representation of this texture
void UnifiedRender::Texture::delete_opengl() {
    glDeleteTextures(1, &gl_tex_num);
}

UnifiedRender::Texture::Texture(TTF_Font* font, UnifiedRender::Color color, const std::string& msg) {
    // TTF_SetFontStyle(g_ui_context->default_font, TTF_STYLE_BOLD);
    SDL_Color black_color = {
        static_cast<Uint8>(color.r * 255.f),
        static_cast<Uint8>(color.g * 255.f),
        static_cast<Uint8>(color.b * 255.f),
        0
    };

    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, msg.c_str(), black_color);
    if(surface == nullptr) {
        CXX_THROW(std::runtime_error, std::string() + "Cannot create text surface: " + TTF_GetError());
    }

    buffer.reset();
    width = static_cast<size_t>(surface->w);
    height = static_cast<size_t>(surface->h);
    this->to_opengl(surface);
    SDL_FreeSurface(surface);

    const char* error_msg = SDL_GetError();
    if(error_msg[0] != '\0') {
        print_error("SDL error %s", error_msg);
    }
}

#define STB_IMAGE_WRITE_IMPLEMENTATION 1
#include "unified_render/stb_image_write.h"
void UnifiedRender::Texture::to_file(const std::string& filename) {
    int channel_count = 4;
    int stride = channel_count * width;
    int data_size = stride * height;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gl_tex_num);

    GLubyte* data = (GLubyte*)malloc(data_size);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_write_png(filename.c_str(), width, height, channel_count, data, stride);
}

//
// Texture array
//

// Creates a new texture array
UnifiedRender::TextureArray::TextureArray(const std::string& path, size_t _tiles_x, size_t _tiles_y)
    : BinaryImage(path),
    tiles_x{ _tiles_x },
    tiles_y{ _tiles_y }
{

}

// Uploads the TextureArray to the driver
void UnifiedRender::TextureArray::to_opengl(void) {
    glGenTextures(1, &gl_tex_num);
    glBindTexture(GL_TEXTURE_2D_ARRAY, gl_tex_num);

    // set up texture handle parameters
    // glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);  // !single image!
    // glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 1);   // !single image! mat->mips == 1
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

    size_t p_dx = width / tiles_x; // pixels of each tile in x
    size_t p_dy = height / tiles_y; // pixels of each tile in y
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, p_dx, p_dy, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glPixelStorei(GL_UNPACK_ROW_LENGTH, width);
    glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, height);

    for(size_t x = 0; x < tiles_x; x++) {
        for(size_t y = 0; y < tiles_y; y++) {
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, x * tiles_x + y, p_dx, p_dy, 1, GL_RGBA, GL_UNSIGNED_BYTE, buffer.get() + (x * p_dy * width + y * p_dx));
        }
    }
    //glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    // unbind texture handle
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
}

//
// Texture manager
//

std::shared_ptr<UnifiedRender::Texture> UnifiedRender::TextureManager::get_white(void) {
    if(white.get() == nullptr) {
        white = std::make_shared<UnifiedRender::Texture>(1, 1);
        white->buffer.get()[0] = 0xFFFFFFFF;
        white->to_opengl();
    }
    return std::shared_ptr<UnifiedRender::Texture>(white);
}

UnifiedRender::TextureManager::~TextureManager(void) {
    textures.clear();
}

//
// Finds a texture in the list of a texture manager
// if the texture is already in the list we load the saved texture from the list
// instead of loading it from the disk.
//
// Otherwise we load it from the disk and add it to the saved texture list
//
// The object returned is a pointer and we will not give ownership of textures in the list
// and program should not modify the contents of it since it will differ from the texture
// on the disk, and our main point is to mirror loaded textures from the disk - not modify
// them.
//
std::shared_ptr<UnifiedRender::Texture> UnifiedRender::TextureManager::load(const std::string& path, TextureOptions options) {
    // Find texture when wanting to be loaded and load texture from cached texture list
    auto key = std::make_pair(path, options);
    auto it = textures.find(key);
    if(it != textures.end()) {
        return (*it).second;
    }

    print_info("Loaded and cached texture %s", path.c_str());

    // Otherwise texture is not in our control, so we create a new texture
    std::shared_ptr<UnifiedRender::Texture> tex;
    try {
        tex = std::make_shared<UnifiedRender::Texture>(path);
    } catch(BinaryImageException&) {
        tex = std::make_shared<UnifiedRender::Texture>();
        tex->create_dummy();
    }
    tex->to_opengl(options);
    if(options.min_filter == GL_NEAREST_MIPMAP_NEAREST || options.min_filter == GL_NEAREST_MIPMAP_LINEAR || options.min_filter == GL_LINEAR_MIPMAP_NEAREST || options.min_filter == GL_LINEAR_MIPMAP_LINEAR) {
        tex->gen_mipmaps();
    }
    textures[key] = tex;
    return textures[key];
}

std::shared_ptr<UnifiedRender::Texture> UnifiedRender::TextureManager::load(const UnifiedRender::IO::Asset::Base* asset, TextureOptions options) {
    return this->load((asset == nullptr) ? "" : asset->abs_path, options);
}