#ifndef SIMULATION_HPP
#define SIMULATION_HPP

#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>

class Simulation {

private:
    sf::RenderWindow* window; // pointer for dynamic window
    sf::VideoMode videoMode; 
    sf::Event ev;

    void initVariables();
    void initWindow();

public:
    Simulation();
    ~Simulation();

    void update();
    void render();
    const bool running();

};

#endif