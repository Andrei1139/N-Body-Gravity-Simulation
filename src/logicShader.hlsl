#define EPSILON 1000

cbuffer Data {
    float G;
    float theta;
    float2 padding;
};
RWTexture2D<float4> tex: register(u0);
struct Particle {
    float3 pos;
    float mass;
    float3 vel;
    float3 acc;
};
RWStructuredBuffer<Particle> particles: register(u1);
struct Tile {
    int3 leftUp, rightDown;
    float mass;
    uint nrParticles;
    float3 coords;
    uint children[8];
};
StructuredBuffer<Tile> octTree: register(t0);

[numthreads(32, 1, 1)]
void CSMain(uint3 threadID: SV_DispatchThreadID) {
    uint nrParticles, aux;
    particles.GetDimensions(nrParticles, aux);

    if (threadID.x >= nrParticles) {
        return;
    }

    // Original inter-particle interaction algorithm (O(n^2))

    // uint index = threadID.x;

    // float3 oldAcc = particles[index].acc, newAcc = float3(0, 0, 0);

    // float3 vecRadius;
    // float radius;

    // for (uint i = 0; i < nrParticles; ++i) {
    //     Particle currParticle = particles[i];
    //     float3 vecRadius = (currParticle.pos - particles[index].pos);
    //     float radius = sqrt(pow(vecRadius.x, 2) + pow(vecRadius.y, 2) + pow(vecRadius.z, 2));

    //     newAcc += G * currParticle.mass * vecRadius / (pow(radius, 3) + EPSILON);
    // }

    // // particles[index].vel += particles[index].acc;
    // // particles[index].pos += particles[index].vel;
    // particles[index].pos += particles[index].vel + oldAcc / 2;
    // particles[index].vel += (oldAcc + newAcc) / 2;
    // particles[index].acc = newAcc;


    uint stack[100];
    int stackPtr = 0;
    stack[0] = 0;

    Particle currParticle = particles[threadID.x];
    float3 newAcc = float3(0, 0, 0);

    while (stackPtr >= 0) {
        Tile tile = octTree[stack[stackPtr--]];
        float3 vecRadius = (tile.coords - currParticle.pos);
        float radius = sqrt(pow(vecRadius.x, 2) + pow(vecRadius.y, 2) + pow(vecRadius.z, 2));

        // Only use virtual particles in certain conditions
        if (theta * radius >= float(tile.rightDown.x - tile.leftUp.x) || tile.nrParticles == 1) {
            newAcc += G * tile.mass * vecRadius / (pow(radius, 3) + EPSILON);
        } else {
            // Else push children on stack
            for (uint i = 0; i < 8; ++i) {
                if (tile.children[i] != (uint)-1) {
                    stack[++stackPtr] = tile.children[i];
                }
            }
        }
    }

    currParticle.pos += currParticle.vel + currParticle.acc / 2;
    currParticle.vel += (currParticle.acc + newAcc) / 2;
    currParticle.acc = newAcc;

    particles[threadID.x] = currParticle;
}