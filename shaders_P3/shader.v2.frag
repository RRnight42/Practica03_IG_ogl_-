#version 420 core

out vec4 outColor;
uniform vec3 Id;



void main()
{
   
    outColor = vec4(Id,1);
   
}


