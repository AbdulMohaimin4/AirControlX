#include "simulation.hpp"

// method definitions

void Simulation::initVariables() {

    this->window = nullptr;
}

void Simulation::initWindow() {

    this->videoMode.height = 600;
    this->videoMode.width = 800;
    this->window = new sf::RenderWindow(this->videoMode, "AirControlX", sf::Style::Titlebar | sf::Style::Close);
}

// constructor/destructor

Simulation::Simulation() {

    this->initVariables();
    this->initWindow();
}

Simulation::~Simulation() {

    delete this->window;
}

void Simulation::update() {


}

void Simulation::render() {


}

const bool Simulation::running() {

    return this->window->isOpen();
}
