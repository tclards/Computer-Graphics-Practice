// an ultra simple hlsl pixel shader
// TODO: Part 3A - DONE
// TODO: Part 4B - DONE
// TODO: Part 4C - DONE
// TODO: Part 4F - DONE

struct outputToRasterizer
{
    float4 posH : SV_Position;
    float3 posW : UVM;
    float3 normW : NRM;
};

struct OBJ_MATERIAL
{
    float3 Kd; // diffuse reflectivity
    float d; // dissolve (transparency) 
    float3 Ks; // specular reflectivity
    float Ns; // specular exponent
    float3 Ka; // ambient reflectivity
    float sharpness; // local reflection map sharpness
    float3 Tf; // transmission filter
    float Ni; // optical density (index of refraction)
    float3 Ke; // emissive reflectivity
    unsigned int illum; // illumination model
};

cbuffer SceneData : register(B0)
{
    float4x4 viewMat, projMat;

    float4 lightDir;
    float4 lightColor;
    float4 camWorldPos;
    float4 sunAmbient;
};

cbuffer MeshData : register(B1)
{
    float4x4 worldMat;
    OBJ_MATERIAL material;
};

float4 main(outputToRasterizer outputVS) : SV_TARGET
{
    // Lighting Variables
    float alphaTransparency = material.d;
    float3 surfaceColor = material.Kd;
    float3 light_Color = lightColor.xyz;
    float3 lightNormal = normalize(lightDir.xyz);
    float3 surfaceNormal = normalize(outputVS.normW);
    float3 viewDir = normalize(camWorldPos.xyz - outputVS.posW.xyz);
    float3 halfVector = normalize((-lightNormal) + viewDir);
    float specularExponent = material.Ns + 0.000001f;
    float3 specularColor = material.Ks;
    float3 ambientTerm = sunAmbient.xyz;
    float3 ambientLight = ambientTerm * material.Ka;
    float3 emissive = material.Ke;
    
    // directional
    float3 directional = surfaceColor;
    float lightRatio = saturate(dot(-lightNormal, surfaceNormal));
    directional = lightRatio * light_Color;
    
    // specular
    float intensity = max(pow(saturate(dot(surfaceNormal, halfVector)), specularExponent), 0);
    float3 specular =  specularColor * intensity;
    
    // final result calculation
    float3 finalResult = saturate(directional + ambientLight) * surfaceColor + specular + emissive;
    
    return float4(finalResult, alphaTransparency);
}