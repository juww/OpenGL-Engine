#version 430 core

layout (points) in;
layout (line_strip, max_vertices = 2) out;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
} vs_in[];

uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;

void main()
{
    gl_Position = projection * gl_in[0].gl_Position;
    FragPos = vs_in[0].FragPos;
    Normal = vs_in[0].Normal;
    EmitVertex();
    gl_Position = projection * (gl_in[0].gl_Position + 0.5f * vec4(vs_in[0].Normal, 0.0f));
    FragPos = vs_in[0].FragPos;
    Normal = vs_in[0].Normal;
    EmitVertex();
    EndPrimitive();

}