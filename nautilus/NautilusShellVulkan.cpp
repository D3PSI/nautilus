#ifndef NAUTILUS_SHELL_VULKAN_CPP
#define NAUTILUS_SHELL_VULKAN_CPP

#include "NautilusShellVulkan.hpp"
#include "NautilusCore.hpp"

namespace nautilus {

    NautilusShellVulkan::NautilusShellVulkan() {
        m_api = NAUTILUS_API_VULKAN;
        m_core.m_allocator = NautilusCore::vulkanAllocator();
    }

    void NautilusShellVulkan::onAttach() {
    }

    void NautilusShellVulkan::onRender() {
    }

    NautilusStatus NautilusShellVulkan::render() {
        printStats();
        showNextSwapchainImage();
        return NAUTILUS_STATUS_OK;
    }

    NautilusStatus NautilusShellVulkan::setAPIWindowHints() {
        m_title = defaults::CONTEXT_NAME_VULKAN;
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        return NAUTILUS_STATUS_OK;
    }

    NautilusStatus NautilusShellVulkan::initAPI() {
        if(m_initializedAPI) return NAUTILUS_STATUS_OK;
        logger::log("Initializing Vulkan...");
        ASSERT_NAUTILUS(NautilusCore::createVulkanInstance());
        ASSERT_NAUTILUS(NautilusCore::createVulkanDebugMessenger());
        logger::log("Successfully initialized Vulkan");
        m_core.m_instance = NautilusCore::vulkanInstance();
        ASSERT_NAUTILUS(createSurfaceGLFW());
        ASSERT_NAUTILUS(selectBestPhysicalDevice());
        ASSERT_NAUTILUS(createLogicalDevice());
        ASSERT_NAUTILUS(createSwapchain());
        ASSERT_NAUTILUS(createSwapchainImageViews());
        ASSERT_NAUTILUS(initializeSynchronizationObjects());
        ASSERT_NAUTILUS(allocateCommandPools());
        ASSERT_NAUTILUS(createRenderPasses());
        ASSERT_NAUTILUS(allocateSwapchainFramebuffers());
        ASSERT_NAUTILUS(allocateCommandBuffers());
        glfwMakeContextCurrent(m_window);
        glfwShowWindow(m_window);
        glfwFocusWindow(m_window);
        m_initializedAPI = true;
        return NAUTILUS_STATUS_OK;
    }

    NautilusStatus NautilusShellVulkan::createSurfaceGLFW() {
        logger::log("Creating Vulkan surface...");
        ASSERT_VULKAN(glfwCreateWindowSurface(
            m_core.m_instance, 
            m_window, 
            m_core.m_allocator, 
            &m_core.m_surface));
        logger::log("Successfully created Vulkan surface");
        return NAUTILUS_STATUS_OK;
    }

    NautilusStatus NautilusShellVulkan::selectBestPhysicalDevice() {
        logger::log("Enumerating GPUs...");
        uint32_t physicalDeviceCount = 0;
        vkEnumeratePhysicalDevices(m_core.m_instance, &physicalDeviceCount, nullptr);
        if(physicalDeviceCount == 0) {
            logger::log("Failed to find a suitable GPU", NAUTILUS_STATUS_FATAL);
            return NAUTILUS_STATUS_FATAL;
        }
        std::vector< VkPhysicalDevice > physicalDevices(physicalDeviceCount);
        vkEnumeratePhysicalDevices(m_core.m_instance, &physicalDeviceCount, physicalDevices.data());
        std::multimap< int, VkPhysicalDevice > possibleGPUs;
        for(const auto& physicalDev : physicalDevices) {
            printPhysicalDevicePropertiesAndFeatures(physicalDev);
            int rating = evaluateDeviceSuitabilityScore(physicalDev);
            possibleGPUs.insert(std::make_pair(rating, physicalDev));
        }
        if(possibleGPUs.rbegin()->first > 0) {        // Is the first possibility even suitable?
            logger::log("Suitable GPU found: ");
            printPhysicalDevicePropertiesAndFeatures(possibleGPUs.rbegin()->second);
            m_core.m_physicalDevice = possibleGPUs.rbegin()->second;
            m_maxVulkanMSAASampleCount = enumerateMaximumMultisamplingSampleCount();
            logger::log("Enumerated maximum multisampling count: " + m_maxVulkanMSAASampleCount);
            return NAUTILUS_STATUS_OK;
        } else {
            logger::log("Failed to find a suitable GPU that supports Vulkan", NAUTILUS_STATUS_FATAL);
            return NAUTILUS_STATUS_FATAL;
        }

    }

    uint32_t NautilusShellVulkan::evaluateDeviceSuitabilityScore(VkPhysicalDevice _device) {
        NautilusQueueFamilyVulkan       family                = NautilusCore::findSuitableVulkanQueueFamily(_device, m_core.m_surface);
        NautilusSwapchainDetailsVulkan  swapchainDetails      = querySwapchainDetails(_device);
        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(_device, &physicalDeviceProperties);
        VkPhysicalDeviceFeatures physicalDeviceFeatures;
        vkGetPhysicalDeviceFeatures(_device, &physicalDeviceFeatures);
        int score = 0;
        if(physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            score += 1000;
        if(!family.isComplete() 
            || !checkDeviceSwapchainExtensionSupport(_device) 
            || swapchainDetails.m_supportedFormats.empty() 
            || swapchainDetails.m_presentationModes.empty()
            || !physicalDeviceFeatures.samplerAnisotropy)       // absolutely necessary features needed to run application on that GPU
            return 0;
        return score;
    }

    NautilusStatus NautilusShellVulkan::printPhysicalDevicePropertiesAndFeatures(VkPhysicalDevice _device) {
        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(_device, &physicalDeviceProperties);
        VkPhysicalDeviceFeatures physicalDeviceFeatures;
        vkGetPhysicalDeviceFeatures(_device, &physicalDeviceFeatures);
        // TODO: Print info
        std::string info = std::string("GPU information:    ") + physicalDeviceProperties.deviceName;
        logger::log(info);
        return NAUTILUS_STATUS_OK;
    }

    bool NautilusShellVulkan::checkDeviceSwapchainExtensionSupport(VkPhysicalDevice _device) {
        logger::log("Checking, whether the device supports the necessary extensions...");
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(
            _device,
            nullptr,
            &extensionCount,
            nullptr);
        std::vector< VkExtensionProperties > availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(
            _device,
            nullptr,
            &extensionCount, 
            availableExtensions.data());
        // Check whether all required extensions are amongst the available ones
        std::set< std::string > extensions(NautilusCore::vulkanRequiredExtensions().begin(), NautilusCore::vulkanRequiredExtensions().end());
        for(const auto& extension : availableExtensions)
            extensions.erase(extension.extensionName);
        if(extensions.empty())
            logger::log("Device supports necessary extensions");
        else
            logger::log("Device does not support necessary extensions", NAUTILUS_STATUS_FATAL);
        return extensions.empty();
    }

    VkSampleCountFlagBits NautilusShellVulkan::enumerateMaximumMultisamplingSampleCount() {
        VkPhysicalDeviceProperties physicalDeviceProps;
        vkGetPhysicalDeviceProperties(m_core.m_physicalDevice, &physicalDeviceProps);
        VkSampleCountFlags counts = std::min(physicalDeviceProps.limits.framebufferColorSampleCounts, physicalDeviceProps.limits.framebufferDepthSampleCounts);
        if(counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
        if(counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
        if(counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
        if(counts & VK_SAMPLE_COUNT_8_BIT)  { return VK_SAMPLE_COUNT_8_BIT;  }
        if(counts & VK_SAMPLE_COUNT_4_BIT)  { return VK_SAMPLE_COUNT_4_BIT;  }
        if(counts & VK_SAMPLE_COUNT_2_BIT)  { return VK_SAMPLE_COUNT_2_BIT;  }
        return VK_SAMPLE_COUNT_1_BIT;
    }

    NautilusStatus NautilusShellVulkan::createLogicalDevice() {
        logger::log("Creating logical device...");
        NautilusQueueFamilyVulkan family = NautilusCore::findSuitableVulkanQueueFamily(m_core.m_physicalDevice, m_core.m_surface);
        std::vector< VkDeviceQueueCreateInfo > deviceQueueCreateInfos;
        std::set< uint32_t > uniqueQueueFamilies = { family.m_graphicsFamilyIndex.value(), family.m_presentationFamilyIndex.value(), family.m_transferFamilyIndex.value() };
        float queuePriority = 1.0f;
        for(uint32_t qF : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo deviceQueueCreateInfo          = {};
            deviceQueueCreateInfo.sType                            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            deviceQueueCreateInfo.queueFamilyIndex                 = qF;
            deviceQueueCreateInfo.queueCount                       = 1;
            deviceQueueCreateInfo.pQueuePriorities                 = &queuePriority;
            deviceQueueCreateInfos.push_back(deviceQueueCreateInfo);
        }
        VkPhysicalDeviceFeatures physicalDeviceFeatures            = {};
        physicalDeviceFeatures.samplerAnisotropy                   = VK_TRUE;
        physicalDeviceFeatures.fillModeNonSolid                    = VK_TRUE;
        VkDeviceCreateInfo deviceCreateInfo                        = {};
        deviceCreateInfo.sType                                     = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.queueCreateInfoCount                      = static_cast< uint32_t >(deviceQueueCreateInfos.size());
        deviceCreateInfo.pQueueCreateInfos                         = deviceQueueCreateInfos.data();
        deviceCreateInfo.enabledExtensionCount                     = static_cast< uint32_t >(NautilusCore::vulkanRequiredExtensions().size());
        deviceCreateInfo.ppEnabledExtensionNames                   = NautilusCore::vulkanRequiredExtensions().data();
        deviceCreateInfo.pEnabledFeatures                          = &physicalDeviceFeatures;
        if(NautilusCore::enableVulkanValidationLayers()) {
            deviceCreateInfo.enabledLayerCount                     = static_cast< uint32_t >(NautilusCore::vulkanValidationLayers().size());
            deviceCreateInfo.ppEnabledLayerNames                   = NautilusCore::vulkanValidationLayers().data();
        } else
            deviceCreateInfo.enabledLayerCount                     = 0;
        ASSERT_VULKAN(vkCreateDevice(
            m_core.m_physicalDevice, 
            &deviceCreateInfo, 
            m_core.m_allocator, 
            &m_core.m_logicalDevice));
        logger::log("Successfully created logical device");
        logger::log("Retrieving queue handle for graphics queue...");
        vkGetDeviceQueue(
            m_core.m_logicalDevice,
            family.m_graphicsFamilyIndex.value(),
            0,
            &m_graphicsQueue);
        logger::log("Successfully retrieved queue handle for graphics queue");
        logger::log("Retrieving queue handle for presentation queue...");
        vkGetDeviceQueue(
            m_core.m_logicalDevice,
            family.m_presentationFamilyIndex.value(),
            0,
            &m_presentQueue);
        logger::log("Successfully retrieved queue handle for presentation queue");
        logger::log("Retrieving queue handle for transfer queue...");
        vkGetDeviceQueue(
            m_core.m_logicalDevice,
            family.m_transferFamilyIndex.value(),
            0,
            &m_transferQueue);
        logger::log("Successfully retrieved queue handle for transfer queue");
        return NAUTILUS_STATUS_OK;
    }

    NautilusStatus NautilusShellVulkan::createSwapchain() {
        logger::log("Creating swapchain...");
        NautilusSwapchainDetailsVulkan  details             = querySwapchainDetails(m_core.m_physicalDevice);
        VkSurfaceFormatKHR              surfaceFormat       = evaluateBestSwapchainSurfaceFormat(details.m_supportedFormats);
        VkPresentModeKHR                presentMode         = evaluateBestSwapchainSurfacePresentMode(details.m_presentationModes);
        VkExtent2D                      extent              = evaluateSwapchainExtent(details.m_surfaceCapabilities);
        m_core.m_swapchainImageFormat                       = surfaceFormat.format;
        m_core.m_swapchainImageExtent                       = extent;
        uint32_t                        swapchainImageCount = details.m_surfaceCapabilities.minImageCount + 1;
        if(details.m_surfaceCapabilities.maxImageCount > 0 && swapchainImageCount > details.m_surfaceCapabilities.maxImageCount)
            swapchainImageCount                             = details.m_surfaceCapabilities.maxImageCount;
        VkSwapchainCreateInfoKHR        swapchainCreateInfo = {};
        swapchainCreateInfo.sType                           = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchainCreateInfo.surface                         = m_core.m_surface;
        swapchainCreateInfo.minImageCount                   = swapchainImageCount;
        swapchainCreateInfo.imageFormat                     = surfaceFormat.format;
        swapchainCreateInfo.imageColorSpace                 = surfaceFormat.colorSpace;
        swapchainCreateInfo.imageExtent                     = extent;
        swapchainCreateInfo.imageArrayLayers                = 1;                                          // Amount of layers in an image, always 1, unless doing stereoscopic stuff
        swapchainCreateInfo.imageUsage                      = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;        // Render directly to swapchain
        NautilusQueueFamilyVulkan family                    = NautilusCore::findSuitableVulkanQueueFamily(m_core.m_physicalDevice, m_core.m_surface);
        uint32_t queueFamilyIndices[]                       = { family.m_graphicsFamilyIndex.value(), family.m_presentationFamilyIndex.value() };
        if(family.m_graphicsFamilyIndex != family.m_presentationFamilyIndex) {           // If presentation queue and graphics queue are in the same queue family, exclusive ownership is not necessary
            swapchainCreateInfo.imageSharingMode            = VK_SHARING_MODE_CONCURRENT;
            swapchainCreateInfo.queueFamilyIndexCount       = 2;
            swapchainCreateInfo.pQueueFamilyIndices         = queueFamilyIndices;
        }
        else  // else it is
            swapchainCreateInfo.imageSharingMode            = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCreateInfo.preTransform                    = details.m_surfaceCapabilities.currentTransform;        // Specify current image transform as no transform is necessary
        swapchainCreateInfo.compositeAlpha                  = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;                   // Window should not be blended with other windows behind it, no thanks
        swapchainCreateInfo.clipped                         = true;                                                // Do not render pixels that are outside the clip space as this is just performance loss
        swapchainCreateInfo.presentMode                     = presentMode;
        swapchainCreateInfo.oldSwapchain                    = VK_NULL_HANDLE;
        ASSERT_VULKAN(vkCreateSwapchainKHR(
            m_core.m_logicalDevice,
            &swapchainCreateInfo,
            m_core.m_allocator,
            &m_core.m_swapchain));
        logger::log("Successfully created swapchain");
        logger::log("Retrieving the handles for the swapchain images...");
        vkGetSwapchainImagesKHR(
            m_core.m_logicalDevice, 
            m_core.m_swapchain,
            &swapchainImageCount,
            nullptr);
        m_swapchainImages.resize(swapchainImageCount);
        vkGetSwapchainImagesKHR(
            m_core.m_logicalDevice,
            m_core.m_swapchain,
            &swapchainImageCount,
            m_swapchainImages.data());
        logger::log("Successfully retrieved the handles for the swapchain images");
        return NAUTILUS_STATUS_OK;
    }

    VkSurfaceFormatKHR NautilusShellVulkan::evaluateBestSwapchainSurfaceFormat(const std::vector< VkSurfaceFormatKHR >& _availableFormats) {
        if(_availableFormats.size() == 1 && _availableFormats[0].format == VK_FORMAT_UNDEFINED)     // If Vulkan only returns one entry and its format is set to VK_FORMAT_UNDEFINED it means that Vulkan has no preferred format so we can choose freely
            return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
        for(const auto& format : _availableFormats)
            if(format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)  
                return format;
        return _availableFormats[0];        // If there are no better choices, just return the first one in the array, usually it is good enough
    }

    VkPresentModeKHR NautilusShellVulkan::evaluateBestSwapchainSurfacePresentMode(const std::vector< VkPresentModeKHR >& _availablePresentModes) {
        for(const auto& presMode : _availablePresentModes)
            if(presMode == VK_PRESENT_MODE_MAILBOX_KHR) // VK_PRESENT_MODE_MAILBOX_KHR MAY cause tearing issues in borderless window or fullscreen mode
                return presMode;
        return VK_PRESENT_MODE_FIFO_KHR;        // If no better mode is available, return VK_PRESENT_MODE_FIFO_KHR as its implied to be supported if the GPU support Vulkan
    }

    VkExtent2D NautilusShellVulkan::evaluateSwapchainExtent(const VkSurfaceCapabilitiesKHR& _capabilities) {
        if(_capabilities.currentExtent.width != std::numeric_limits< uint32_t >::max())
            return _capabilities.currentExtent;
        else {
            int width, height;
            glfwGetFramebufferSize(m_window, &width, &height);
            VkExtent2D preferredExtent = {
                static_cast< uint32_t >(width),
                static_cast< uint32_t >(height)
            };
            preferredExtent.width             = std::clamp(preferredExtent.width, _capabilities.minImageExtent.width, _capabilities.maxImageExtent.width);
            preferredExtent.height            = std::clamp(preferredExtent.height, _capabilities.minImageExtent.height, _capabilities.maxImageExtent.height);
            return preferredExtent;
        }
    }

    NautilusStatus NautilusShellVulkan::createSwapchainImageViews() {
        logger::log("Creating swapchain image views...");
        m_swapchainImageViews.resize(m_swapchainImages.size());
        for(size_t i = 0; i < m_swapchainImages.size(); i++)     
            m_swapchainImageViews[i] = createImageView(
                m_swapchainImages[i], 
                m_core.m_swapchainImageFormat, 
                VK_IMAGE_ASPECT_COLOR_BIT,
                1);
        logger::log("Successfully created swapchain image views");
        return NAUTILUS_STATUS_OK;
    }

    VkImageView NautilusShellVulkan::createImageView(
        VkImage                 _image, 
        VkFormat                _format, 
        VkImageAspectFlags      _aspectFlags,
        uint32_t                _mipLevels) {
        logger::log("Creating image view...");
        VkImageViewCreateInfo imageViewCreateInfo               = {};
        imageViewCreateInfo.sType                               = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image                               = _image;
        imageViewCreateInfo.viewType                            = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format                              = _format;
        imageViewCreateInfo.subresourceRange.aspectMask         = _aspectFlags;
        imageViewCreateInfo.subresourceRange.baseMipLevel       = 0;
        imageViewCreateInfo.subresourceRange.levelCount         = _mipLevels;
        imageViewCreateInfo.subresourceRange.baseArrayLayer     = 0;
        imageViewCreateInfo.subresourceRange.layerCount         = 1;
        imageViewCreateInfo.components.r                        = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g                        = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b                        = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a                        = VK_COMPONENT_SWIZZLE_IDENTITY;
        VkImageView imgView;
        VkResult result = vkCreateImageView(
            m_core.m_logicalDevice,
            &imageViewCreateInfo,
            m_core.m_allocator,
            &imgView);
        logger::log("Successfully created image view");
        return imgView;
    }

    NautilusStatus NautilusShellVulkan::initializeSynchronizationObjects() {
        logger::log("Initializing sync-objects...");
        m_swapchainImageAvailableSemaphores.resize(m_maxInFlightFrames);
        m_renderingCompletedSemaphores.resize(m_maxInFlightFrames);
        m_inFlightFences.resize(m_maxInFlightFrames);
        VkSemaphoreCreateInfo semaphoreCreateInfo        = {};
        semaphoreCreateInfo.sType                        = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VkFenceCreateInfo fenceCreateInfo                = {};
        fenceCreateInfo.sType                            = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags                            = VK_FENCE_CREATE_SIGNALED_BIT;
        for(size_t i = 0; i < m_maxInFlightFrames; i++) {
            ASSERT_VULKAN(vkCreateSemaphore(
                m_core.m_logicalDevice,
                &semaphoreCreateInfo,
                m_core.m_allocator,
                &m_swapchainImageAvailableSemaphores[i]));
            logger::log("Successfully initialized semaphore");
            ASSERT_VULKAN(vkCreateSemaphore(
                m_core.m_logicalDevice,
                &semaphoreCreateInfo,
                m_core.m_allocator,
                &m_renderingCompletedSemaphores[i]));
            logger::log("Successfully initialized semaphore");
            ASSERT_VULKAN(vkCreateFence(
                m_core.m_logicalDevice,
                &fenceCreateInfo,
                m_core.m_allocator,
                &m_inFlightFences[i]));
            logger::log("Successfully initialized fence");
        }
        ASSERT_VULKAN(vkCreateFence(
            m_core.m_logicalDevice,
            &fenceCreateInfo,
            m_core.m_allocator,
            &m_graphicsFence));
        logger::log("Successfully initialized graphics fence");
        ASSERT_VULKAN(vkCreateFence(
            m_core.m_logicalDevice,
            &fenceCreateInfo,
            m_core.m_allocator,
            &m_transferFence));
        logger::log("Successfully initialized transfer fence");
        logger::log("Successfully initialized sync-objects");
        return NAUTILUS_STATUS_OK;
    }

    NautilusStatus NautilusShellVulkan::allocateCommandPools() {
        logger::log("Allocating command pool...");
        NautilusQueueFamilyVulkan family = NautilusCore::findSuitableVulkanQueueFamily(m_core.m_physicalDevice, m_core.m_surface);
        VkCommandPoolCreateInfo commandPoolCreateInfo          = {};
        commandPoolCreateInfo.sType                            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolCreateInfo.queueFamilyIndex                 = family.m_graphicsFamilyIndex.value();
        std::unique_lock< std::mutex > graphicsLock(m_graphicsLock);
        ASSERT_VULKAN(vkCreateCommandPool(
            m_core.m_logicalDevice,
            &commandPoolCreateInfo,
            m_core.m_allocator,
            &m_graphicsCommandPool));
        graphicsLock.unlock();
        logger::log("Successfully allocated command pool");
        logger::log("Allocating command pool...");
        commandPoolCreateInfo                                  = {};
        commandPoolCreateInfo.sType                            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolCreateInfo.queueFamilyIndex                 = family.m_transferFamilyIndex.value();
        std::unique_lock< std::mutex > transferLock(m_transferLock);
        ASSERT_VULKAN(vkCreateCommandPool(
            m_core.m_logicalDevice,
            &commandPoolCreateInfo,
            m_core.m_allocator,
            &m_transferCommandPool));
        transferLock.unlock();
        logger::log("Successfully allocated command pool");
        return NAUTILUS_STATUS_OK;
    }

    NautilusStatus NautilusShellVulkan::createRenderPasses() {
        logger::log("Creating render passes...");
        VkAttachmentDescription colorAttachmentDescription          = {};
        colorAttachmentDescription.format                           = m_core.m_swapchainImageFormat;
        colorAttachmentDescription.samples                          = VK_SAMPLE_COUNT_1_BIT; 
        colorAttachmentDescription.loadOp                           = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachmentDescription.storeOp                          = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentDescription.stencilLoadOp                    = VK_ATTACHMENT_LOAD_OP_DONT_CARE;                  // No stencil buffering, so nobody cares about stencil operations
        colorAttachmentDescription.stencilStoreOp                   = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachmentDescription.initialLayout                    = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachmentDescription.finalLayout                      = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;         // Render the image to the offscreen render-target
        colorAttachmentDescription.finalLayout                      = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        VkAttachmentReference colorAttachmentReference              = {};
        colorAttachmentReference.attachment                         = 0; 
        colorAttachmentReference.layout                             = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        /*VkAttachmentDescription depthBufferAttachmentDescription    = {};
        depthBufferAttachmentDescription.format                     = depthBuffer->imgFormat;
        depthBufferAttachmentDescription.samples                    = VK_SAMPLE_COUNT_1_BIT;
        depthBufferAttachmentDescription.loadOp                     = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthBufferAttachmentDescription.storeOp                    = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthBufferAttachmentDescription.stencilLoadOp              = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthBufferAttachmentDescription.stencilStoreOp             = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthBufferAttachmentDescription.initialLayout              = VK_IMAGE_LAYOUT_UNDEFINED;
        depthBufferAttachmentDescription.finalLayout                = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;*/
        VkAttachmentReference depthBufferAttachmentReference        = {};
        depthBufferAttachmentReference.attachment                   = 1;
        depthBufferAttachmentReference.layout                       = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        VkAttachmentDescription colorAttachmentResolve              = {};
        colorAttachmentResolve.format                               = m_core.m_swapchainImageFormat;
        colorAttachmentResolve.samples                              = VK_SAMPLE_COUNT_1_BIT;     // Sample multisampled color attachment down to present to screen
        colorAttachmentResolve.loadOp                               = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentResolve.storeOp                              = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentResolve.stencilLoadOp                        = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentResolve.stencilStoreOp                       = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachmentResolve.initialLayout                        = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachmentResolve.finalLayout                          = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        VkAttachmentReference colorAttachmentResolveRef             = {};
        colorAttachmentResolveRef.attachment                        = 2;
        colorAttachmentResolveRef.layout                            = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        VkSubpassDescription subpassDescription                     = {};
        subpassDescription.pipelineBindPoint                        = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.colorAttachmentCount                     = 1;
        subpassDescription.pColorAttachments                        = &colorAttachmentReference;
        //subpassDescription.pDepthStencilAttachment                  = &depthBufferAttachmentReference;
        //subpassDescription.pResolveAttachments                      = &colorAttachmentResolveRef;
        std::vector< VkAttachmentDescription > attachments          = { 
            colorAttachmentDescription
            //depthBufferAttachmentDescription, 
        };
        VkRenderPassCreateInfo renderPassCreateInfo                 = {};
        renderPassCreateInfo.sType                                  = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount                        = static_cast< uint32_t >(attachments.size());
        renderPassCreateInfo.pAttachments                           = attachments.data();
        renderPassCreateInfo.subpassCount                           = 1;
        renderPassCreateInfo.pSubpasses                             = &subpassDescription;
        VkSubpassDependency subpassDependency                       = {};
        subpassDependency.srcSubpass                                = VK_SUBPASS_EXTERNAL;
        subpassDependency.dstSubpass                                = 0;
        subpassDependency.srcStageMask                              = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependency.srcAccessMask                             = 0;
        subpassDependency.dstStageMask                              = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependency.dstAccessMask                             = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT
                                                                    | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        renderPassCreateInfo.dependencyCount                        = 1;
        renderPassCreateInfo.pDependencies                          = &subpassDependency;
        ASSERT_VULKAN(vkCreateRenderPass(
            m_core.m_logicalDevice,
            &renderPassCreateInfo, 
            m_core.m_allocator, 
            &m_renderPass));
        logger::log("Successfully created render passes");
        return NAUTILUS_STATUS_OK;
    }

    NautilusStatus NautilusShellVulkan::allocateSwapchainFramebuffers() {
        logger::log("Allocating framebuffers...");
        m_swapchainFramebuffers.resize(m_swapchainImageViews.size());
        for (size_t i = 0; i < m_swapchainImageViews.size(); i++) {
            logger::log("Creating framebuffer...");
            std::vector< VkImageView > attachments = {
                m_swapchainImageViews[i],
            };
            VkFramebufferCreateInfo framebufferCreateInfo          = {};
            framebufferCreateInfo.sType                            = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferCreateInfo.renderPass                       = m_renderPass;
            framebufferCreateInfo.attachmentCount                  = static_cast< uint32_t >(attachments.size());
            framebufferCreateInfo.pAttachments                     = attachments.data();
            framebufferCreateInfo.width                            = m_core.m_swapchainImageExtent.width;
            framebufferCreateInfo.height                           = m_core.m_swapchainImageExtent.height;
            framebufferCreateInfo.layers                           = 1;
            ASSERT_VULKAN(vkCreateFramebuffer(
                m_core.m_logicalDevice,
                &framebufferCreateInfo,
                m_core.m_allocator,
                &m_swapchainFramebuffers[i]));
            logger::log("Successfully allocated framebuffer");
        }
        logger::log("Successfully allocated framebuffers");
        return NAUTILUS_STATUS_OK;
    }

    NautilusStatus NautilusShellVulkan::allocateCommandBuffers() {
        logger::log("Allocating command buffers...");
        m_commandBuffers.resize(m_swapchainFramebuffers.size());        // For every frame in the swapchain, create a command buffer
        std::scoped_lock< std::mutex > graphicsLock(m_graphicsLock);
        VkCommandBufferAllocateInfo commandBufferAllocateInfo          = {};
        commandBufferAllocateInfo.sType                                = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocateInfo.commandPool                          = m_graphicsCommandPool;
        commandBufferAllocateInfo.level                                = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocateInfo.commandBufferCount                   = static_cast< uint32_t >(m_commandBuffers.size());
        ASSERT_VULKAN(vkAllocateCommandBuffers(
            m_core.m_logicalDevice,
            &commandBufferAllocateInfo,
            m_commandBuffers.data()));
        logger::log("Successfully allocated command buffers");
        logger::log("Recording the first batch of command buffers...");
        recordSwapchainCommandBuffers();
        logger::log("Successfully recorded the first batch of command buffers");
        return NAUTILUS_STATUS_OK;
    }

    NautilusStatus NautilusShellVulkan::recordSwapchainCommandBuffers() {
        for(uint32_t i = 0; i < m_swapchainFramebuffers.size(); i++) {
            VkCommandBuffer cmdBuf = m_commandBuffers[i];
            VkCommandBufferBeginInfo commandBufferBeginInfo            = {};
            commandBufferBeginInfo.sType                               = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            commandBufferBeginInfo.flags                               = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
            ASSERT_VULKAN(vkBeginCommandBuffer(cmdBuf, &commandBufferBeginInfo));
            VkRenderPassBeginInfo renderPassBeginInfo                  = {};
            renderPassBeginInfo.sType                                  = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassBeginInfo.renderPass                             = m_renderPass;
            renderPassBeginInfo.framebuffer                            = m_swapchainFramebuffers[i];
            renderPassBeginInfo.renderArea.offset                      = {0, 0};
            renderPassBeginInfo.renderArea.extent                      = m_core.m_swapchainImageExtent;     
            std::array< VkClearValue, 2 > clearColorValues             = {}; 
            clearColorValues[0].color                                  = {1.0f, 0.0f, 0.0f, 1.0f};
            clearColorValues[1].depthStencil                           = {1.0f, 0};
            renderPassBeginInfo.clearValueCount                        = static_cast< uint32_t >(clearColorValues.size());
            renderPassBeginInfo.pClearValues                           = clearColorValues.data();
            vkCmdBeginRenderPass(cmdBuf, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);        // Rendering commands will be embedded in the primary command buffer
            onRender();
            vkCmdEndRenderPass(cmdBuf);
            ASSERT_VULKAN(vkEndCommandBuffer(cmdBuf));
        }
        return NAUTILUS_STATUS_OK;
    }

    NautilusStatus NautilusShellVulkan::showNextSwapchainImage() {
        vkWaitForFences(
            m_core.m_logicalDevice,
            1,
            &m_inFlightFences[m_currentSwapchainImage],
            VK_TRUE,
            std::numeric_limits< uint64_t >::max());
        vkResetFences(m_core.m_logicalDevice, 1, &m_inFlightFences[m_currentSwapchainImage]);
        uint32_t swapchainImageIndex;
        VkResult result = vkAcquireNextImageKHR(
            m_core.m_logicalDevice,
            m_core.m_swapchain,
            std::numeric_limits< uint64_t >::max(),                                     // numeric limit of 64-bit unsigned interger disables timeout
            m_swapchainImageAvailableSemaphores[m_currentSwapchainImage],   // signal this semaphore once operation is complete
            VK_NULL_HANDLE,
            &swapchainImageIndex);
        if(result == VK_ERROR_OUT_OF_DATE_KHR) {   
            recreateSwapchain();
            return NAUTILUS_STATUS_SWAPCHAIN_RECREATED;
        }
        ASSERT_VULKAN(result);
        VkSubmitInfo submitInfo                            = {};
        submitInfo.sType                                   = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        std::unique_lock< std::mutex > lock(m_graphicsLock);
        VkSemaphore waitSemaphores[]                       = {m_swapchainImageAvailableSemaphores[m_currentSwapchainImage]};
        VkPipelineStageFlags waitStages[]                  = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount                      = 1;
        submitInfo.pWaitSemaphores                         = waitSemaphores;
        submitInfo.pWaitDstStageMask                       = waitStages;
        submitInfo.commandBufferCount                      = 1;
        submitInfo.pCommandBuffers                         = &m_commandBuffers[swapchainImageIndex];
        lock.unlock();
        VkSemaphore signalSemaphores[]                     = {m_renderingCompletedSemaphores[m_currentSwapchainImage]};
        submitInfo.signalSemaphoreCount                    = 1;
        submitInfo.pSignalSemaphores                       = signalSemaphores;
        ASSERT_VULKAN(vkQueueSubmit(
            m_graphicsQueue,
            1,
            &submitInfo,
            m_inFlightFences[m_currentSwapchainImage]));
        VkPresentInfoKHR presentationInfo                  = {};
        presentationInfo.sType                             = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentationInfo.waitSemaphoreCount                = 1;
        presentationInfo.pWaitSemaphores                   = signalSemaphores;
        VkSwapchainKHR swapchains[]                        = {m_core.m_swapchain};
        presentationInfo.swapchainCount                    = 1;
        presentationInfo.pSwapchains                       = swapchains;
        presentationInfo.pImageIndices                     = &swapchainImageIndex;
        result = vkQueuePresentKHR(m_presentQueue, &presentationInfo);
        if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_hasFramebufferBeenResized) {
            m_hasFramebufferBeenResized = false;
            recreateSwapchain();
            return NAUTILUS_STATUS_SWAPCHAIN_RECREATED;
        }
        ASSERT_VULKAN(result);
        m_currentSwapchainImage = (m_currentSwapchainImage + 1) % m_maxInFlightFrames;
        return NAUTILUS_STATUS_OK;
    }

    NautilusStatus NautilusShellVulkan::recreateSwapchain() {
        vkDeviceWaitIdle(m_core.m_logicalDevice);
        std::unique_lock< std::mutex > commandLock(m_commandBufferLock);
        int width = 0;
        int height = 0;
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(m_window, &width, &height);
            glfwWaitEvents();
        }
        commandLock.unlock();
        ASSERT_NAUTILUS(cleanSwapchain());
        ASSERT_NAUTILUS(createSwapchain());
        ASSERT_NAUTILUS(createSwapchainImageViews());
        ASSERT_NAUTILUS(initializeSynchronizationObjects());
        ASSERT_NAUTILUS(createRenderPasses());
        ASSERT_NAUTILUS(allocateSwapchainFramebuffers());
        ASSERT_NAUTILUS(allocateCommandBuffers()); 
        return NAUTILUS_STATUS_OK;
    }

    NautilusStatus NautilusShellVulkan::clean() {
        vkDeviceWaitIdle(m_core.m_logicalDevice);
        ASSERT_NAUTILUS(cleanSwapchain());
        logger::log("Successfully destroyed camera");
        std::unique_lock< std::mutex > transferLock(m_transferLock);
        vkDestroyCommandPool(m_core.m_logicalDevice, m_transferCommandPool, m_core.m_allocator);
        transferLock.unlock();
        logger::log("Successfully destroyed command pool");
        std::unique_lock< std::mutex > graphicsLock(m_graphicsLock);
        vkDestroyCommandPool(m_core.m_logicalDevice, m_graphicsCommandPool, m_core.m_allocator);
        graphicsLock.unlock();
        logger::log("Successfully destroyed command pool");
        vkDestroyDevice(m_core.m_logicalDevice, m_core.m_allocator);
        logger::log("Successfully destroyed device");
        vkDestroySurfaceKHR(m_core.m_instance, m_core.m_surface, m_core.m_allocator);
        logger::log("Successfully destroyed surface");
        return NAUTILUS_STATUS_OK;
    }

    NautilusStatus NautilusShellVulkan::cleanSwapchain() {
        logger::log("Cleaning swapchain...");
        logger::log("Destroying framebuffers...");
        for (auto framebuffer : m_swapchainFramebuffers) {
            vkDestroyFramebuffer(m_core.m_logicalDevice, framebuffer, m_core.m_allocator);
            logger::log("Successfully destroyed framebuffer");
        }
        logger::log("Successfully destroyed framebuffers");
        std::unique_lock< std::mutex > lock(m_graphicsLock);
        vkFreeCommandBuffers(
            m_core.m_logicalDevice,
            m_graphicsCommandPool,
            static_cast< uint32_t >(m_commandBuffers.size()),
            m_commandBuffers.data());
        lock.unlock();
        logger::log("Successfully freed command buffers");
        vkDestroyRenderPass(m_core.m_logicalDevice, m_renderPass, m_core.m_allocator);
        logger::log("Successfully destroyed render pass");
        for (auto imageView : m_swapchainImageViews) {
            vkDestroyImageView(m_core.m_logicalDevice, imageView, m_core.m_allocator);
            logger::log("Successfully destroyed image view");
        }
        vkDestroySwapchainKHR(m_core.m_logicalDevice, m_core.m_swapchain, m_core.m_allocator);
        logger::log("Successfully destroyed swapchain");
        for (size_t i = 0; i < m_maxInFlightFrames; i++) {
            vkDestroySemaphore(m_core.m_logicalDevice, m_renderingCompletedSemaphores[i], m_core.m_allocator);
            vkDestroySemaphore(m_core.m_logicalDevice, m_swapchainImageAvailableSemaphores[i], m_core.m_allocator);
            vkDestroyFence(m_core.m_logicalDevice, m_inFlightFences[i], m_core.m_allocator);
        }
        vkDestroyFence(m_core.m_logicalDevice, m_graphicsFence, m_core.m_allocator);
        vkDestroyFence(m_core.m_logicalDevice, m_transferFence, m_core.m_allocator);
        logger::log("Successfully destroyed sync-objects");
        logger::log("Successfully cleaned swapchain");
        return NAUTILUS_STATUS_OK;
    }

    void NautilusShellVulkan::resize(GLFWwindow* _window, int _w, int _h) {
        m_hasFramebufferBeenResized = true;
        onResize(_window, _h, _w);
    }

    NautilusStatus NautilusShellVulkan::updateShellViewport(const NautilusViewport& _viewport) {
        return NAUTILUS_STATUS_OK;
    }

    NautilusSwapchainDetailsVulkan NautilusShellVulkan::querySwapchainDetails(VkPhysicalDevice _device) {
        logger::log("Gathering swapchain details...");
        NautilusSwapchainDetailsVulkan details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_device, m_core.m_surface, &details.m_surfaceCapabilities);
        uint32_t numFormats;
        vkGetPhysicalDeviceSurfaceFormatsKHR(
            _device, 
            m_core.m_surface, 
            &numFormats, 
            nullptr);
        if(numFormats != 0) {
            details.m_supportedFormats.resize(numFormats);
            vkGetPhysicalDeviceSurfaceFormatsKHR(
                _device,
                m_core.m_surface,
                &numFormats,
                details.m_supportedFormats.data());
        }
        uint32_t numPresentModes;
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            _device,
            m_core.m_surface,
            &numPresentModes,
            nullptr);
        if(numPresentModes != 0) {     
            details.m_presentationModes.resize(numPresentModes);
            vkGetPhysicalDeviceSurfacePresentModesKHR(
                _device,
                m_core.m_surface,
                &numPresentModes,
                details.m_presentationModes.data());
        }
        logger::log("Successfully gathered swapchain details");
        return details;
    }

}

#endif      // NAUTILUS_SHELL_VULKAN_CPP