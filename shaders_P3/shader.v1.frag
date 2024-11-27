#version 420 core

out vec4 outColor;

in vec3 color;
in vec3 pos;
in vec3 norm;
in vec2 texCoord;
in mat3 matrixTBN;

layout (binding = 0) uniform sampler2D colorTex;
layout (binding = 1) uniform sampler2D emiTex;
layout (binding = 2) uniform sampler2D bumpMapping;

//Propiedades del objeto
vec3 Ka;
vec3 Kd;
vec3 Ks;
vec3 N;
float alpha = 5000.0;
vec3 Ke;
in vec2 tc;

//Propiedades de la luz
vec3 Ia;
uniform vec3 Id;
vec3 Is;
uniform vec3 lpos;

vec3 shade();

void main()
{
	Ka = texture(colorTex, texCoord).rgb;
	Kd = texture(colorTex, texCoord).rgb;
	Ke = texture(emiTex, texCoord).rgb;
	Ks = vec3(1.0);

    N = normalize(matrixTBN * (texture(bumpMapping, tc).rgb * 2.0 - 1.0));
	
    outColor = vec4(shade(), 1.0);
    //outColor = Kd.rgbb;
}

vec3 shade()
{
	vec3 c = vec3(0.0);
	c = Ia * Ka;

    
	vec3 L = normalize (lpos - pos);
	vec3 diffuse = Id * Kd * dot (L,N);
	c += clamp(diffuse, 0.0, 1.0);
	
	vec3 V = normalize (-pos);
	vec3 R = normalize (reflect (-L,N));
	float factor = max (dot (R,V), 0.01);
	vec3 specular = Is*Ks*pow(factor,alpha);
	c += clamp(specular, 0.0, 1.0);

	c+=Ke;
	
	return c;
}
