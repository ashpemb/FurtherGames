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
	float4 SpecularMaterial;
	float4 SpecularLight;
	float SpecularPower;
	float3 EyePosW;

}

//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    //float4 Color : COLOR0;
	float3 NormalW : NORMAL;
	float3 PosW : POSITION;
};

//------------------------------------------------------------------------------------
// Vertex Shader - Implements Gouraud Shading using Diffuse lighting only
//------------------------------------------------------------------------------------
VS_OUTPUT VS(float4 Pos : POSITION, float3 NormalL : NORMAL)
{
	VS_OUTPUT output = (VS_OUTPUT)0;


	output.Pos = mul(Pos, World);

	output.PosW = output.Pos;

	output.Pos = mul(output.Pos, View); // VS
	output.Pos = mul(output.Pos, Projection);


	// Apply View and Projection transformations
	float3 normalW = mul(float4(NormalL, 0.0f), World).xyz; // VS
	output.NormalW = normalize(normalW);
	// Convert normal from local space to world space



	return output;

}



//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( VS_OUTPUT input ) : SV_Target
{
	float3 normalW = normalize(input.NormalW);
	// Compute the vector from the vertex to the eye position.
	// output.Pos is currently the position in world space
	float3 toEye = normalize(EyePosW - input.PosW.xyz); // PS

	// Compute Colour
	// Compute the reflection vector.
	float3 r = reflect(-LightVecW, normalW); // PS

	// Determine how much (if any) specular light makes it
	// into the eye.
	float specularAmount = pow(max(dot(r, toEye), 0.0f), SpecularPower);

	// Calculate Diffuse and Ambient Lighting
	float diffuseAmount = max(dot(LightVecW, normalW), 0.0f);
	float4 ambient = AmbientMaterial * AmbientLight;
		float3 diffuse = diffuseAmount * (DiffuseMtrl * DiffuseLight).rgb;
		// Compute the ambient, diffuse, and specular terms separately.
		float3 specular = specularAmount * (SpecularMaterial * SpecularLight).rgb;

		// Sum all the terms together and copy over the diffuse alpha.
		float4 Color;
	Color.rgb = ambient + diffuse + specular;
	Color.a = DiffuseMtrl.a;

    return Color;
}
