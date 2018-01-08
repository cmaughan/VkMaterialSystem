#include "stdafx.h"
#include "rendering.h"
#include "vkh.h"
#include "os_support.h"
#include "os_input.h"
#include "mesh.h"
#include "asset_rdata_types.h"
#include "material.h"

namespace Rendering
{
	using vkh::GContext;

	std::vector<VkFramebuffer>		frameBuffers;
	vkh::VkhRenderBuffer			depthBuffer;
	std::vector<VkCommandBuffer>	commandBuffers;

	void createMainRenderPass();

	void init()
	{
		vkh::createWin32Context(GContext, GAppInfo.curW, GAppInfo.curH, GAppInfo.instance, GAppInfo.wndHdl, APP_NAME);
		vkh::createDepthBuffer(depthBuffer, GAppInfo.curW, GAppInfo.curH, GContext.device, GContext.gpu.device);

		createMainRenderPass();

		vkh::createFrameBuffers(frameBuffers, GContext.swapChain, &depthBuffer.view, GContext.mainRenderPass, GContext.device);

		uint32_t swapChainImageCount = static_cast<uint32_t>(GContext.swapChain.imageViews.size());
		commandBuffers.resize(swapChainImageCount);
		for (uint32_t i = 0; i < swapChainImageCount; ++i)
		{
			vkh::createCommandBuffer(commandBuffers[i], GContext.gfxCommandPool, GContext.device);
		}

	}

	void createMainRenderPass()
	{
		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = GContext.swapChain.imageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentDescription depthAttachment = {};
		depthAttachment.format = vkh::depthFormat(); //common, no stencil though
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		std::vector<VkAttachmentDescription> renderPassAttachments;
		renderPassAttachments.push_back(colorAttachment);

		vkh::createRenderPass(GContext.mainRenderPass, renderPassAttachments, &depthAttachment, GContext.device);

	}


	void draw(MaterialInstance mInst)
	{
		//acquire an image from the swap chain
		uint32_t imageIndex;

		//using uint64 max for timeout disables it
		VkResult res = vkAcquireNextImageKHR(GContext.device, GContext.swapChain.swapChain, UINT64_MAX, GContext.imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

		vkh::waitForFence(GContext.frameFences[imageIndex], GContext.device);
		vkResetFences(GContext.device, 1, &GContext.frameFences[imageIndex]);

		//record drawing
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = nullptr; // Optional
		vkResetCommandBuffer(commandBuffers[imageIndex], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
		res = vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo);


		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = GContext.mainRenderPass;
		renderPassInfo.framebuffer = frameBuffers[imageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = GContext.swapChain.extent;

		std::vector<VkClearValue> clearColors;

		//color
		clearColors.push_back({ 0.0f, 0.0f, 0.0f, 1.0f });

		//depth
		clearColors.push_back({ 1.0f, 1.0f, 1.0f, 1.0f });

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearColors.size());
		renderPassInfo.pClearValues = &clearColors[0];
		vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		//eventually this will have to iterate over multiple objects/materials
		{

			const MeshRenderData& mesh = Mesh::getRenderData();
			const MaterialRenderData& mat = Material::getRenderData(mInst.parent);
			const MaterialInstancePage& page = mat.instPages[mInst.page];

			vkCmdBindPipeline(commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, mat.pipeline);
			VkBuffer vertexBuffers[] = { mesh.vBuffer };
			VkDeviceSize offsets[] = { 0 };

			glm::vec4 mouseData = glm::vec4(0, 0, 0, 0);
			mouseData.x = (float)getMouseX();
			mouseData.y = (float)getMouseY();
			mouseData.z = (float)getMouseLeftButton();
			mouseData.w = (float)getMouseRightButton();

			glm::vec2 resolution = glm::vec2(100, 100);


			Material::setGlobalVector2("resolution", resolution);
			Material::setGlobalVector4("mouse", mouseData);
			Material::setGlobalFloat("time", (float)(os_getMilliseconds() / 1000.0f));

			//Material::setUniformVector4(materialId, "global.mouse", mouseData);
			//Material::setUniformFloat(materialId, "test", 1.0f);
			if (mat.pushConstantLayout.blockSize > 0)
			{
				//push constant data is completely set up for every object 
				Material::setPushConstantVector(mInst.parent, "col", glm::vec4(0.0, 1.0, 1.0, 1.0));
				Material::setPushConstantFloat(mInst.parent, "time", (float)(os_getMilliseconds() / 1000.0f));

				vkCmdPushConstants(
					commandBuffers[imageIndex],
					mat.pipelineLayout,
					mat.pushConstantLayout.visibleStages,
					0,
					mat.pushConstantLayout.blockSize,
					mat.pushConstantData);
			}

			if (mat.numDescSets > 0)
			{
				vkCmdBindDescriptorSets(commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, mat.pipelineLayout, 0, 1, &mat.globalDescSet, 0, 0);
				vkCmdBindDescriptorSets(commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, mat.pipelineLayout, 2, 2, page.descSets, 0, 0);

			}

			vkCmdBindVertexBuffers(commandBuffers[imageIndex], 0, 1, vertexBuffers, offsets);
			vkCmdBindIndexBuffer(commandBuffers[imageIndex], mesh.iBuffer, 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(commandBuffers[imageIndex], static_cast<uint32_t>(mesh.iCount), 1, 0, 0, 0);

		}


		vkCmdEndRenderPass(commandBuffers[imageIndex]);
		res = vkEndCommandBuffer(commandBuffers[imageIndex]);
		assert(res == VK_SUCCESS);


		//submit

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		//wait on writing colours to the buffer until the semaphore says the buffer is available
		VkSemaphore waitSemaphores[] = { GContext.imageAvailableSemaphore };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;

		VkSemaphore signalSemaphores[] = { GContext.renderFinishedSemaphore };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;
		submitInfo.pCommandBuffers = &commandBuffers[imageIndex];
		submitInfo.commandBufferCount = 1;

		res = vkQueueSubmit(GContext.deviceQueues.graphicsQueue, 1, &submitInfo, GContext.frameFences[imageIndex]);
		assert(res == VK_SUCCESS);

		//present

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { GContext.swapChain.swapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr; // Optional
		res = vkQueuePresentKHR(GContext.deviceQueues.transferQueue, &presentInfo);
	}
}