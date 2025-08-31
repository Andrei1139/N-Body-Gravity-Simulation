#pragma once

struct Particle {
    Particle(float x, float y, float z, float mass, float vx, float vy, float vz):
    pos{x, y, z},
    mass{mass},
    vel{vx, vy, vz},
    acc{0, 0, 0} {
    }
    Particle() = default;

    float pos[3];
    float mass;
    float vel[3];
    float acc[3];
};