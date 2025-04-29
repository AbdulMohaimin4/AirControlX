#include <SFML/Graphics.hpp>
#include <sstream>
#include <iomanip>

std::string formatTime(int totalSeconds) {
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;

    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << minutes << ":"
        << std::setw(2) << std::setfill('0') << seconds;
    return oss.str();
}

int main() {
    sf::RenderWindow window(sf::VideoMode(400, 200), "Countdown Timer");
    window.setFramerateLimit(60);

    // Font
    sf::Font font;
    if (!font.loadFromFile("fonts/tahoma.ttf")) {
        return -1; // Ensure you have arial.ttf or replace with another font file path
    }

    // Timer box
    sf::RectangleShape timerBox(sf::Vector2f(200, 80));
    timerBox.setFillColor(sf::Color(30, 30, 30));
    timerBox.setOutlineColor(sf::Color::White);
    timerBox.setOutlineThickness(3);
    timerBox.setPosition(100, 60);

    // Timer text
    sf::Text timerText;
    timerText.setFont(font);
    timerText.setCharacterSize(36);
    timerText.setFillColor(sf::Color::White);
    timerText.setPosition(140, 75);

    // Timer logic
    const int startingTime = 5 * 60; // 5 minutes in seconds
    int timeRemaining = startingTime;
    sf::Clock clock;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // Update timer
        if (timeRemaining > 0) {
            sf::Time elapsed = clock.getElapsedTime();
            if (elapsed.asSeconds() >= 1.0f) {
                timeRemaining -= static_cast<int>(elapsed.asSeconds());
                if (timeRemaining < 0) timeRemaining = 0;
                clock.restart();
            }
        }

        // Update textS
        timerText.setString(formatTime(timeRemaining));

        // Render
        window.clear(sf::Color::Black);
        window.draw(timerBox);
        window.draw(timerText);
        window.display();
    }

    return 0;
}
