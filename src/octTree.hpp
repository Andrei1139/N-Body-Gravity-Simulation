#pragma once

#include "particle.hpp"
#include <qtypes.h>
#include <vector>
struct Tile {
    Tile() {
        for (auto i = 0; i < 8; ++i) {
            children[i] = (uint)-1;
        }
    }
    int leftUpCoords[3], rightDownCoords[3];
    float mass = 0;
    uint nrParticles = 0;
    float coords[3] = {0, 0, 0};
    uint children[8];

    static void updateOctTree(const std::vector<Particle> &particles, std::vector<Tile> &octTree);
    static void displayOctTree(const std::vector<Tile> &octTree, int index = 0, int depth = 0);
};