#pragma once

#define VKB_DEBUG

#include <platform/application.h>

#include <vulkan/vulkan.hpp>

static uint32_t findMemoryType(vk::PhysicalDeviceMemoryProperties const& memoryProperties, uint32_t typeBits, vk::MemoryPropertyFlags requirementsMask)
{
    uint32_t typeIndex = uint32_t(~0);
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
    {
        if ((typeBits & 1) && ((memoryProperties.memoryTypes[i].propertyFlags & requirementsMask) == requirementsMask))
        {
            typeIndex = i;
            break;
        }
        typeBits >>= 1;
    }
    assert(typeIndex != uint32_t(~0));
    return typeIndex;
}

class BufferData
{
public:
    vk::Buffer                 buffer;
    vk::DeviceMemory           deviceMemory;

    static BufferData CreateBufferData(vk::PhysicalDevice const &physicalDevice,
                          vk::Device const         &device,
                          vk::DeviceSize            size,
                          vk::BufferUsageFlags      usage,
                          vk::MemoryPropertyFlags   propertyFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)
    {
        BufferData bufferData;

        vk::BufferCreateInfo bufferInfo{};
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = vk::SharingMode::eExclusive;

        if (device.createBuffer(&bufferInfo, nullptr, &bufferData.buffer) != vk::Result::eSuccess)
        {
            throw std::runtime_error("failed to create vertex buffer!");
        }

        // alloc gpu mem

        vk::MemoryRequirements memRequirements;
        device.getBufferMemoryRequirements(bufferData.buffer, &memRequirements);

        vk::MemoryAllocateInfo allocInfo{};
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(physicalDevice.getMemoryProperties(), memRequirements.memoryTypeBits, propertyFlags);

        if (device.allocateMemory(&allocInfo, nullptr, &bufferData.deviceMemory) != vk::Result::eSuccess)
        {
            throw std::runtime_error("failed to allocate buffer memory!");
        }

        device.bindBufferMemory(bufferData.buffer, bufferData.deviceMemory, 0);

        return bufferData;
    }

    void clear(const vk::Device& device)
    {
        if (buffer)
            device.destroyBuffer(buffer);

        if (deviceMemory)
            device.freeMemory(deviceMemory);
    }

    template <typename DataType>
    void upload(const vk::Device& device, std::vector<DataType> const& data)
    {
        size_t size = sizeof(DataType) * data.size();
        void* dataPtr = device.mapMemory(deviceMemory, 0, size);
        memcpy(dataPtr, data.data(), (size_t)size);
        device.unmapMemory(deviceMemory);
    }
};

class LoomApplication : public vkb::Application
{
    struct SwapchainData
    {
        vk::Extent2D                 extent;                           // The swapchain extent
        vk::Format                   format = vk::Format::eUndefined;  // Pixel format of the swapchain.
        vk::SwapchainKHR             swapchain;                        // The swapchain.
        std::vector<vk::ImageView>   image_views;                      // The image view for each swapchain image.
        std::vector<vk::Framebuffer> framebuffers;                     // The framebuffer for each swapchain image view.
    };

    struct FrameData
    {
        vk::Fence         queue_submit_fence;
        vk::CommandPool   primary_command_pool;
        vk::CommandBuffer primary_command_buffer;
        vk::Semaphore     swapchain_acquire_semaphore;
        vk::Semaphore     swapchain_release_semaphore;
    };

   public:
    LoomApplication();
    virtual ~LoomApplication();

   private:
    // from vkb::Application
    virtual bool prepare(const vkb::ApplicationOptions &options) override;
    virtual bool resize(const uint32_t width, const uint32_t height) override;
    virtual void update(float delta_time) override;

    std::pair<vk::Result, uint32_t> acquire_next_image();
    vk::Device                      create_device(const std::vector<const char *> &required_device_extensions);
    vk::Pipeline                    create_graphics_pipeline();
    vk::ImageView                   create_image_view(vk::Image image);
    vk::Instance                    create_instance(std::vector<const char *> const &required_instance_extensions, std::vector<const char *> const &required_validation_layers);
    vk::RenderPass                  create_render_pass();
    vk::ShaderModule                create_shader_module(const char *path);
    vk::SwapchainKHR                create_swapchain(vk::Extent2D const &swapchain_extent, vk::SurfaceFormatKHR surface_format, vk::SwapchainKHR old_swapchain);
    void                            init_framebuffers();
    void                            init_swapchain();
    void                            render(uint32_t swapchain_index);
    void                            select_physical_device_and_surface();
    void                            teardown_framebuffers();
    void                            teardown_per_frame(FrameData &per_frame_data);

   private:
    vk::Instance               instance;               // The Vulkan instance.
    vk::PhysicalDevice         gpu;                    // The Vulkan physical device.
    vk::Device                 device;                 // The Vulkan device.
    vk::Queue                  queue;                  // The Vulkan device queue.
    SwapchainData              swapchain_data;         // The swapchain state.
    vk::SurfaceKHR             surface;                // The surface we will render to.
    uint32_t                   graphics_queue_index;   // The queue family index where graphics work will be submitted.
    vk::RenderPass             render_pass;            // The renderpass description.
    vk::PipelineLayout         pipeline_layout;        // The pipeline layout for resources.
    vk::Pipeline               pipeline;               // The graphics pipeline.
    BufferData                 mVertexBuffer;
    BufferData                 mIndexBuffer;
    vk::DebugUtilsMessengerEXT debug_utils_messenger;  // The debug utils messenger.
    std::vector<vk::Semaphore> recycled_semaphores;    // A set of semaphores that can be reused.
    std::vector<FrameData>     per_frame_data;         // A set of per-frame data.

#if defined(VKB_DEBUG) || defined(VKB_VALIDATION_LAYERS)
    vk::DebugUtilsMessengerCreateInfoEXT debug_utils_create_info;
#endif
};

// #include <hpp_api_vulkan_sample.h>
// class LoomApplication2 : public HPPApiVulkanSample
//{
//
// };

std::unique_ptr<vkb::Application> create_loom_app();
