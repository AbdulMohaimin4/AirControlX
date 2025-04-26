#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

int main()
{
    sf::RenderWindow window(sf::VideoMode(800, 600), "SFML Drop-Down Example");

    // Font for the text
    sf::Font font;
    if (!font.loadFromFile("tahoma.ttf")) {
        return -1; // Load a font from your project folder
    }

    // Drop-down main box
    sf::RectangleShape dropdownBox(sf::Vector2f(200.f, 40.f));
    dropdownBox.setPosition(300.f, 100.f);
    dropdownBox.setFillColor(sf::Color(200, 200, 200));

    // Placeholder text
    sf::Text selectedText("Select Option", font, 20);
    selectedText.setPosition(dropdownBox.getPosition().x + 10, dropdownBox.getPosition().y + 5);
    selectedText.setFillColor(sf::Color::Black);

    // Options
    std::vector<std::string> options = { "Option 1", "Option 2", "Option 3" };
    std::vector<sf::Text> optionTexts;
    for (size_t i = 0; i < options.size(); ++i) {
        sf::Text text(options[i], font, 20);
        text.setPosition(dropdownBox.getPosition().x + 10, dropdownBox.getPosition().y + 45 + i * 35);
        text.setFillColor(sf::Color::Black);
        optionTexts.push_back(text);
    }

    bool isExpanded = false;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed) {
                sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);

                // Clicked on dropdown main box
                if (dropdownBox.getGlobalBounds().contains(mousePos)) {
                    isExpanded = !isExpanded;
                }
                // Clicked on an option
                else if (isExpanded) {
                    for (size_t i = 0; i < optionTexts.size(); ++i) {
                        if (optionTexts[i].getGlobalBounds().contains(mousePos)) {
                            selectedText.setString(options[i]);
                            isExpanded = false;
                        }
                    }
                }
            }
        }

        window.clear(sf::Color::White);
        
        window.draw(dropdownBox);
        window.draw(selectedText);

        if (isExpanded) {
            for (const auto& text : optionTexts) {
                // Draw background rectangles behind each option (optional)
                sf::RectangleShape optionBg(sf::Vector2f(200.f, 30.f));
                optionBg.setPosition(text.getPosition().x - 10, text.getPosition().y - 5);
                optionBg.setFillColor(sf::Color(220, 220, 220));
                window.draw(optionBg);

                window.draw(text);
            }
        }

        window.display();
    }

    return 0;
}
