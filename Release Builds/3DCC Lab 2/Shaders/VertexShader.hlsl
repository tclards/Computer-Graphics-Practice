// an ultra simple hlsl vertex shader
#pragma pack_matrix(row_major)
// TODO: Part 1F - DONE
// TODO: Part 1H - DONE
// TODO: Part 2B - DONE
// TODO: Part 2D - DONE
// TODO: Part 4A - DONE
// TODO: Part 4B - DONE

struct inputFromAssembler
{
    float3 pos : POS;
    float3 uvm : UVM;
    float3 nrm : NRM;
};

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

outputToRasterizer main(inputFromAssembler inputVertex)
{
    float4 pos = { inputVertex.pos, 1.0f };
    pos = mul(pos, worldMat);
    float4 posW = pos;
    pos = mul(pos, viewMat);
    pos = mul(pos, projMat);
    
    float4 normal = mul(float4(inputVertex.nrm, 0), worldMat);
    
    outputToRasterizer output;
    output.posH = pos;
    output.posW = posW.xyz;
    output.normW = normalize(normal.xyz);
    
    return output;
}