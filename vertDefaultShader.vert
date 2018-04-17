//shadertype=glsl
#version 330 core
layout(location = 0) in vec3 aPos;
//layout (location = 1) in vec3 aColor; 
 
uniform vec3 customPos;
uniform mat4 transform;
out vec3 ourColor; 
out vec3 centerPos;

void main()
{
    gl_Position = transform * vec4(aPos, 1.0);
	centerPos = customPos;
    ourColor = abs(aPos); 
}  