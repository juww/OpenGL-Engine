#version 430 core

layout (triangles) in;
layout (line_strip, max_vertices = 2) out;

in geom_data {
	vec3 Normal;
	vec2 texCoord;
	mat4 projection;
} data_in[];

void main()
{
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();
    gl_Position = (gl_in[0].gl_Position + 0.21f * vec4(data_in[0].Normal, 0.0f));
    EmitVertex();
    EndPrimitive();

    gl_Position = gl_in[1].gl_Position;
    EmitVertex();
    gl_Position = (gl_in[1].gl_Position + 0.21f * vec4(data_in[1].Normal, 0.0f));
    EmitVertex();
    EndPrimitive();

    gl_Position = gl_in[2].gl_Position;
    EmitVertex();
    gl_Position = (gl_in[2].gl_Position + 0.21f * vec4(data_in[2].Normal, 0.0f));
    EmitVertex();
    EndPrimitive();
}