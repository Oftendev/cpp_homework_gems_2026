#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <string>

enum class CellType { Empty, Normal, ColorBonus, DestroyBonus };

// Все доступные цвета
enum class Color {
    Red,
    Green,
    Blue,
    Yellow,
    Cyan,
    Magenta,
    Count  // количество
};

// Абстрактный класс клетки
class Cell {
   public:
    CellType type;
    Color color;

    Cell() : type(CellType::Empty), color(Color::Red) {}

    virtual bool isEmpty() const { return type == CellType::Empty; }
    virtual bool isBonus() const { return false; }
    virtual sf::Color getFillColor() const;  // Реализация в Cell.cpp
    virtual std::string getSymbol() const { return ""; }
    static bool areNeighbors(const sf::Vector2i& a, const sf::Vector2i& b);
    virtual ~Cell() = default;
};

// 2 класса для нормальных и бонусных клеток
class NormalCell : public Cell {
   public:
    NormalCell(Color c);
};

class BonusCell : public Cell {
   public:
    BonusCell(CellType bonusType, Color bonusColor);
    virtual bool isBonus() const override { return true; }
    virtual sf::Color getFillColor() const override;
    // Активирует бонус: изменяет клетку с бонусом и цели
    virtual void execute(
        const sf::Vector2i& pos,
        std::vector<std::vector<std::shared_ptr<Cell>>>& board) = 0;
};
