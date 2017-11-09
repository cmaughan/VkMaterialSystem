#pragma once
#include "vkh.h"

struct MeshRenderData
{
	vkh::VkhMesh vkMesh;
};

struct GlobalMaterialInputs
{
	VkDeviceMemory* mem;
	VkBuffer* data;
	char* mapped;
	uint32_t* dataLayout;
};

struct MaterialRenderData
{
	vkh::VkhMaterial vkMat;
	
	//every odd entry is a hashed string name, every even entry is a value
	uint32_t* pushConstantLayout;
	uint32_t pushConstantSize;
	char* pushConstantData;
	VkShaderStageFlags pushConstantStages;

	VkDeviceMemory* staticMems;
	VkBuffer* staticBuffers;

	VkDeviceMemory staticUniformMem;
};

struct TextureRenderData
{
	vkh::VkhTexture vulkanTexture;
	VkImageView view;
	VkFormat format;
	VkSampler sampler;
};

struct VertexRenderData
{
	VkVertexInputAttributeDescription* attrDescriptions;
	uint32_t attrCount;
};