/////////////
// GLOBALS //
/////////////
cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

cbuffer ScreenSizeBuffer
{
    float screenSize;
    float3 paddingSizeBuffer;
};

//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
    float4 position : POSITION;
    float2 tex: TEXCOORD0;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float2 texCoord1 : TEXCOORD1;
    float2 texCoord2 : TEXCOORD2;
    float2 texCoord3 : TEXCOORD3;
    float2 texCoord4 : TEXCOORD4;
    float2 texCoord5 : TEXCOORD5;
    float2 texCoord6 : TEXCOORD6;
    float2 texCoord7 : TEXCOORD7;
    float2 texCoord8 : TEXCOORD8;
    float2 texCoord9 : TEXCOORD9;
};


////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType ColorVertexShader(VertexInputType input)
{
    PixelInputType output;
    float texelSize;

	// Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;

	// Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = input.position;// + float4(1.0, 0.0, 0.0, 0.0);

	// Store the input color for the pixel shader to use.
    output.tex = input.tex;
    
	// Determine the floating point size of a texel for a screen with this specific height.
    texelSize = 1.0f / screenSize;

    // Create UV coordinates for the pixel and its four vertical neighbors on either side.
    output.texCoord1 = input.tex + float2(0.0f, texelSize * -4.0f);
    output.texCoord2 = input.tex + float2(0.0f, texelSize * -3.0f);
    output.texCoord3 = input.tex + float2(0.0f, texelSize * -2.0f);
    output.texCoord4 = input.tex + float2(0.0f, texelSize * -1.0f);
    output.texCoord5 = input.tex + float2(0.0f, texelSize *  0.0f);
    output.texCoord6 = input.tex + float2(0.0f, texelSize *  1.0f);
    output.texCoord7 = input.tex + float2(0.0f, texelSize *  2.0f);
    output.texCoord8 = input.tex + float2(0.0f, texelSize *  3.0f);
    output.texCoord9 = input.tex + float2(0.0f, texelSize *  4.0f);

    return output;
}