#version 450

// location = 0 specifies the location of the framebuffer
layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

void main()
{
	outColor = vec4(fragColor, 1.0);
}
