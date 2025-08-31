#include "engine.hpp"
#include "galaxy.hpp"
#include "particle.hpp"
#include "utils.hpp"
#include <cstddef>
#include <ctime>
#include <d3dcommon.h>
#include <fstream>
#include <iostream>
#include <QApplication>
#include <QWindow>
#include <d3d11.h>
#include <QTimer>
#include <qobject.h>
#include <vector>
#include "window.hpp"

constexpr int width = 1280, height = 720, msecPerFrame = 1000 / 60;

float clockToMSecs(clock_t ticks) {
    return static_cast<float>(ticks) / CLOCKS_PER_SEC * 1000;
}

int main(int argc, char **argv) {
    std::ifstream in("..//src//config.txt");
    if (in.fail()) {
        std::cerr << "Could not open config.txt file\n";
        return -1;
    }

    std::size_t NR_PARTICLES;
    float G, THETA;
    in >> NR_PARTICLES;
    in.ignore(256, '\n');
    in >> G;
    in.ignore(256, '\n');
    in >> THETA;
    in.ignore(256, '\n');

    constexpr float MASS = 1, RADIUS = 1;

    QApplication app(argc, argv);

    Window window;
    window.setWidth(width);
    window.setHeight(height);

    // GALAXY MODE
    
    // std::vector<Particle> particles;
    // Galaxy galaxy1(NR_PARTICLES / 2, width / 3, height / 2, width / 2, 40);
    // Galaxy galaxy2(NR_PARTICLES / 2, 2 * width / 3, height / 2, width / 2, 40, false);

    // for (Particle &particle: galaxy1.getStars()) {
    //     particles.push_back(particle);
    // }
    // for (Particle &particle: galaxy2.getStars()) {
    //     particles.push_back(particle);
    // }

    // FILLED SPACE MODE

    std::vector<Particle> particles(NR_PARTICLES);
    for (auto i = 0; i < NR_PARTICLES; ++i) {
        particles[i] = Particle(
            (float)get_random_int(width),
            (float)get_random_int(height),
            (float)get_random_int(width),
            MASS,
            0,
            0,
            0
        );
    }

    
    Engine engine(window, particles, G, THETA);
    
    window.show();
    engine.iterate();

    auto start = clock();
    auto end = start;

    auto oneIteration = [&](){
        if (window.activeRendering) {
            engine.iterate();

            end = clock();
            std::cout << 1000.0 / clockToMSecs(end - start) << "\n";
            start = clock();
        }
    };
    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, oneIteration);
    timer.start(msecPerFrame);

    return app.exec();
}