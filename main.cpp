#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

using namespace sf;

RenderWindow * window = NULL;

int main() {

    window = new RenderWindow(VideoMode(800, 600), "Test");

    window->setFramerateLimit(60);
    
    while (window->isOpen()) {
        sf::Event event;
        while (window->pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window->close();
            }
        }

        window->clear(Color::Red);

        window->display();
    }

    delete window;

    return 0;
}