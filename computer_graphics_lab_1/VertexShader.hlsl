#define NUM_INSTANCES 10
#define NUM_TEX 2

struct VSInput
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD;
};

struct VSOutput
{
    float4 pos : SV_Position;
    float3 worldPos : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float2 tex : TEXCOORD2;
    nointerpolation uint texIndex : TEXCOORD3;
};

cbuffer mBuffer : register(b0)
{
    float4x4 models[NUM_INSTANCES];
    float4x4 normals[NUM_INSTANCES];
    float4 isNormalMapActive[NUM_INSTANCES];
};

cbuffer vpBuffer : register(b1)
{
    float4x4 vp;
};

VSOutput VSMain(VSInput input, uint instanceID : SV_InstanceID)
{
    VSOutput output;
    float4 WorldPos = mul(models[instanceID], float4(input.pos, 1.0));
    output.pos = mul(vp, WorldPos);
    output.worldPos = WorldPos.xyz;
    output.normal = mul((float3x3) normals[instanceID], input.normal);
    output.tex = input.tex;
    output.texIndex = isNormalMapActive[instanceID];
    return output;
}