#include "Cell.hpp"

sf::Color convertToSFMLColor(Color c) {
    switch (c) {
        case Color::Red:
            return sf::Color::Red;
        case Color::Green:
            return sf::Color::Green;
        case Color::Blue:
            return sf::Color::Blue;
        case Color::Yellow:
            return sf::Color::Yellow;
        case Color::Cyan:
            return sf::Color::Cyan;
        case Color::Magenta:
            return sf::Color::Magenta;
        default:
            return sf::Color::White;
    }
}

// Реализация метода getFillColor класа-родителя Cell
sf::Color Cell::getFillColor() const {
    switch (type) {
        case CellType::Empty: {
            return sf::Color(50, 50, 50);
        };
        case CellType::Normal: {
            return convertToSFMLColor(color);
        };
        default:
            return sf::Color::White;
    }
}

// Реализуем стстический метод проверки соседства
bool Cell::areNeighbors(const sf::Vector2i& a, const sf::Vector2i& b) {
    return (std::abs(a.x - b.x) + std::abs(a.y - b.y)) == 1;
}

// Реализация конструктора NormalCell
NormalCell::NormalCell(Color c) {
    type = CellType::Normal;
    color = c;
}

// Реализация конструктора BonusCell
BonusCell::BonusCell(CellType bonusType, Color bonusColor) {
    type = bonusType;
    color = bonusColor;
}

// Цвет для BonusCell
sf::Color BonusCell::getFillColor() const {
    if (type == CellType::ColorBonus) {
        return sf::Color(240, 230, 140);  // хаки
    } else {
        return sf::Color(0, 0, 128);  // тёмно-синий
    }
}