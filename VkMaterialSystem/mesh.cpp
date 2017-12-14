#include "stdafx.h"

#include "mesh.h"
#include "vkh.h"
#include <map>
#include <vector>
#include "asset_rdata_types.h"

struct MeshAsset
{
	MeshRenderData rData;
};

struct MeshStore
{
	std::vector<MeshAsset> data;
};

MeshStore meshStorage;

namespace Mesh
{
	uint32_t make(Vertex* vertices, uint32_t vertexCount, uint32_t* indices, uint32_t indexCount)
	{
		using vkh::GContext;

		size_t vBufferSize = sizeof(Vertex) * vertexCount + sizeof(uint32_t) * indexCount;
		size_t iBufferSize = sizeof(uint32_t) * indexCount;
		
		MeshAsset newAsset;
		MeshRenderData& m = newAsset.rData;

		m.iCount = indexCount;
		m.vCount = vertexCount;

		//TODO - find a better way to structure this
		extern vkh::VkhContext GContext;

		vkh::createBuffer(m.vBuffer,
			m.vBufferMemory,
			vBufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			GContext.gpu.device,
			GContext.device
		);

		vkh::createBuffer(m.iBuffer,
			m.iBufferMemory,
			iBufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			GContext.gpu.device,
			GContext.device
		);


		//transfer data to the above buffers

		VkBuffer stagingBuffer;
		vkh::Allocation stagingMemory;

		vkh::createBuffer(stagingBuffer,
			stagingMemory,
			vBufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			GContext.gpu.device,
			GContext.device
		);

		void* data;
		vkMapMemory(GContext.device, stagingMemory.handle, stagingMemory.offset, vBufferSize, 0, &data);
		memcpy(data, vertices, (size_t)vBufferSize);
		vkUnmapMemory(GContext.device, stagingMemory.handle);

		//copy to device local here
		vkh::copyBuffer(stagingBuffer, m.vBuffer, vBufferSize);
		vkh::freeDeviceMemory(stagingMemory);
		vkDestroyBuffer(GContext.device, stagingBuffer, nullptr);

		vkh::createBuffer(stagingBuffer,
			stagingMemory,
			iBufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			GContext.gpu.device,
			GContext.device
		);

		vkMapMemory(GContext.device, stagingMemory.handle, stagingMemory.offset, iBufferSize, 0, &data);
		memcpy(data, indices, (size_t)iBufferSize);
		vkUnmapMemory(GContext.device, stagingMemory.handle);



		vkh::copyBuffer(stagingBuffer, m.iBuffer, iBufferSize);
		vkh::freeDeviceMemory(stagingMemory);
		vkDestroyBuffer(GContext.device, stagingBuffer, nullptr);

		meshStorage.data.push_back(newAsset);
		return static_cast<uint32_t>(meshStorage.data.size()) - 1;
	}
	
	const VertexRenderData* vertexRenderData()
	{
		static VertexRenderData* vkRenderData = nullptr;
		if (!vkRenderData)
		{
			vkRenderData = (VertexRenderData*)malloc(sizeof(VertexRenderData));
			vkRenderData->attrCount = 3;

			vkRenderData->attrDescriptions = (VkVertexInputAttributeDescription*)malloc(sizeof(VkVertexInputAttributeDescription) * vkRenderData->attrCount);
			vkRenderData->attrDescriptions[0] = { 0,0,VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos) };
			vkRenderData->attrDescriptions[1] = { 1,0,VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv) };
			vkRenderData->attrDescriptions[2] = { 2,0,VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, pos) };
		}

		return vkRenderData;
	}

	MeshRenderData getRenderData(uint32_t meshId)
	{
		return meshStorage.data[meshId].rData;
	}

	void destroy()
	{
		assert(0); //unimplemented
	}
}
