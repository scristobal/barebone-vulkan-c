#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <vulkan/vulkan_core.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

const char *validationLayers[] = {"VK_LAYER_KHRONOS_validation"};

const uint32_t validationLayerCount =
    sizeof(validationLayers) / sizeof(validationLayers[0]);

#include <cglm/cglm.h>
#include <cglm/mat4.h>
#include <cglm/vec4.h>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

GLFWwindow *initWindow() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    return glfwCreateWindow(WIDTH, HEIGHT, "Vulkan window", NULL, NULL);
}

bool checkValidationLayerSupport() {
    uint32_t availableLayerCount;
    vkEnumerateInstanceLayerProperties(&availableLayerCount, NULL);

    VkLayerProperties availableLayers[256];
    vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers);

    if (availableLayerCount > 256)
        availableLayerCount = 256;

    // fprintf(stdout, "There are %d avaiable layers:\n", availableLayerCount);
    // for (uint32_t i; i < availableLayerCount; i++) {
    //     fprintf(stdout, "\t%s\n", availableLayers[i].layerName);
    // }
    // fprintf(stdout, "\n");

    for (uint32_t i; i < validationLayerCount; i++) {
        bool layerFound = false;

        for (uint32_t i = 0; i < availableLayerCount; i++) {
            if (strcmp(validationLayers[i], availableLayers[i].layerName) ==
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

void getRequiredExtensions(uint32_t *extensionCount, const char ***extensions) {
    const char **required;
    required = glfwGetRequiredInstanceExtensions(extensionCount);

    *extensions = required;

    if (!enableValidationLayers) {
        return;
    }

    const char **result = malloc((*extensionCount + 1) * sizeof(const char *));
    if (!result) {
        return;
    }

    for (uint32_t i; i < *extensionCount; i++) {
        result[i] = required[i];
    }

    result[*extensionCount] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    ++*extensionCount;

    *extensions = result;
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

    if (!enableValidationLayers) {
        return debugMessenger;
    }

    VkDebugUtilsMessengerCreateInfoEXT createInfo = {};

    populateDebugMessengerCreateInfo(&createInfo);

    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, NULL,
                                     &debugMessenger) != VK_SUCCESS) {
        fprintf(stderr, "failed to set up debug messenger.\n");
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

VkInstance createInstance() {
    if (enableValidationLayers && !checkValidationLayerSupport()) {
        fprintf(stderr,
                "ERROR: validation layers requested, but not available\n");
        exit(1);
    }

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

    uint32_t extensionCount;
    const char **extensions;

    getRequiredExtensions(&extensionCount, &extensions);

    createInfo.enabledExtensionCount = extensionCount;
    createInfo.ppEnabledExtensionNames = extensions;

    // fprintf(stdout, "Number of required extensions %d:\n", extensionCount);
    // for (uint32_t i; i < extensionCount; i++) {
    //     fprintf(stdout, "\t%s\n", extensions[i]);
    // }
    // fprintf(stdout, "\n");

    VkInstance instance;

    if (vkCreateInstance(&createInfo, NULL, &instance) != VK_SUCCESS) {
        fprintf(stderr, "ERROR: failed to create instance\n");
        exit(1);
    }

    return instance;
};

void cleanup(GLFWwindow *window, VkInstance instance,
             VkDebugUtilsMessengerEXT debugMessenger) {
    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, NULL);
    }

    vkDestroyInstance(instance, NULL);
    glfwDestroyWindow(window);
    glfwTerminate();
}

int main() {
    GLFWwindow *window = initWindow();
    VkInstance instance = createInstance();
    VkDebugUtilsMessengerEXT debugMessenger = setupDebugMessenger(instance);

    uint32_t extensionCount = 0;
    VkExtensionProperties extensions[256];

    if (vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL) !=
        VK_SUCCESS) {
        fprintf(stderr, "ERROR: failed to query extensions\n");
        exit(1);
    }

    if (extensionCount > 256) {
        extensionCount = 256; // truncate to fit on array
    }

    if (vkEnumerateInstanceExtensionProperties(NULL, &extensionCount,
                                               extensions) != VK_SUCCESS) {
        fprintf(stderr, "ERROR: failed to query extensions\n");
        exit(1);
    }

    // fprintf(stdout, "Your system supports %d extensions:\n", extensionCount);
    // for (uint32_t i = 0; i < extensionCount; i++) {
    //     fprintf(stdout, "\t%s\n", extensions[i].extensionName);
    // }
    // fprintf(stdout, "\n");

    // random code to test cglm works
    mat4 matrix;
    vec4 vec;
    vec4 res;
    glm_mat4_mulv(matrix, vec, res);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    cleanup(window, instance, debugMessenger);

    return 0;
}
