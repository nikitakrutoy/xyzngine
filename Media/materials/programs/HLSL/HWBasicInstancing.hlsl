struct VS_INPUT
{
	float4 Position	:	SV_POSITION;
	float3 Normal	:	NORMAL;
	float3 Tangent	:	TANGENT;
	float2 uv0		:	TEXCOORD0;

	float4 mat14	:	TEXCOORD1;
	float4 mat24	:	TEXCOORD2;
	float4 mat34	:	TEXCOORD3;
};

struct PS_INPUT
{
#ifdef DEPTH_SHADOWCASTER
	float3 unused	:	TEXCOORD0;
	float depth		:	TEXCOORD1;
#else
	float2 uv0		:	TEXCOORD0;
	float3 Normal	:	TEXCOORD1;
	float3 vPos		:	TEXCOORD2;
	
	#ifdef DEPTH_SHADOWRECEIVER
		float4 lightSpacePos	:	TEXCOORD3;
	#endif
#endif
};

struct VS_OUTPUT
{
	float4 Position	:	SV_POSITION;
	PS_INPUT	ps;
};

#define SHADOW_BIAS 0

VS_OUTPUT main_vs( VS_INPUT input,
				   uniform float4x4 viewProjMatrix

#if defined( DEPTH_SHADOWCASTER ) || defined( DEPTH_SHADOWRECEIVER )
				,  uniform float4 depthRange
#endif
#ifdef DEPTH_SHADOWRECEIVER
				,  uniform float4x4 texViewProjMatrix
#endif
				)
{
	VS_OUTPUT output;

	float3x4 worldMatrix;
	worldMatrix[0] = input.mat14;
	worldMatrix[1] = input.mat24;
	worldMatrix[2] = input.mat34;

	float4 worldPos = float4( mul( worldMatrix, input.Position ).xyz, 1.0f );
	float3 worldNorm= mul( (float3x3)(worldMatrix), input.Normal );

	//Transform the position
	output.Position		= mul( viewProjMatrix, worldPos );

#ifdef DEPTH_SHADOWCASTER
	output.ps.unused	= float3( 0, 0, 0 );
	output.ps.depth		= (output.Position.z - depthRange.x + SHADOW_BIAS) * depthRange.w;
#else
	output.ps.uv0		= input.uv0;

	//Pass Normal and position for Blinn Phong lighting
	output.ps.Normal	= normalize(worldNorm);
	output.ps.vPos		= worldPos.xyz;
	
	#ifdef DEPTH_SHADOWRECEIVER
		// Calculate the position of vertex in light space to do shadows
		output.ps.lightSpacePos = mul( texViewProjMatrix, worldPos );
		// make linear
		output.ps.lightSpacePos.z = (output.ps.lightSpacePos.z - depthRange.x) * depthRange.w;
	#endif
#endif

	return output;
}
