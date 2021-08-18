#version 330 compatibility

layout (location = 0) in vec3 m_pos;
layout (location = 1) in vec2 m_texcoord;
layout (location = 2) in vec3 m_colour;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;

out vec2 v_texcoord;
out vec3 v_colour;

void main() {
	gl_Position = ((projection * view) * model) * vec4(m_pos, 1.0);
	v_texcoord = m_texcoord;
	v_colour = m_colour;
}
