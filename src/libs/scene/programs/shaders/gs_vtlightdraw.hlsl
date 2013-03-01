struct VS_IN {
    float4 coords : coords;         // [min.xy, max.xy] in clip space
    float quadZ : quadZ;
    uint lightIndex : lightIndex;
};

struct GS_OUT {
    float4 positionViewport : SV_Position;
    // NOTE: Using a uint4 to work around a compiler bug. Otherwise the SV_SampleIndex input to the per-sample
    // shader below gets put into a '.y' which doesn't appear to work on some implementations.
    nointerpolation uint4 lightIndex : lightIndex;
};

// Takes point output and converts into screen-space quads
[maxvertexcount(4)]
void main(point VS_IN input[1], inout TriangleStream<GS_OUT> quadStream)
{
    GS_OUT output;
    output.lightIndex = input[0].lightIndex;
    output.positionViewport.zw = float2(input[0].quadZ, 1.0f);

    // Branch around empty regions (i.e. light entirely offscreen)
    if (all(input[0].coords.xy < input[0].coords.zw)) {
        output.positionViewport.xy = input[0].coords.xw;      // [-1,  1]
        quadStream.Append(output);
        output.positionViewport.xy = input[0].coords.zw;      // [ 1,  1]
        quadStream.Append(output);
        output.positionViewport.xy = input[0].coords.xy;      // [-1, -1]
        quadStream.Append(output);
        output.positionViewport.xy = input[0].coords.zy;      // [ 1, -1]
        quadStream.Append(output);

        quadStream.RestartStrip();
    }
}
