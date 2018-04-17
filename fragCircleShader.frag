#version 330 core
uniform vec2 viewDimensions;
uniform float outerRadius;
uniform vec4 customColor;
in vec3 centerPos;
in vec3 ourColor;
out vec4 outColor;

void main(){
 float x = (gl_FragCoord.x - (viewDimensions.x / 2.0f)) / (viewDimensions.x / 2.0f);
 float y = (gl_FragCoord.y - (viewDimensions.y / 2.0f)) / (viewDimensions.y / 2.0f);
 float x_sqr = (centerPos.x - x) * (centerPos.x - x);
 float y_sqr = (centerPos.y - y) * (centerPos.y - y);
 float len = sqrt(x_sqr + y_sqr);
 if(len > outerRadius) {
  discard;
 }
 outColor = vec4(ourColor,1.0f);
}