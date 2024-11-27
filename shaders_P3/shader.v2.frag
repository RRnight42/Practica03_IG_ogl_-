#version 420 core

out vec4 outColor;

in vec3 color;
in vec3 pos;
in vec3 norm;
in vec2 texCoord;

layout (binding = 1) uniform sampler2D emiTex;

vec3 Ke;

//Propiedades de la luz

uniform vec3 Id;
uniform vec3 lpos;


void main()
{
    Ke = texture(emiTex, texCoord).rgb;
    outColor = vec4(Id, 1) * vec4(1.0, 1.0, 0.0, 1.0);
    //outColor = vec4(Ke, 1);
}


