RWTexture2D<float4> tex: register(u0);

[numthreads(16, 16, 1)]
void CSMain(uint3 threadID: SV_DispatchThreadID) {
    tex[threadID.xy] = 0;  
}