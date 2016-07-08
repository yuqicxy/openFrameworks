#version 420 core

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

// uniforms (resources)
layout (set = 0, binding = 0) uniform DefaultMatrices 
{
	mat4 projectionMatrix;
	mat4 modelMatrix;
	mat4 viewMatrix;
}; // note: if you don't specify a variable name for the block its elements will live in the global namespace.

layout (set = 1, binding = 0) uniform StyleSet
{
	vec4 globalColor;
} style;

// inputs (vertex attributes)
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec4 inColor;
layout (location = 2) in vec3 inNormal;

// outputs 
layout (location = 0) out vec4 outColor;
layout (location = 1) out vec3 outNormal;

void main() 
{
	outNormal   = (inverse(transpose( viewMatrix * modelMatrix)) * vec4(inNormal, 0.0)).xyz;
	outColor    = style.globalColor;
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(inPos.xyz, 1.0);
}
