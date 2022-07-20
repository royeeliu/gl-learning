#include "common.hlsli"

cbuffer SceneConstantBuffer : register(b0)
{
    float4 offset;
};

PSInput VSMain(float4 position : POSITION, float4 color : COLOR)
{
    PSInput result;

    result.position = position + offset;
    result.color = color;

    return result;
}