#define MIN_XY_LIMIT -500
#define MAX_XY_LIMIT 2000

RWTexture2D<float4> tex: register(u0);
struct Particle {
    float3 pos;
    float mass;
    float3 vel;
    float3 acc;
};
RWStructuredBuffer<Particle> particles: register(u1);

// Original rendering algorithm

// [numthreads(16, 16, 1)]
// void CSMain(uint3 threadID: SV_DispatchThreadID) {
//     uint nrParticles, filterW, filterH, texW, texH, aux;
//     particles.GetDimensions(nrParticles, aux);
//     filter.GetDimensions(filterW, filterH);
//     tex.GetDimensions(texW, texH);

//     int2 pixelPos = int2(threadID.xy);
//     int2 filterPos = int2(pixelPos.x / filterW, pixelPos.y / filterH);

//     tex[threadID.xy] = float4(0, 0, 0, 1);
//     if (filter[filterPos] == 0) {
//         return;
//     }

//     for (uint i = 0; i < nrParticles; ++i) {
//         int2 intPos = int2(particles[i].pos.xy);
//         if (pow(pixelPos.x - intPos.x, 2) + pow(pixelPos.y - intPos.y, 2) <= pow(particles[i].radius, 2) &&
//             intPos.x >= MIN_XY_LIMIT && intPos.y >= MIN_XY_LIMIT &&
//             intPos.x <= MAX_XY_LIMIT && intPos.y <= MAX_XY_LIMIT) {
//             // tex[threadID.xy] = float4(particles[i].color / (z / 100 + 1), 1);
//             tex[threadID.xy] = float4(particles[i].color, 1);
//             return;
//         }
//     }
// }

[numthreads(32, 1, 1)]
void CSMain(uint3 threadID: SV_DispatchThreadID) {
    tex[int2(particles[threadID.x].pos.xy)] = float4(1, 1, 1, 1);
}