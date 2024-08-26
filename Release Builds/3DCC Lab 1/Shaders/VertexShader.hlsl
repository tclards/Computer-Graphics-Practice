// an ultra simple hlsl vertex shader
#pragma pack_matrix(row_major)

// TODO: Part 1C - DONE
// TODO: Part 2B - DONE
// TODO: Part 2F - DONE
// TODO: Part 2G - DONE
// TODO: Part 3B - DONE

cbuffer SHADER_VARS
{
    float4x4 world;
    float4x4 view;
    float4x4 perspective;
};

float4 worldMat(float4 inputVertex : POSITION) : POSITION
{
    return mul(inputVertex, world);
}

float4 viewMat(float4 inputVertex : POSITION) : POSITION
{
    return mul(inputVertex, view);
}

float4 perspectiveMat(float4 inputVertex : POSITION) : POSITION
{
    return mul(inputVertex, perspective);
}

float4 main(float4 inputVertex : POSITION) : SV_POSITION
{
    float4 temp1 = worldMat(inputVertex);
    float4 temp2 = viewMat(temp1);
    float4 output = perspectiveMat(temp2);

    return float4(output);
}