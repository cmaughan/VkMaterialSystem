#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform PER_OBJECT 
{ 
	vec4 col; 
}pc;

layout(binding = 0, set = 0) uniform Globals
{
	float time;
	vec4 mouse;
}global;

layout(binding = 0, set = 2) uniform Instance
{
	vec4 tint;
}inst_data;

layout(binding = 3, set = 2) uniform Instance2
{
	vec4 tint2;
}inst_data2;


layout(location = 0) in vec3 vertex;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragUV;

out gl_PerVertex
{
    vec4 gl_Position;
};

//BEGIN FUNCTIONS

void main() 
{
    gl_Position = vec4(vertex, 1.0);
//	float t = sin( mod(global.time, 1.0)) * 2.0;
    fragColor = pc.col * inst_data.tint * inst_data2.tint2;
	fragUV = uv;
}

