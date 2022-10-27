#version 450

layout(location = 0) in vec2 position;
layout(location = 1) in vec3 color;


layout(push_constant) uniform Push{
	vec2 offest;
	vec3 color;
}push;

void main(){
	gl_Position = vec4(position + push.offest,0.0,1.0);
}