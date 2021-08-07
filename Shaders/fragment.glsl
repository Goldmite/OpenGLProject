#version 450 core
out vec4 FragColor;

in vec2 TexCoord;

layout (binding = 0) uniform sampler2D texture;
layout (binding = 1, rgba32f) uniform image2D outTexture;

void main()
{
    ivec2 ID = ivec2(gl_FragCoord.xy);
    float ColorDecay = 0.25;
    vec4 color = texelFetch(texture, ID, 0).rgba;
    imageStore(outTexture, ID, max(color - ColorDecay, 0.0f));
    FragColor = color;
}