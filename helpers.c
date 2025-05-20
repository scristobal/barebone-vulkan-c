#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <vulkan/vulkan_core.h>

void displayInstanceExtensions() {
    uint32_t extensionCount = 0;

    if (vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL) !=
        VK_SUCCESS) {
        fprintf(stderr, "ERROR: failed to query extensions\n");
        exit(1);
    }

    VkExtensionProperties extensions[extensionCount];

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

void displayRequiredInstanceExtensions(uint32_t extensionCount, const char **extensions) {
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

void displayAvailableExtensions(uint32_t availableExtensionCount,
                            VkExtensionProperties *availableExtensions) {
    fprintf(stdout, "There are %d avaiable layers:\n", availableExtensionCount);
    for (uint32_t i; i < availableExtensionCount; i++) {
        fprintf(stdout, "\t%s\n", availableExtensions[i].extensionName);
    }
    fprintf(stdout, "\n");
}

void displayDevice(VkPhysicalDevice *device) {

    VkPhysicalDeviceProperties deviceProps;
    vkGetPhysicalDeviceProperties(*device, &deviceProps);

    fprintf(stdout, "physical device: %s\n", deviceProps.deviceName);
}

void displayDevices(VkPhysicalDevice *devices, uint32_t deviceCount) {

    for (uint32_t i = 0; i < deviceCount; i++) {
        displayDevice(&devices[i]);
    }
}

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/**
 * Maps the contents of a file into memory (read-only).
 *
 * Note: Caller must call munmap(ptr, size). Example:
 *
 * ```c
 * size_t size;
 * void *data = mmap_file_read("example.txt", &size);
 * if (!data) {
 *   return 1;
 * }
 *
 * // Cleanup
 * if (munmap(data, size) == -1) {
 *   perror("munmap");
 *   return 1;
 * }
 * ```
 *
 */
void *mmap_file_read(const char *path, size_t *size_out) {
    if (!path || !size_out) return NULL;

    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        return NULL;
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        close(fd);
        return NULL;
    }

    if (st.st_size == 0) {
        close(fd);
        return NULL;
    }

    void *mapped = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped == MAP_FAILED) {
        close(fd);
        return NULL;
    }

    close(fd);

    *size_out = (size_t)st.st_size;
    return mapped;
}


