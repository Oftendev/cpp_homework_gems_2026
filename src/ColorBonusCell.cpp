#include "ColorBonusCell.hpp"

#include <algorithm>

#include "CellFactory.hpp"
#include "RandomGenerator.hpp"

ColorBonusCell::ColorBonusCell(Color bonusColor)
    : BonusCell(CellType::ColorBonus, bonusColor) {};

std::vector<sf::Vector2i> ColorBonusCell::getTargetCells(
    const sf::Vector2i& pos, int width, int height,
    const std::vector<std::vector<std::shared_ptr<Cell>>>& board) const {
    std::vector<sf::Vector2i> availableCells;
    sf::Vector2i dist;

    // Проходим по клеткам в радиусе COLORBONUS_RADIUS
    for (int i = -COLORBONUS_RADIUS; i <= COLORBONUS_RADIUS; i++) {
        for (int j = -COLORBONUS_RADIUS; j <= COLORBONUS_RADIUS; j++) {
            dist.x = pos.x + i;
            dist.y = pos.y + j;
            // Выкидываем все координаты, не помещающиеся в сетку
            if ((dist.x >= 0 && dist.x < width) &&
                (dist.y >= 0 && dist.y < height)) {
                // Выкидываем все не нормальные клетки и не соседей
                if (board[dist.y][dist.x]->type == CellType::Normal &&
                    !Cell::areNeighbors(pos, dist)) {
                    availableCells.push_back({dist.x, dist.y});
                }
            }
        }
    }

    std::mt19937& rng = RandomGenerator::getInstance().getRng();
    std::shuffle(availableCells.begin(), availableCells.end(), rng);

    int paintCount = std::min(2, static_cast<int>(availableCells.size()));
    availableCells.resize(paintCount);
    return availableCells;
}

void ColorBonusCell::execute(
    const sf::Vector2i& pos,
    std::vector<std::vector<std::shared_ptr<Cell>>>& board) {
    // 1) Превращаем саму клетку бонуса в нормальную клетку этого же цвета
    board[pos.y][pos.x] = CellFactory::createNormalCell(color);

    // 2) Получаем цели
    std::vector<sf::Vector2i> targets =
        getTargetCells(pos, board[0].size(), board.size(), board);

    // 3) Перекрашиваем цели
    for (const auto& t : targets) {
        board[t.y][t.x] = CellFactory::createNormalCell(color);
    }
}