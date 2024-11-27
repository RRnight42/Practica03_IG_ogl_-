#version 420 core

layout (location=0) in vec3 inPos;
layout (location=1) in vec3 inColor;
layout (location=2) in vec3 inNormal;
layout (location=3) in vec2 inTexCoord;
layout (location=4) in vec3 inTangent;

uniform mat4 modelViewProj;
uniform mat4 modelView;
uniform mat4 normal;

out vec3 color;
out vec3 pos;
out vec3 norm;
out vec3 tang;
out vec2 texCoord;
out mat3 matrixTBN;
out vec2 tc;

mat3 TBN(vec3 tangente, vec3 normal)
{
   
    vec3 T = normalize(tangente);
    vec3 N = normalize(normal);
    vec3 B = normalize(cross(N, T));

    return mat3(T, B, N);
}


void main()
{
	color = inColor;
	texCoord = inTexCoord;
	norm = (normal * vec4(inNormal, 0.0)).xyz;
    tang = (modelView * vec4(inTangent, 0.0)).xyz;
	pos = (modelView * vec4(inPos, 1.0)).xyz;
    tc = inTexCoord;
    matrixTBN = TBN( tang , norm);
	gl_Position =  modelViewProj * vec4 (inPos,1.0);
}
