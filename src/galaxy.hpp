#pragma once

#include "particle.hpp"
#include "utils.hpp"
#include <cmath>
#include <utility>
#include <vector>
class Galaxy {
    public:
        constexpr static float VEL_SCALING = 125, DEF_SIGMA = 1;
        Galaxy(int size, float x, float y, float z, float radius, float z_flat = true) {
            stars.resize(size);

            if (z_flat) {
                for (Particle &particle: stars) {
                    particle = Particle(
                        get_random_float_normal(x, radius / 2),
                        get_random_float_normal(y, radius / 2),
                        get_random_float_normal(z, DEF_SIGMA),
                        1, 0, 0, 0);

                    // Clockwise rotation
                        particle.vel[1] = -(particle.pos[0] - x) / VEL_SCALING;
                        particle.vel[0] = (particle.pos[1] - y) / VEL_SCALING;
                }
            } else {
                for (Particle &particle: stars) {
                    particle = Particle(
                        get_random_float_normal(x, radius / 2),
                        get_random_float_normal(y, DEF_SIGMA),
                        get_random_float_normal(z, radius / 2),
                        1, 0, 0, 0);

                    // Clockwise rotation
                        particle.vel[2] = -(particle.pos[0] - x) / VEL_SCALING;
                        particle.vel[0] = (particle.pos[2] - z) / VEL_SCALING;
                }
            }
        }

        std::vector<Particle>& getStars() {
            return stars;
        }

    private:
        std::vector<Particle> stars;
};