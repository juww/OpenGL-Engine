#version 430 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in geom_data {
	mat4 model;
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
} data_in[];

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out mat3 TBN;

void main() {

	mat3 normalMatrix = mat3(transpose(inverse(data_in[0].model)));
	vec4 e1 = gl_in[1].gl_Position - gl_in[0].gl_Position;
	vec4 e2 = gl_in[2].gl_Position - gl_in[0].gl_Position;
	vec2 deltaUV1 = data_in[1].TexCoords - data_in[0].TexCoords;
	vec2 deltaUV2 = data_in[2].TexCoords - data_in[0].TexCoords;
	float f = 1.0 / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

	vec3 tangent;
	vec3 bitangent;

    tangent.x = f * (deltaUV2.y * e1.x - deltaUV1.y * e2.x);
    tangent.y = f * (deltaUV2.y * e1.y - deltaUV1.y * e2.y);
    tangent.z = f * (deltaUV2.y * e1.z - deltaUV1.y * e2.z);

    bitangent.x = f * (-deltaUV2.x * e1.x + deltaUV1.x * e2.x);
    bitangent.y = f * (-deltaUV2.x * e1.y + deltaUV1.x * e2.y);
    bitangent.z = f * (-deltaUV2.x * e1.z + deltaUV1.x * e2.z);

	vec3 T = normalize(normalMatrix * tangent);
	vec3 N = normalize(normalMatrix * tangent);
	vec3 B = normalize(normalMatrix * bitangent);
	mat3 tbn = mat3(T, B, N);

	for(int i = 0; i < 3; i++) {
		gl_Position = gl_in[i].gl_Position;
		FragPos = data_in[i].FragPos;
		Normal = mat3(transpose(inverse(data_in[i].model))) * data_in[i].Normal;
		TexCoords = data_in[i].TexCoords;

		T = normalize(mat3(transpose(inverse(data_in[i].model))) * tangent);
		N = normalize(mat3(transpose(inverse(data_in[i].model))) * data_in[i].Normal);
		T = normalize(T - dot(T, N) * N);
		B = cross(N, T);
		TBN = transpose(mat3(T, B, N));
		EmitVertex();
	}

	EndPrimitive();
}