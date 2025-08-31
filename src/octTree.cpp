#include "octTree.hpp"
#include <iostream>
#include <synchapi.h>

static uint getChildIndexAndData(float coords[3], int leftUp[3], int rightDown[3], Tile &tile) {
    // Assume coords are within range
    int middle[3] = {(leftUp[0] + rightDown[0]) / 2, (leftUp[1] + rightDown[1]) / 2, (leftUp[2] + rightDown[2]) / 2};
    uint res = (coords[0] > middle[0]) + 2 * (coords[1] > middle[1]) + 4 * (coords[2] > middle[2]);

    // Left-side or right-side
    if (res % 2 == 0) {
        tile.leftUpCoords[0] = leftUp[0];
        tile.rightDownCoords[0] = middle[0];
    } else {
        tile.leftUpCoords[0] = middle[0];
        tile.rightDownCoords[0] = rightDown[0];
    }
    // Up or down
    if (res % 4 < 2) {
        tile.leftUpCoords[1] = leftUp[1];
        tile.rightDownCoords[1] = middle[1];
    } else {
        tile.leftUpCoords[1] = middle[1];
        tile.rightDownCoords[1] = rightDown[1];
    }
    // Front or back
    if (res < 4) {
        tile.leftUpCoords[2] = leftUp[2];
        tile.rightDownCoords[2] = middle[2];
    } else {
        tile.leftUpCoords[2] = middle[2];
        tile.rightDownCoords[2] = rightDown[2];
    }

    return res;
}

static void insertParticle(float mass, float coords[3], std::vector<Tile> &octTree, int index, int &size, int depth = 0) {
    octTree[index].nrParticles++;
    uint thisIndex;
    Tile thisTile, otherTile;
    if (octTree[index].nrParticles == 2 && depth < 10) {
        thisIndex = getChildIndexAndData(coords, octTree[index].leftUpCoords, octTree[index].rightDownCoords, thisTile);
        auto otherIndex = getChildIndexAndData(octTree[index].coords, octTree[index].leftUpCoords, octTree[index].rightDownCoords, otherTile);

        octTree[index].children[thisIndex] = size;
        octTree[size++] = thisTile;
        insertParticle(mass, coords, octTree, size - 1, size, depth + 1);

        if (thisIndex != otherIndex) {
            octTree[index].children[otherIndex] = size;
            octTree[size++] = otherTile;
            insertParticle(octTree[index].mass, octTree[index].coords, octTree, size - 1, size, depth + 1);
        } else {
            insertParticle(octTree[index].mass, octTree[index].coords, octTree, octTree[index].children[otherIndex], size, depth + 1);
        }
    }
    
    // Update coords and mass of virtual particle
    octTree[index].coords[0] = (octTree[index].coords[0] * octTree[index].mass + coords[0] * mass) /
    (mass + octTree[index].mass);
    octTree[index].coords[1] = (octTree[index].coords[1] * octTree[index].mass + coords[1] * mass) /
    (mass + octTree[index].mass);
    octTree[index].coords[2] = (octTree[index].coords[2] * octTree[index].mass + coords[2] * mass) /
    (mass + octTree[index].mass);
    octTree[index].mass += mass;

    if (depth >= 10) {
        // At highest level of precision, act as if even virtual patricles
        // represent singular particles
        octTree[index].nrParticles = 1;
        return;
    };

    if (octTree[index].nrParticles <= 2) {
        return;
    }

    thisIndex = getChildIndexAndData(coords, octTree[index].leftUpCoords, octTree[index].rightDownCoords, thisTile);
    if (octTree[index].children[thisIndex] == (uint)-1) {
        octTree[index].children[thisIndex] = size;
        octTree[size++] = thisTile;
    }
    insertParticle(mass, coords, octTree, octTree[index].children[thisIndex], size, depth + 1);
}

void Tile::updateOctTree(const std::vector<Particle> &particles, std::vector<Tile> &octTree) {
    // In order to only have to resize initially, a "virtual" size is stored
    // here
    int size = 1;
    for (const Particle &particle: particles) {
        insertParticle(particle.mass, (float *)particle.pos, octTree, 0, size);
    }
}

[[maybe_unused]] void Tile::displayOctTree(const std::vector<Tile> &octTree, int index, int depth) {
    for (auto i = 0; i <= depth; ++i) {
        std::cout << "-";
    }
    std::cout << octTree[index].mass << ": " << octTree[index].coords[0] << ", " << octTree[index].coords[1]
    << ", " << octTree[index].coords[2] << std::endl;

    for (auto i = 0; i < 8; ++i) {
        std::cout << octTree[index].children[i] << std::endl;
        if (octTree[index].children[i] != (uint)-1) {
            displayOctTree(octTree, octTree[index].children[i], depth + 1);
        }
    }
}