//--------------------------------------------------------------------------------------
// File: DX11 Framework.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer ConstantBuffer : register( b0 )
{
	matrix World;
	matrix View;
	matrix Projection;
	float4 DiffuseMtrl;
	float4 DiffuseLight;
	float3 LightVecW;
	float gTime;
	float4 AmbientMaterial;
	float4 AmbientLight;
}

//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR0;
};

//------------------------------------------------------------------------------------
// Vertex Shader - Implements Gouraud Shading using Diffuse lighting only
//------------------------------------------------------------------------------------
VS_OUTPUT VS(float4 Pos : POSITION, float3 NormalL : NORMAL)
{
	VS_OUTPUT output = (VS_OUTPUT)0;

	output.Pos = mul(Pos, World);
	output.Pos = mul(output.Pos, View);
	output.Pos = mul(output.Pos, Projection);

	// Convert from local space to world space 
	// W component of vector is 0 as vectors cannot be translated
	float3 normalW = mul(float4(NormalL, 0.0f), World).xyz;
		normalW = normalize(normalW);

	// Compute Colour using Diffuse lighting only
	float diffuseAmount = max(dot(LightVecW, normalW), 0.0f);
	float4 ambient = AmbientMaterial * AmbientLight;
	float3 diffuse = diffuseAmount * (DiffuseMtrl * DiffuseLight).rgb;
	output.Color.rgb = diffuse + ambient;
	output.Color.a = DiffuseMtrl.a;

	return output;
}



//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( VS_OUTPUT input ) : SV_Target
{
    return input.Color;
}
