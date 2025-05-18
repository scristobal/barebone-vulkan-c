#include "helpers.c"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
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
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = validationLayerCount;
        createInfo.ppEnabledLayerNames = validationLayers;

        populateDebugMessengerCreateInfo(&debugCreateInfo);
        createInfo.pNext =
            (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = NULL;
    }

    // displayInstanceExtensions();

    uint32_t extensionCount;
    const char **extensions = getRequiredExtensions(&extensionCount);

    createInfo.enabledExtensionCount = extensionCount;
    createInfo.ppEnabledExtensionNames = extensions;

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

    uint32_t presentationCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
                                              &presentationCount, NULL);
    if (formatCount == 0 || presentationCount == 0)
        return false;

    if (getGraphicsFamily(device) == -1 ||
        getPresentationFamily(device, surface) == -1)
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

        VkDeviceQueueCreateInfo queueCreateInfo = {};

        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        queueCreateInfos[i] = queueCreateInfo;
    }

    if (queueCreateInfoCount == 2) {
        queueCreateInfos[0].queueFamilyIndex = graphicsIndex;
        queueCreateInfos[1].queueFamilyIndex = presentaionIndex;
    } else {
        queueCreateInfos[0].queueFamilyIndex = graphicsIndex;
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.pQueueCreateInfos = queueCreateInfos;
    createInfo.queueCreateInfoCount = queueCreateInfoCount;
    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.ppEnabledExtensionNames = deviceExtensions;
    createInfo.enabledExtensionCount = 1;

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = validationLayerCount;
        createInfo.ppEnabledLayerNames = validationLayers;
    } else {
        createInfo.enabledLayerCount = 0;
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

void createSwapchain(VkPhysicalDevice device, VkSurfaceKHR surface) {
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, NULL);

    VkSurfaceFormatKHR formats[formatCount];
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount,
                                         formats);
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
                                              &presentModeCount, NULL);
    VkPresentModeKHR presetModes[presentModeCount];
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
                                              &presentModeCount, presetModes);
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

    createSwapchain(physicalDevice, surface);

    // random code to test cglm works
    mat4 matrix;
    vec4 vec;
    vec4 res;
    glm_mat4_mulv(matrix, vec, res);

    // main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

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
