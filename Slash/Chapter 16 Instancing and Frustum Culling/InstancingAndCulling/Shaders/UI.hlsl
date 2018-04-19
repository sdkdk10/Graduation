//=============================================================================
// Sky.fx by Frank Luna (C) 2011 All Rights Reserved.
//=============================================================================

// Include common HLSL code.
#include "Common.hlsl"
//
//
VertexOut_UI VS(VertexIn_UI vin)
{

	VertexOut_UI vout = (VertexOut_UI)0.0f;
	// Use local vertex position as cubemap lookup vector.
	vout.PosL = vin.PosL;

	// Transform to world space.
	float4 posW = mul(float4(vin.PosL, 1.0f), gWorld);

	// Always center sky about camera.

	// Set z = w so that z/w = 1 (i.e., skydome always on far plane).
	vout.PosH = mul(posW, gViewProj).xyww;

	return vout;
}

float4 PS(VertexOut_UI pin) : SV_Target
{
	float4 litColor = gCubeMap.Sample(gsamLinearWrap, pin.PosL);

	//litColor = ceil(litColor * 5) / 5.0f;
	return litColor;

}



