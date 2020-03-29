#ifndef NAUTILUS_VULKAN_SHELL_HPP
#define NAUTILUS_VULKAN_SHELL_HPP

#include "NautilusShell.hpp"
#include "NautilusNS.hpp"
#include "NautilusAssert.hpp"
#include "NautilusVulkanQueueFamily.hpp"
#include "NautilusVulkanSwapchainDetails.hpp"

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <algorithm>
#include <string.h>
#include <set>

class NautilusVulkanShell : 
    public NautilusShell {
    using NautilusShell::NautilusShell;
public:    

    /**
     * Gets executed when the shell gets attached to the core
     */
    virtual void onAttach(void);

    /**
     * Gets executed at the specified frequency by the application loop
     * Computes rendering operations
     */ 
    virtual void onRender(void);

    /**
     * Sets the default window hints for the corresponding API
     * @return Returns a NautilusStatus status code
     */ 
    virtual NautilusStatus setDefaultWindowHints(void);

    /**
     * Initializes the Vulkan API
     * @return Returns a NautilusStatus status code
     */ 
    NautilusStatus initAPI(void);

protected:

    VkSurfaceKHR                    m_surface               = VK_NULL_HANDLE;
    VkPhysicalDevice                m_physicalDevice        = VK_NULL_HANDLE;
    VkDevice                        m_logicalDevice         = VK_NULL_HANDLE;
    VkQueue                         m_graphicsQueue         = VK_NULL_HANDLE;
    VkCommandPool                   m_graphicsCommandPool   = VK_NULL_HANDLE;
    VkFence                         m_graphicsFence         = VK_NULL_HANDLE;
    std::mutex                      m_graphicsLock;
    VkQueue                         m_presentQueue          = VK_NULL_HANDLE;
    VkCommandPool                   m_presentCommandPool    = VK_NULL_HANDLE;
    VkFence                         m_presentFence          = VK_NULL_HANDLE;
    std::mutex                      m_presentLock;
    VkQueue                         m_transferQueue         = VK_NULL_HANDLE;
    VkCommandPool                   m_transferCommandPool   = VK_NULL_HANDLE;
    VkFence                         m_transferFence         = VK_NULL_HANDLE;
    std::mutex                      m_transferLock;
    std::vector< VkImage >          m_swapchainImages;
    VkFormat                        m_swapchainImageFormat;
    VkExtent2D                      m_swapchainImageExtent;
    VkSwapchainKHR                  m_swapchain             = VK_NULL_HANDLE;
    std::vector< VkImageView >      m_swapchainImageViews;
    std::vector< VkFramebuffer >    m_swapchainFramebuffers;
    std::vector< VkSemaphore >      m_swapchainImageAvailableSemaphores;
    std::vector< VkSemaphore >      m_renderingCompletedSemaphores;
    std::vector< VkFence >          m_inFlightFences;
    uint32_t                        m_maxInFlightFrames     = 3;

private:

    /**
     * Creates a GLFWsurface for the GLFWwindow
     * @return Returns a NautilusStatus status code
     */ 
    NautilusStatus createSurfaceGLFW(void);

    /**
     * Selects the most suitable physical device for computation
     * @return Returns a NautilusStatus status code
     */ 
    NautilusStatus selectBestPhysicalDevice(void);

    /**
     * Evaluates how useful a physical device is
     * @param _device The physical device to rate
     * @return Returns an unsigned integer, the higher the better
     */ 
    uint32_t evaluateDeviceSuitabilityScore(VkPhysicalDevice _device);

    /**
     * Prints information about a physical GPU
     * @param _device The physical device
     * @return Returns a NautilusStatus status code
     */ 
    NautilusStatus printPhysicalDevicePropertiesAndFeatures(VkPhysicalDevice _device);

    /**
     * Checks whether a physical device has support for the swapchain extension
     * @param _device The physical device to check
     * @return Returns true if it has support, false otherwise
     */ 
    bool checkDeviceSwapchainExtensionSupport(VkPhysicalDevice _device);

    /**
     * Enumerates the maximum Vulkan multisampling sample count
     * @return Returns the maximum Vulkan multisampling sample count in Vulkan VkSampleCountFlagBits format
     */ 
    VkSampleCountFlagBits enumerateMaximumMultisamplingSampleCount(void);

    /**
     * Finds suitable queue family indices on a physical device
     * @param _device The physical device to check
     * @return Returns a NautilusVulkanQueueFamily structure containing all necessary indices
     */ 
    NautilusVulkanQueueFamily findSuitableQueueFamily(VkPhysicalDevice _device);

    /**
     * Enumerates a physical devices swapchain details
     * @param _device The physical device to check
     * @return Returns a NautilusVulkanSwapchainDetails structure containing all necessary Vulkan information
     */ 
    NautilusVulkanSwapchainDetails querySwapchainDetails(VkPhysicalDevice _device);

    /**
     * Creates a logical device from the selected physical vulkan device
     * @return Returns a NautilusStatus status code
     */ 
    NautilusStatus createLogicalDevice(void);

    /**
     * Creates the required swapchain
     * @return Returns a NautilusStatus status code
     */ 
    NautilusStatus createSwapchain(void);

    /**
     * Evaluates the best format for the swapchain surface
     * @param _availableFormats An array of available formats
     * @return Returns the VkSurfaceFormatKHR that is best suited for the swapchain
     */ 
    VkSurfaceFormatKHR evaluateBestSwapchainSurfaceFormat(const std::vector< VkSurfaceFormatKHR >& _availableFormats);

    /**
     * Evaluates the best swapchain surface presentation mode
     * @param _availablePresentModes An array of available presentation modes
     * @return Returns the VkPresentModeKHR that is best suited for the swapchain
     */ 
    VkPresentModeKHR evaluateBestSwapchainSurfacePresentMode(const std::vector< VkPresentModeKHR >& _availablePresentModes);

    /**
     * Evaluates the best swapchain extent
     * @param _capabilities The swapchains capabilities
     * @return Returns a VkExtent2D structure containing width and height of the swapchain
     */ 
    VkExtent2D evaluateSwapchainExtent(const VkSurfaceCapabilitiesKHR& _capabilities);

    /**
     * Creates the swapchain image views for the swapchain
     * @return Returns a NautilusStatus status code
     */ 
    NautilusStatus createSwapchainImageViews(void);

    /**
     * Creates a view for a Vulkan image
     * @param _image The VkImage to create a VkImageView handle from
     * @param _format The image format
     * @param _aspectFlags The aspect mask to specify in the view creation process
     * @param _mipLevels The amount of mip levels
     */ 
    VkImageView createImageView(
        VkImage                 _image, 
        VkFormat                _format, 
        VkImageAspectFlags      _aspectFlags,
        uint32_t                _mipLevels);

    /**
     * Initializes synchronization objects for multiple queue accesses
     * @return Returns a NautilusStatus status code
     */ 
    NautilusStatus initializeSynchronizationObjects(void);

    /**
     * Allocates the required command pools
     * @return Returns a NautilusStatus status code
     */ 
    NautilusStatus allocateCommandPools(void);

    /**
     * Allocates the actual command buffers
     * @return Returns a NautilusStatus status code
     */ 
    NautilusStatus allocateCommandBuffers(void);

};

#endif      // NAUTILUS_VULKAN_SHELL_HPP