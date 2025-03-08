Texture2D g_Texture : register(t0);
SamplerState g_Sampler : register(s0);

struct VSOutput
{
    float4 pos : SV_Position;
    float2 tex : TEXCOORD;
};

float4 PSMain(VSOutput input) : SV_Target
{
    return g_Texture.Sample(g_Sampler, input.tex);

}