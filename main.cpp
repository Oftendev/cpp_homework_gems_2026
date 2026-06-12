#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include "include/GemsGame.hpp"
#include "include/GameRenderer.hpp"

int main() {
    constexpr int WIDTH = 8;
    constexpr int HEIGHT = 8;
    constexpr int CELL_SIZE = 60;

    sf::RenderWindow window(sf::VideoMode(WIDTH * CELL_SIZE, HEIGHT * CELL_SIZE), "GEMS project");
    window.setFramerateLimit(60);
    
    GemsGame game(WIDTH, HEIGHT, CELL_SIZE);
    GameRenderer renderer(CELL_SIZE);
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            switch (event.type) {
                case sf::Event::Closed: {
                    window.close();
                    break;
                }
                case sf::Event::MouseButtonPressed: {
                    if (event.mouseButton.button == sf::Mouse::Left) {
                        game.handleClick(event.mouseButton.x, event.mouseButton.y);
                    }
                    break;
                }
                case sf::Event::KeyPressed: {
                    if (event.key.code == sf::Keyboard::Space) {
                        game.switchPause();
                    }
                    break;
                }
                default: break;
            }
        }
        // Обновлем состояние игры
        game.update();

        window.clear(sf::Color::White);
        // Запускаем отрисовку поля
        renderer.draw(window, game);
        window.display();        
    }

}