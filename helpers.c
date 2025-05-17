#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <vulkan/vulkan_core.h>

void displayEXT() {
    uint32_t extensionCount = 0;
    VkExtensionProperties extensions[256];

    if (vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL) !=
        VK_SUCCESS) {
        fprintf(stderr, "ERROR: failed to query extensions\n");
        exit(1);
    }

    if (extensionCount > 256)
        extensionCount = 256; // truncate to fit on array

    if (vkEnumerateInstanceExtensionProperties(NULL, &extensionCount,
                                               extensions) != VK_SUCCESS) {
        fprintf(stderr, "ERROR: failed to query extensions\n");
        exit(1);
    }

    fprintf(stdout, "Your system supports %d extensions:\n", extensionCount);
    for (uint32_t i = 0; i < extensionCount; i++) {
        fprintf(stdout, "\t%s\n", extensions[i].extensionName);
    }
    fprintf(stdout, "\n");
}

void displayRequiredEXT(uint32_t extensionCount, const char **extensions) {
    fprintf(stdout, "Number of required extensions %d:\n", extensionCount);
    for (uint32_t i; i < extensionCount; i++) {
        fprintf(stdout, "\t%s\n", extensions[i]);
    }
    fprintf(stdout, "\n");
}

void displayAvailableLayers(uint32_t availableLayerCount,
                            VkLayerProperties *availableLayers) {
    fprintf(stdout, "There are %d avaiable layers:\n", availableLayerCount);
    for (uint32_t i; i < availableLayerCount; i++) {
        fprintf(stdout, "\t%s\n", availableLayers[i].layerName);
    }
    fprintf(stdout, "\n");
}
