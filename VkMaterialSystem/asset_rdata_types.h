#pragma once
#include "vkh.h"

struct MeshRenderData
{
	vkh::VkhMesh vkMesh;
};

struct MaterialRenderData
{
	vkh::VkhMaterial vkMat;
	
	//every odd entry is a hashed string name, every even entry is a value
	uint32_t* pushConstantLayout;
	uint32_t pushConstantSize;
	char* pushConstantData;

	uint32_t* staticBufferLayout;

	VkBuffer staticBuffer;
	VkDeviceMemory staticMem;





	uint32_t* dynamicBufferLayout;
	VkBuffer* dynamicBuffers;
	VkBuffer* stagingBuffers;
	VkDeviceMemory dynamicMem;

	uint32_t* offsets;
	uint32_t numOffsets;
};

struct VertexRenderData
{
	VkVertexInputAttributeDescription* attrDescriptions;
	uint32_t attrCount;
};