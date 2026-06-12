#include "DestroyBonusCell.hpp"

#include <algorithm>

#include "CellFactory.hpp"
#include "RandomGenerator.hpp"

DestroyBonusCell::DestroyBonusCell()
    : BonusCell(CellType::DestroyBonus, Color::Red) {}

std::vector<sf::Vector2i> DestroyBonusCell::getTargetCells(
    int width, int height,
    const std::vector<std::vector<std::shared_ptr<Cell>>>& board) const {
    std::vector<sf::Vector2i> availableCells;
    // Проходим по всему полю
    for (int xi = 0; xi < width; xi++) {
        for (int yi = 0; yi < height; yi++) {
            if (board[yi][xi]->type == CellType::Normal) {
                availableCells.push_back({xi, yi});
            }
        }
    }

    auto& rng = RandomGenerator::getInstance().getRng();
    std::shuffle(availableCells.begin(), availableCells.end(), rng);
    int destroyCount = std::min(4, static_cast<int>(availableCells.size()));
    availableCells.resize(destroyCount);
    return availableCells;
}

void DestroyBonusCell::execute(
    const sf::Vector2i& pos,
    std::vector<std::vector<std::shared_ptr<Cell>>>& board) {
    // 1) Очищаем клетку бонуса
    board[pos.y][pos.x] = CellFactory::createEmptyCell();

    // 2) Получаем цели
    std::vector<sf::Vector2i> targets =
        getTargetCells(board[0].size(), board.size(), board);

    // 3) Очищаем цели
    for (const auto& t : targets) {
        board[t.y][t.x] = CellFactory::createEmptyCell();
    }
}