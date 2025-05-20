CC ?= gcc
CFLAGS = -std=c99 -O2
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

vert.spv: shader.vert
	glslc shader.vert -o vert.spv

frag.spv: shader.frag
	glslc shader.frag -o frag.spv

VulkanTest: main.c frag.spv vert.spv
	$(CC) $(CFLAGS) -o VulkanTest main.c $(LDFLAGS)

.PHONY: test clean

test: VulkanTest
	./VulkanTest

clean:
	rm -f VulkanTest *.spv
