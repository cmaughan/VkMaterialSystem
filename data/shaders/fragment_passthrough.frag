#version 440 core
#extension GL_ARB_separate_shader_objects : require

layout(binding = 0, set = 0) uniform GLOBAL_DATA
{
	float time;
	vec4 mouse;
	vec2 resolution;
	mat4 viewMatrix;
	vec4 worldSpaceCameraPos;
}global;

//arrays of sampler2Ds crash glslangValidator
//layout(set = 1, binding = 0) uniform texture2D gTextures[16];
//layout(set = 2, binding = 0) uniform sampler gSamp[8];

layout(push_constant) uniform PER_OBJECT 
{ 
	vec4 col; 
	//int img1;
	//int img2;
}pc;

layout(binding = 0, set = 3) uniform Instance
{
	vec4 tint;
}inst_data;

layout(binding = 1, set = 2) uniform sampler2D texSampler;
layout(binding = 4, set = 3) uniform sampler2D testSampler;

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragUV;

void main() 
{
	vec4 tex = texture(texSampler, fragUV);
	vec4 tex2 = texture(testSampler, fragUV);
//	vec4 tex2 = texture(sampler2D(gTextures[pc.img1],gSamp[0]), fragUV);
    outColor = mix(tex,tex2,fragColor.g) * global.mouse.z * pc.col * inst_data.tint;
}