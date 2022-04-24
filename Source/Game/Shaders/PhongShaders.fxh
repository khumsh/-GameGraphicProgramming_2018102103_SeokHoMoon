//--------------------------------------------------------------------------------------
// File: PhongShaders.fx
//
// Copyright (c) Kyung Hee University.
//--------------------------------------------------------------------------------------

#define NUM_LIGHTS (2)

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------

Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Cbuffer:  cbChangeOnCameraMovement
  Summary:  Constant buffer used for view transformation and shading
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/

cbuffer cbChangeOnCameraMovement : register(b0)
{
    matrix View;
    float4 CameraPosition;
}

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Cbuffer:  cbChangeOnResize
  Summary:  Constant buffer used for projection transformation
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/

cbuffer cbChangeOnResize : register(b1)
{
    matrix Projection;
};

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Cbuffer:  cbChangesEveryFrame
  Summary:  Constant buffer used for world transformation
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/

cbuffer cbChangesEveryFrame : register(b2)
{
    matrix World;
	float4 OutputColor;
};

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Cbuffer:  cbLights
  Summary:  Constant buffer used for shading
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/

cbuffer cbLights : register(b3)
{
    float4 LightPositions[NUM_LIGHTS];
    float4 LightColors[NUM_LIGHTS];
};

//--------------------------------------------------------------------------------------
/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Struct:   VS_PHONG_INPUT
  Summary:  Used as the input to the vertex shader
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/

struct VS_PHONG_INPUT
{
    float4 Pos : POSITION;
    float2 Tex : TEXCOORD0;
    float3 Normal : NORMAL;
};

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Struct:   PS_PHONG_INPUT
  Summary:  Used as the input to the pixel shader, output of the 
            vertex shader
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
struct PS_PHONG_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
    float3 Normal: NORMAL;
    float3 WorldPosition : WORLDPOS;
};


/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Struct:   PS_LIGHT_CUBE_INPUT
  Summary:  Used as the input to the pixel shader, output of the 
            vertex shader
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/

struct PS_LIGHT_CUBE_INPUT
{
    float4 Pos : SV_POSITION;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------

PS_PHONG_INPUT VSPhong( VS_PHONG_INPUT input )
{
    PS_PHONG_INPUT output = (PS_PHONG_INPUT)0;
    output.Pos = mul( input.Pos, World );
    output.Pos = mul( output.Pos, View );
    output.Pos = mul( output.Pos, Projection );

    output.Tex = input.Tex;

    output.Normal = normalize(mul(float4(input.Normal, 0), World).xyz);
    output.WorldPosition = mul( input.Pos, World );
    
    return output;
}


PS_LIGHT_CUBE_INPUT VSLightCube( VS_PHONG_INPUT input )
{
    PS_LIGHT_CUBE_INPUT output = (PS_LIGHT_CUBE_INPUT)0;
    output.Pos = mul( input.Pos, World );
    output.Pos = mul( output.Pos, View );
    output.Pos = mul( output.Pos, Projection );
    
    return output;
}
//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------

float4 PSPhong( PS_PHONG_INPUT input) : SV_Target
{
    /*
    Ambient light
        for each light:
            Compute the ambience term
                Recommended ambience is below 0.2
            The ambient term is then multiplied by the color of the light
        Multiply the ambient term by the color sampled from the texture

    (Ambience term * color) * (light color) = ma * sa
    */


	float3 diffuse = float3(0.0f, 0.0f, 0.0f);
    float3 ambience = float3(0.1f, 0.1f, 0.1f);
	float3 ambienceTerm = float3(0.0f, 0.0f, 0.0f);
	float3 specular = float3(0.0f, 0.0f, 0.0f);
	float3 viewDirection = normalize(input.WorldPosition - CameraPosition.xyz);
    
    for (uint i = 0; i < NUM_LIGHTS; ++i)  
    {     
        // (Ambience term * color) * (light color) = ma * sa
        ambienceTerm += (ambience * txDiffuse.Sample(samLinear, input.Tex).rgb) * LightColors[i].xyz;  
        
        float3 lightDirection = normalize(input.WorldPosition - LightPositions[i].xyz);
		float lambertianTerm = dot(normalize(input.Normal), -lightDirection);
		diffuse += max(lambertianTerm, 0.0f) * txDiffuse.Sample(samLinear, input.Tex).rgb * LightColors[i].xyz;
        
		float3 reflectDirection = normalize(reflect(lightDirection, input.Normal));
		specular += pow(max(dot(-viewDirection, reflectDirection), 0.0f), 8.0f) 
                    * LightColors[i].xyz * txDiffuse.Sample(samLinear, input.Tex).rgb;
    }
    

    return float4(saturate(diffuse + ambienceTerm + specular), 1.0f);

}


float4 PSLightCube(PS_LIGHT_CUBE_INPUT input) : SV_Target
{
	return OutputColor;
}