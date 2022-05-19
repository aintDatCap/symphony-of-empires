// Eng3D - General purpouse game engine
// Copyright (C) 2021, Eng3D contributors
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
//      mesh.hpp
//
// Abstract:
//      Does some important stuff.
// ----------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <vector>
#include <string>

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

#ifdef E3D_BACKEND_OPENGL
#   include <GL/glew.h>
#   include <GL/gl.h>
#   include <GL/glu.h>
#elif defined E3D_BACKEND_RGX
#   include <gccore.h>
#endif

#ifdef E3D_BACKEND_OPENGL
namespace Eng3D::OpenGL {
    class VAO {
        GLuint id;
    public:
        VAO();
        ~VAO();
        VAO(const VAO&) = default;
        VAO(VAO&&) noexcept = default;
        VAO& operator=(const VAO&) = default;

        void bind(void) const;
        inline GLuint get_id(void) const {
            return id;
        }
    };

    class VBO {
        GLuint id;
    public:
        VBO();
        ~VBO();
        VBO(const VBO&) = default;
        VBO(VBO&&) noexcept = default;
        VBO& operator=(const VBO&) = default;

        void bind(GLenum target = GL_ARRAY_BUFFER) const;
        inline GLuint get_id(void) const {
            return id;
        }
    };
};
#endif

namespace Eng3D {
    enum class MeshMode {
#ifdef E3D_BACKEND_OPENGL
        TRIANGLE_FAN = GL_TRIANGLE_FAN,
        TRIANGLE_STRIP = GL_TRIANGLE_STRIP,
        TRIANGLES = GL_TRIANGLES,
        LINES = GL_LINES,
#elif defined E3D_BACKEND_RGX
        TRIANGLE_FAN = 1,
        TRIANGLE_STRIP = 2,
        TRIANGLES = 3,
        LINES = 4,
#endif
    };

    template<typename V = glm::vec3, typename T = glm::vec2>
    class MeshData {
    public:
        MeshData(void) {};
        MeshData(V _vert, T _tex): vert(_vert), tex(_tex) {};
        ~MeshData() {};
        MeshData(const MeshData&) = default;
        MeshData(MeshData&&) noexcept = default;
        MeshData& operator=(const MeshData&) = default;

        V vert;
        T tex;
    };

    // Packed model - packs both vertices and texcoords into the same buffer
    template<typename V = glm::vec3, typename T = glm::vec2>
    class Mesh {
    public:
        Mesh(enum Eng3D::MeshMode _mode) : mode(_mode) {};
        virtual ~Mesh() {};
        Mesh(const Mesh&) = default;
        Mesh(Mesh&&) noexcept = default;
        Mesh& operator=(const Mesh&) = default;

#ifdef E3D_BACKEND_OPENGL
        virtual void draw(void) const {
            vao.bind();
            glDrawArrays(static_cast<GLenum>(mode), 0, buffer.size());
        };
#else
#   error not implemented
#endif

#ifdef E3D_BACKEND_OPENGL
        virtual void upload(void) const {
            if(buffer.empty()) {
                return;
            }

            vao.bind();
            vbo.bind(GL_ARRAY_BUFFER);
            glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(buffer[0]), &buffer[0], GL_STATIC_DRAW);

            // Vertices
            glVertexAttribPointer(0, V::length(), GL_FLOAT, GL_FALSE, sizeof(buffer[0]), (void*)0);
            glEnableVertexAttribArray(0);

            // Texcoords
            constexpr int tex_stride = ((int)V::length() * (int)sizeof(float));
            glVertexAttribPointer(1, T::length(), GL_FLOAT, GL_FALSE, sizeof(buffer[0]), (void*)((uintptr_t)tex_stride));
            glEnableVertexAttribArray(1);
        };
#else
#   error not implemented
#endif

        std::vector<Eng3D::MeshData<V, T>> buffer;
        enum Eng3D::MeshMode mode;

#ifdef E3D_BACKEND_OPENGL
        // The initialization should be done in this order, first the VAO
        // then initialize the VBO!
        Eng3D::OpenGL::VAO vao;
        Eng3D::OpenGL::VBO vbo;
#else
#   error not implemented
#endif
    };
};
