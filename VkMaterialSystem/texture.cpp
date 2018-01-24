#include "stdafx.h"

#include "texture.h"
#include "asset_rdata_types.h"
#include <map>

#define STB_IMAGE_IMPLEMENTATION
#include <stb\stb_image.h>

struct TextureAsset
{
	TextureRenderData rData;
	uint32_t width;
	uint32_t height;
	uint32_t numChannels;
};

struct TextureStorage
{
	std::vector<TextureAsset> data;
	std::vector<uint32_t> deletedIndices;
};

TextureStorage texStorage;


namespace Texture
{
	TextureRenderData* getRenderData(uint32_t texId)
	{
		return &texStorage.data[texId].rData;
	}

	uint32_t nextKey()
	{
		if (texStorage.deletedIndices.size() == 0)
		{
			return texStorage.data.size();
		}

		uint32_t id = texStorage.deletedIndices.back();
		texStorage.deletedIndices.pop_back();
		return id;
	}

	void loadDefaultTexture()
	{
		uint32_t id = nextKey();
		checkf(id == 0, "Trying to load default texture after other texture assets have already been loaded");

		const char* default_file = "../data/textures/chckerbw2.png";
		make(default_file);
	}

	uint32_t make(const char* filepath)
	{
		uint32_t newId = nextKey();

		TextureAsset t;

		int texWidth, texHeight, texChannels;

		//STBI_rgb_alpha forces an alpha even if the image doesn't have one
		stbi_uc* pixels = stbi_load(filepath, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	
		checkf(pixels, "Could not load image");

		VkDeviceSize imageSize = texWidth * texHeight * 4;

		VkBuffer stagingBuffer;
		vkh::Allocation stagingBufferMemory;

		vkh::createBuffer(stagingBuffer,
			stagingBufferMemory,
			imageSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		void* data;
		vkMapMemory(vkh::GContext.device, stagingBufferMemory.handle, stagingBufferMemory.offset, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(vkh::GContext.device, stagingBufferMemory.handle);

		stbi_image_free(pixels);

		t.width = texWidth;
		t.height = texHeight;
		t.numChannels = texChannels;
		t.rData.format = VK_FORMAT_R8G8B8A8_UNORM;

		//VK image format must match buffer
		vkh::createImage(t.rData.image,
			t.width, t.height,
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

		vkh::allocBindImageToMem(t.rData.deviceMemory,
			t.rData.image,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		vkh::transitionImageLayout(t.rData.image, t.rData.format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		vkh::copyBufferToImage(stagingBuffer, t.rData.image, t.width, t.height);

		vkh::transitionImageLayout(t.rData.image, t.rData.format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		vkh::createImageView(t.rData.view, 1, t.rData.image, t.rData.format);
		t.rData.samplerIdx = 2;

		vkDestroyBuffer(vkh::GContext.device, stagingBuffer, nullptr);
		vkh::freeDeviceMemory(stagingBufferMemory);

		texStorage.data.push_back(t); // (std::pair<uint32_t, TextureAsset>(newId, t));
		return newId;
	}


	void destroy(uint32_t texId)
	{

	}

}