// TODO use c++20 module
// import vulkan_hpp;

#include <iostream>

// TODO use c++20 designated initializers
// #define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

// vulkan-hpp utils
// #include "math.hpp"
#include "shaders.hpp"
#include "utils.hpp"

static std::string AppName = "Peak";
static std::string EngineName = "Vulkan.hpp";

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    auto framebufferResized = reinterpret_cast<bool*>(glfwGetWindowUserPointer(window));
    *framebufferResized = true;
}

int main(int /*argc*/, char** /*argv*/)
{
    try
    {
        vk::raii::Context context;
        vk::raii::Instance instance = vk::raii::su::makeInstance(context, AppName, EngineName, {}, vk::su::getInstanceExtensions());
#if !defined(NDEBUG)
        vk::raii::DebugUtilsMessengerEXT debugUtilsMessenger(instance, vk::su::makeDebugUtilsMessengerCreateInfoEXT());
#endif
        vk::raii::PhysicalDevice physicalDevice = vk::raii::PhysicalDevices(instance).front();

        vk::raii::su::SurfaceData surfaceData(instance, AppName, vk::Extent2D(800, 600));

        bool framebufferResized = false;
        glfwSetWindowUserPointer(surfaceData.window.handle, &framebufferResized);
        glfwSetFramebufferSizeCallback(surfaceData.window.handle, framebufferResizeCallback);

        std::pair<uint32_t, uint32_t> graphicsAndPresentQueueFamilyIndex =
            vk::raii::su::findGraphicsAndPresentQueueFamilyIndex(physicalDevice, surfaceData.surface);
        vk::raii::Device device = vk::raii::su::makeDevice(physicalDevice, graphicsAndPresentQueueFamilyIndex.first, vk::su::getDeviceExtensions());

        vk::raii::Queue graphicsQueue(device, graphicsAndPresentQueueFamilyIndex.first, 0);
        vk::raii::Queue presentQueue(device, graphicsAndPresentQueueFamilyIndex.second, 0);

        const int FRAME_OVERLAP = 2;
        struct FrameData
        {
            vk::raii::CommandPool commandPool{nullptr};
            vk::raii::CommandBuffer commandBuffer{nullptr};
            vk::raii::Semaphore swapchainSemaphore{nullptr};
            vk::raii::Semaphore renderSemaphore{nullptr};
            vk::raii::Fence renderFence{nullptr};
        };
        std::vector<FrameData> frameDatas(2);
        for (int i = 0; i < FRAME_OVERLAP; ++i)
        {
            frameDatas[i].commandPool = vk::raii::CommandPool(device, {{vk::CommandPoolCreateFlagBits::eResetCommandBuffer}, graphicsAndPresentQueueFamilyIndex.first});
            frameDatas[i].commandBuffer = vk::raii::su::makeCommandBuffer(device, frameDatas[i].commandPool);
            frameDatas[i].swapchainSemaphore = vk::raii::Semaphore(device, vk::SemaphoreCreateInfo());
            frameDatas[i].renderSemaphore = vk::raii::Semaphore(device, vk::SemaphoreCreateInfo());
            frameDatas[i].renderFence = vk::raii::Fence(device, vk::FenceCreateInfo());
        }

        vk::Format colorFormat = vk::su::pickSurfaceFormat(physicalDevice.getSurfaceFormatsKHR(*surfaceData.surface)).format;
        vk::raii::RenderPass renderPass = vk::raii::su::makeRenderPass(device, colorFormat, vk::Format::eUndefined);

        std::shared_ptr<vk::raii::su::SwapChainData> swapChainData;
        std::vector<vk::raii::Framebuffer> framebuffers;
        auto recreateSwapChain = [&]()
        {
            // handling minimization
            int width = 0, height = 0;
            glfwGetFramebufferSize(surfaceData.window.handle, &width, &height);
            while (width == 0 || height == 0)
            {
                glfwGetFramebufferSize(surfaceData.window.handle, &width, &height);
                glfwWaitEvents();
            }

            device.waitIdle();

            framebuffers.clear();
            swapChainData.reset();

            surfaceData.extent.setWidth(width);
            surfaceData.extent.setHeight(height);

            swapChainData = std::make_shared<vk::raii::su::SwapChainData>(physicalDevice,
                                                                          device,
                                                                          surfaceData.surface,
                                                                          surfaceData.extent,
                                                                          vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc,
                                                                          nullptr,
                                                                          graphicsAndPresentQueueFamilyIndex.first,
                                                                          graphicsAndPresentQueueFamilyIndex.second);

            framebuffers =
                vk::raii::su::makeFramebuffers(device, renderPass, swapChainData->imageViews, nullptr, surfaceData.extent);
        };

        recreateSwapChain();

        int frame = 0;
        while (!glfwWindowShouldClose(surfaceData.window.handle))
        {
            processInput(surfaceData.window.handle);

            auto&& frameData = frameDatas[frame];

            device.waitForFences({*frameData.renderFence}, VK_TRUE, vk::su::FenceTimeout);

            vk::Result result;
            uint32_t imageIndex;
            std::tie(result, imageIndex) = swapChainData->swapChain.acquireNextImage(vk::su::FenceTimeout, *frameData.swapchainSemaphore);
            if (result == vk::Result::eErrorOutOfDateKHR)
            {
                recreateSwapChain();
                continue;
            }
            else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
            {
                throw std::runtime_error("failed to acquire swap chain image!");
            }
            assert(imageIndex < swapChainData->images.size());

            device.resetFences({ *frameData.renderFence });

            {
                auto&& commandBuffer = frameData.commandBuffer;
                commandBuffer.begin({});

                std::array<vk::ClearValue, 1> clearValues;
                clearValues[0].color = vk::ClearColorValue(0.2f, 0.3f, 0.3f, 1.0f);
                vk::RenderPassBeginInfo renderPassBeginInfo(*renderPass, *framebuffers[imageIndex], vk::Rect2D(vk::Offset2D(0, 0), surfaceData.extent), clearValues);
                commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

                // draw

                commandBuffer.endRenderPass();

                commandBuffer.end();

                vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
                vk::SubmitInfo submitInfo(*frameData.swapchainSemaphore, waitDestinationStageMask, *commandBuffer);
                graphicsQueue.submit(submitInfo, *frameData.renderFence);
            }

            if (framebufferResized)
            {
                framebufferResized = false;
                recreateSwapChain();
            }
            else
            {
                vk::PresentInfoKHR presentInfoKHR(nullptr, *swapChainData->swapChain, imageIndex);
                result = presentQueue.presentKHR(presentInfoKHR);
                if (result == vk::Result::eSuboptimalKHR || result == vk::Result::eErrorOutOfDateKHR || framebufferResized)
                {
                    framebufferResized = false;
                    recreateSwapChain();
                }
                else if (result != vk::Result::eSuccess)
                {
                    throw std::runtime_error("failed to present swap chain image!");
                }
            }

            frame = (frame + 1) % FRAME_OVERLAP;

            glfwPollEvents();
        }

        device.waitIdle();
    }
    catch (vk::SystemError& err)
    {
        std::cout << "vk::SystemError: " << err.what() << std::endl;
        exit(-1);
    }
    catch (std::exception& err)
    {
        std::cout << "std::exception: " << err.what() << std::endl;
        exit(-1);
    }
    catch (...)
    {
        std::cout << "unknown error\n";
        exit(-1);
    }

    return 0;
}
