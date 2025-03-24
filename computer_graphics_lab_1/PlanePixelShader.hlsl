cbuffer LightBuffer : register(b0)
{
    float3 lightPos;
    float pad0;
    float3 lightColor;
    float pad1;
    float3 ambient;
    float pad2;
    float3 cameraPosition;
    float pad3;
};

struct VSOutput
{
    float4 pos : SV_Position;
    float3 WorldPos : TEXCOORD0;
    float3 Normal : TEXCOORD1;
    float4 color : COLOR;
};

float4 PSMain(VSOutput input) : SV_Target
{
    
    float3 normal = normalize(input.Normal);

    float3 lightDir = lightPos - input.WorldPos;
    float lightDirLength = length(lightDir);
    float attenuation = clamp(1 / (lightDirLength * lightDirLength), 0, 1);
    lightDir = normalize(lightDir);

    float diff = saturate(dot(normal, lightDir));

    float3 viewDir = normalize(cameraPosition - input.WorldPos);
    float3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(saturate(dot(viewDir, reflectDir)), 32);

    float3 ambientColor = ambient * input.color.rgb;
    float3 diffuseColorFinal = input.color.rgb * lightColor * diff * attenuation;
    float3 specularColor = lightColor * spec;

    float3 finalColor = ambientColor + diffuseColorFinal + specularColor;

    return float4(finalColor, input.color.a);

}