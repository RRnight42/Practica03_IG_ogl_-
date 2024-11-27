#version 420 core

layout (location=0) in vec3 inPos;

uniform mat4 modelViewProj;

uniform vec3 lpos;


void main()
{
	
	gl_Position =  modelViewProj * vec4 (inPos * 0.3 + lpos,1.0);
}
