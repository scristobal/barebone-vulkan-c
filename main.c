#include "helpers.c"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <vulkan/vulkan_core.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cglm/cglm.h>
#include <cglm/mat4.h>
#include <cglm/vec4.h>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

const char *validationLayers[] = {"VK_LAYER_KHRONOS_validation"};
const uint32_t validationLayerCount = 1;

const char *deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
const uint32_t deviceExtensionsCount = 1;

bool checkValidationLayerSupport() {
    uint32_t availableLayerCount;
    vkEnumerateInstanceLayerProperties(&availableLayerCount, NULL);

    VkLayerProperties availableLayers[availableLayerCount];
    vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers);

    // displayAvailableLayers(availableLayerCount, availableLayers);

    for (uint32_t i; i < validationLayerCount; i++) {
        bool layerFound = false;

        for (uint32_t j = 0; j < availableLayerCount; j++) {
            if (strcmp(validationLayers[i], availableLayers[j].layerName) ==
                0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t availableExtensionCount;
    vkEnumerateDeviceExtensionProperties(device, NULL, &availableExtensionCount,
                                         NULL);

    VkExtensionProperties availableExtensions[availableExtensionCount];
    vkEnumerateDeviceExtensionProperties(device, NULL, &availableExtensionCount,
                                         availableExtensions);

    // displayAvailableExtensions(availableExtensionCount, availableExtensions);

    for (uint32_t i = 0; i < deviceExtensionsCount; i++) {
        bool supported = false;

        for (uint32_t j = 0; j < availableExtensionCount; j++) {
            if (strcmp(deviceExtensions[i],
                       availableExtensions[j].extensionName) == 0) {
                supported = true;
                break;
            }
        }

        if (!supported) {
            return false;
        }
    }

    return true;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              VkDebugUtilsMessageTypeFlagsEXT messageType,
              const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
              void *pUserData) {

    fprintf(stderr, "validation layer: %s\n", pCallbackData->pMessage);

    return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger) {

    PFN_vkCreateDebugUtilsMessengerEXT func =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance, "vkCreateDebugUtilsMessengerEXT");

    if (func != NULL) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void populateDebugMessengerCreateInfo(
    VkDebugUtilsMessengerCreateInfoEXT *createInfo) {
    createInfo->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo->messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo->pfnUserCallback = debugCallback;
}

VkDebugUtilsMessengerEXT setupDebugMessenger(VkInstance instance) {
    VkDebugUtilsMessengerEXT debugMessenger;

    VkDebugUtilsMessengerCreateInfoEXT createInfo = {};

    populateDebugMessengerCreateInfo(&createInfo);

    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, NULL,
                                     &debugMessenger) != VK_SUCCESS) {
        fprintf(stderr, "ERROR: failed to set up debug messenger.\n");
        exit(1);
    }

    return debugMessenger;
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                   VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks *pAllocator) {
    PFN_vkDestroyDebugUtilsMessengerEXT func =
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance, "vkDestroyDebugUtilsMessengerEXT");

    if (func != NULL) {
        func(instance, debugMessenger, pAllocator);
    }
}

GLFWwindow *initWindow() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    return glfwCreateWindow(WIDTH, HEIGHT, "Vulkan window", NULL, NULL);
}

const char **getRequiredExtensions(uint32_t *extensionCount) {
    const char **required;

    required = glfwGetRequiredInstanceExtensions(extensionCount);

    if (!enableValidationLayers) {
        return required;
    }

    const char **result = malloc((*extensionCount + 1) * sizeof(const char *));
    // no free called, must outlive instance which is end of program

    if (!result) {
        return required;
    }

    memcpy(result, required, *extensionCount * sizeof(const char *));

    result[*extensionCount] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    ++*extensionCount;

    return result;
}

VkInstance createInstance() {
    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Hello Triangle",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_0,
    };

    // displayInstanceExtensions();

    uint32_t extensionCount;
    const char **extensions = getRequiredExtensions(&extensionCount);

    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = extensionCount,
        .ppEnabledExtensionNames = extensions,
        .enabledLayerCount = 0,
        .pNext = NULL,
    };

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = validationLayerCount;
        createInfo.ppEnabledLayerNames = validationLayers;

        populateDebugMessengerCreateInfo(&debugCreateInfo);

        createInfo.pNext =
            (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
    }

    // displayRequiredInstanceExtensions(extensionCount, extensions);

    VkInstance instance;

    if (vkCreateInstance(&createInfo, NULL, &instance) != VK_SUCCESS) {
        fprintf(stderr, "ERROR: failed to create instance\n");
        exit(1);
    }

    return instance;
};

VkSurfaceKHR createSurface(VkInstance instance, GLFWwindow *window) {
    VkSurfaceKHR surface;

    if (glfwCreateWindowSurface(instance, window, NULL, &surface) !=
        VK_SUCCESS) {
        fprintf(stderr, "ERROR: failed to create window surface.\n");
    }

    return surface;
}

int32_t getGraphicsFamily(VkPhysicalDevice device) {

    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);

    VkQueueFamilyProperties queueFamilies[queueFamilyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                             queueFamilies);

    for (int32_t i = 0; i < queueFamilyCount; i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            return i;
        }
    }

    return -1;
}

int32_t getPresentationFamily(VkPhysicalDevice device, VkSurfaceKHR surface) {
    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);

    VkQueueFamilyProperties queueFamilies[queueFamilyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                             queueFamilies);

    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        VkBool32 presentSupport;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface,
                                             &presentSupport);

        if (presentSupport) {
            return i;
        }
    }

    return -1;
}

bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
    VkPhysicalDeviceProperties deviceProps;
    vkGetPhysicalDeviceProperties(device, &deviceProps);

    if (deviceProps.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        return false;

    if (!checkDeviceExtensionSupport(device)) {
        return false;
    }

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, NULL);

    if (formatCount == 0)
        return false;

    uint32_t presentationCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
                                              &presentationCount, NULL);
    if (presentationCount == 0)
        return false;

    if (getGraphicsFamily(device) == -1)
        return false;

    if (getPresentationFamily(device, surface) == -1)
        return false;

    return true;
}

VkPhysicalDevice pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface) {
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);

    if (deviceCount == 0) {
        fprintf(stderr, "ERROR: failed to find GPUs with Vulkan support.\n");
        exit(1);
    }

    VkPhysicalDevice devices[deviceCount];
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices);

    // displayDevices(devices, deviceCount);

    for (uint32_t i = 0; i < deviceCount; i++) {
        if (isDeviceSuitable(devices[i], surface)) {
            physicalDevice = devices[i];
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE) {
        fprintf(stderr, "ERROR: failed to find a suitable GPU.\n");
        exit(1);
    }

    fprintf(stdout, "using ");
    displayDevice(&physicalDevice);

    return physicalDevice;
}

VkDevice createLogicalDevice(VkPhysicalDevice physicalDevice,
                             VkSurfaceKHR surface) {
    int32_t graphicsIndex = getGraphicsFamily(physicalDevice);
    int32_t presentaionIndex = getPresentationFamily(physicalDevice, surface);

    if (graphicsIndex == -1 || presentaionIndex == -1) {
        fprintf(stderr,
                "ERROR: failed to get graphics and presentation families.\n");
        exit(1);
    }

    uint32_t queueCreateInfoCount;

    if (graphicsIndex == presentaionIndex) {
        queueCreateInfoCount = 1;
    } else {
        queueCreateInfoCount = 2;
    }

    VkDeviceQueueCreateInfo queueCreateInfos[queueCreateInfoCount];

    float queuePriority = 1.0f;

    for (uint32_t i = 0; i < queueCreateInfoCount; i++) {

        VkDeviceQueueCreateInfo queueCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority,

        };

        queueCreateInfos[i] = queueCreateInfo;
    }

    if (queueCreateInfoCount == 2) {
        queueCreateInfos[0].queueFamilyIndex = graphicsIndex;
        queueCreateInfos[1].queueFamilyIndex = presentaionIndex;
    } else {
        queueCreateInfos[0].queueFamilyIndex = graphicsIndex;
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pQueueCreateInfos = queueCreateInfos,
        .queueCreateInfoCount = queueCreateInfoCount,
        .pEnabledFeatures = &deviceFeatures,
        .ppEnabledExtensionNames = deviceExtensions,
        .enabledExtensionCount = 1,
        .enabledLayerCount = 0,
    };

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = validationLayerCount;
        createInfo.ppEnabledLayerNames = validationLayers;
    }

    VkDevice device;

    if (vkCreateDevice(physicalDevice, &createInfo, NULL, &device) !=
        VK_SUCCESS) {
        fprintf(stderr, "ERROR: failed to create logical device.\n");
        exit(1);
    }

    return device;
}

VkQueue getGraphicsQueue(VkDevice device, VkPhysicalDevice physicalDevice) {
    VkQueue graphicsQueue;

    int32_t index = getGraphicsFamily(physicalDevice);

    if (index == -1) {
        fprintf(stderr, "ERROR: failed to get graphics family.\n");
        exit(1);
    }

    vkGetDeviceQueue(device, index, 0, &graphicsQueue);

    return graphicsQueue;
}

VkQueue getPresentationQueue(VkDevice device, VkPhysicalDevice physicalDevice,
                             VkSurfaceKHR surface) {
    VkQueue presentationQueue;
    int32_t index = getPresentationFamily(physicalDevice, surface);

    if (index == -1) {
        fprintf(stderr, "ERROR: failed to get presentation family.\n");
        exit(1);
    }

    vkGetDeviceQueue(device, index, 0, &presentationQueue);

    return presentationQueue;
}

VkSurfaceFormatKHR chooseSurfaceFormat(VkPhysicalDevice device,
                                       VkSurfaceKHR surface) {
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, NULL);

    VkSurfaceFormatKHR formats[formatCount];
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount,
                                         formats);
    for (uint32_t i = 0; i < formatCount; i++) {
        if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
            formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return formats[i];
        }
    }

    return formats[0];
}

VkPresentModeKHR choosePresentMode(VkPhysicalDevice device,
                                   VkSurfaceKHR surface) {

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
                                              &presentModeCount, NULL);
    VkPresentModeKHR presetModes[presentModeCount];
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
                                              &presentModeCount, presetModes);

    for (uint32_t i = 0; i < presentModeCount; i++) {
        if (presetModes[i] == VK_PRESENT_MODE_MAX_ENUM_KHR) {
            return presetModes[i];
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseExtent(VkPhysicalDevice device, VkSurfaceKHR surface,
                        GLFWwindow *window) {
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &capabilities);

    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = {(uint32_t)width, (uint32_t)height};

        if (actualExtent.width < capabilities.minImageExtent.width) {
            actualExtent.width = capabilities.minImageExtent.width;
        }

        if (actualExtent.width > capabilities.maxImageExtent.width) {
            actualExtent.width = capabilities.maxImageExtent.width;
        }

        if (actualExtent.height < capabilities.minImageExtent.height) {
            actualExtent.height = capabilities.minImageExtent.height;
        }

        if (actualExtent.height > capabilities.maxImageExtent.height) {
            actualExtent.height = capabilities.maxImageExtent.height;
        }
        return actualExtent;
    }
}

VkSwapchainKHR createSwapchain(VkDevice device, VkPhysicalDevice physicalDevice,
                               VkSurfaceKHR surface, VkSurfaceFormatKHR format,
                               VkExtent2D extent,
                               VkPresentModeKHR presentMode) {
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface,
                                              &capabilities);

    uint32_t imageCount = capabilities.minImageCount + 1;

    if (capabilities.maxImageCount > 0 &&
        imageCount > capabilities.maxImageCount) {
        imageCount = capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = imageCount,
        .imageFormat = format.format,
        .imageColorSpace = format.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = presentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE,
    };

    uint32_t graphicsFamily = getGraphicsFamily(physicalDevice);
    uint32_t presentFamily = getPresentationFamily(physicalDevice, surface);

    uint32_t queueFamilyIndices[] = {graphicsFamily, presentFamily};

    if (graphicsFamily != presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 1;
        createInfo.pQueueFamilyIndices = NULL;
    }

    VkSwapchainKHR swapchain;

    if (vkCreateSwapchainKHR(device, &createInfo, NULL, &swapchain) !=
        VK_SUCCESS) {
        fprintf(stderr, "ERROR: failed to create Swapchain.\n");
        exit(1);
    }

    return swapchain;
}

void createImageViews(VkDevice device, VkImageView *imageViews, VkImage *images,
                      uint32_t imageCount, VkSurfaceFormatKHR format) {
    for (uint32_t i = 0; i < imageCount; i++) {
        VkImageViewCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = images[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = format.format,
            .components.r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.a = VK_COMPONENT_SWIZZLE_IDENTITY,
            .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .subresourceRange.baseMipLevel = 0,
            .subresourceRange.levelCount = 1,
            .subresourceRange.baseArrayLayer = 0,
            .subresourceRange.layerCount = 1,
        };
        if (vkCreateImageView(device, &createInfo, NULL, &imageViews[i]) !=
            VK_SUCCESS) {
            fprintf(stderr, "ERROR: failed to create image view.\n");
            exit(1);
        }
    }
}

VkShaderModule createShaderModule(VkDevice device, void *code,
                                  size_t codeCount) {
    VkShaderModuleCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = codeCount,
        .pCode = code,
    };

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, NULL, &shaderModule) !=
        VK_SUCCESS) {
        fprintf(stderr, "ERROR: failed to create shader module.\n");
        exit(1);
    }

    return shaderModule;
}

void createGraphicsPipeline(VkDevice device, VkExtent2D extent) {
    size_t vertShaderCodeCount;
    void *vertShaderCode = mmap_file_read("vert.spv", &vertShaderCodeCount);
    if (!vertShaderCode) {
        fprintf(stderr, "ERROR: failed to read SPIR-V vertex shader.\n");
        exit(1);
    }

    VkShaderModule vertShaderModule =
        createShaderModule(device, vertShaderCode, vertShaderCodeCount);

    if (munmap(vertShaderCode, vertShaderCodeCount) == -1) {
        fprintf(stderr, "ERROR: failed to close SPIR-V vertex shader.\n");
    }

    size_t fragShaderCodeCount;
    void *fragShaderCode = mmap_file_read("vert.spv", &fragShaderCodeCount);
    if (!fragShaderCode) {
        fprintf(stderr, "ERROR: failed to read SPIR-V fragment shader.\n");
        exit(1);
    }

    VkShaderModule fragShaderModule =
        createShaderModule(device, fragShaderCode, fragShaderCodeCount);

    if (munmap(fragShaderCode, fragShaderCodeCount) == -1) {
        fprintf(stderr, "ERROR: failed to close SPIR-V fragment shader.\n");
    }

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vertShaderModule,
        .pName = "main",
    };

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = fragShaderModule,
        .pName = "main",
    };

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo,
                                                      fragShaderStageInfo};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 0,
        .pVertexBindingDescriptions = NULL,
        .vertexAttributeDescriptionCount = 0,
        .pVertexAttributeDescriptions = NULL,
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

    static uint32_t dynamicStatesCount = 2;
    VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT,
                                      VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamicState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = dynamicStatesCount,
        .pDynamicStates = dynamicStates,
    };

    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = (float)extent.width,
        .height = (float)extent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };

    VkOffset2D offset = {
        .x = 0,
        .y = 0,
    };

    VkRect2D scissor = {
        .offset = offset,
        .extent = extent,
    };

    VkPipelineViewportStateCreateInfo viewportState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor,
    };

    VkPipelineRasterizationStateCreateInfo rasterizer = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .lineWidth = 1.0f,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f, // Optional
        .depthBiasClamp = 0.0f,          // Optional
        .depthBiasSlopeFactor = 0.0f,    // Optional
    };

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        .blendEnable = VK_TRUE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
    };

    VkPipelineColorBlendStateCreateInfo colorBlending = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
        .logicOpEnable = VK_FALSE,   // if VK_TRUE values below would be used
        .logicOp = VK_LOGIC_OP_COPY, // Optional
        .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f}, // Optional
    };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 0,         // Optional
        .pSetLayouts = NULL,         // Optional
        .pushConstantRangeCount = 0, // Optional
        .pPushConstantRanges = NULL, // Optional
    };

    VkPipelineLayout pipelineLayout;

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, NULL,
                               &pipelineLayout) != VK_SUCCESS) {
        fprintf(stderr, "ERROR: failed to create pipeline layout.\n");
        exit(1);
    }

    vkDestroyPipelineLayout(device, pipelineLayout, NULL);
    vkDestroyShaderModule(device, fragShaderModule, NULL);
    vkDestroyShaderModule(device, vertShaderModule, NULL);
}

int main() {
    GLFWwindow *window = initWindow();

    if (enableValidationLayers && !checkValidationLayerSupport()) {
        fprintf(stderr,
                "ERROR: validation layers requested, but not available\n");
        exit(1);
    }

    VkInstance instance = createInstance();
    VkDebugUtilsMessengerEXT debugMessenger;

    if (enableValidationLayers) {
        debugMessenger = setupDebugMessenger(instance);
    };

    VkSurfaceKHR surface = createSurface(instance, window);

    VkPhysicalDevice physicalDevice = pickPhysicalDevice(instance, surface);
    VkDevice device = createLogicalDevice(physicalDevice, surface);

    VkQueue graphicsQueue = getGraphicsQueue(device, physicalDevice);
    VkQueue presentationQueue =
        getPresentationQueue(device, physicalDevice, surface);

    VkSurfaceFormatKHR format = chooseSurfaceFormat(physicalDevice, surface);
    VkPresentModeKHR presentMode = choosePresentMode(physicalDevice, surface);
    VkExtent2D extent = chooseExtent(physicalDevice, surface, window);

    VkSwapchainKHR swapchain = createSwapchain(device, physicalDevice, surface,
                                               format, extent, presentMode);

    uint32_t imageCount;
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, NULL);
    VkImage swapchainImages[imageCount];
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages);

    VkImageView swapchainImageViews[imageCount];
    createImageViews(device, swapchainImageViews, swapchainImages, imageCount,
                     format);

    createGraphicsPipeline(device, extent);

    // random code to test cglm works
    mat4 matrix;
    vec4 vec;
    vec4 res;
    glm_mat4_mulv(matrix, vec, res);

    // main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    for (uint32_t i = 0; i < imageCount; i++) {
        vkDestroyImageView(device, swapchainImageViews[i], NULL);
    }

    vkDestroySwapchainKHR(device, swapchain, NULL);
    vkDestroyDevice(device, NULL);
    vkDestroySurfaceKHR(instance, surface, NULL);

    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, NULL);
    }

    vkDestroyInstance(instance, NULL);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
