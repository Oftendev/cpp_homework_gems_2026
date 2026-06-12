#include "BonusManager.hpp"

#include <algorithm>

#include "CellFactory.hpp"
#include "RandomGenerator.hpp"

void BonusManager::placeBonus(
    const sf::Vector2i& pos, Color bonusColor,
    std::vector<std::vector<std::shared_ptr<Cell>>>& board, int width,
    int height,
    std::vector<std::pair<sf::Vector2i, std::shared_ptr<Cell>>>&
        bonusesToActivate) {
    if (RandomGenerator::getInstance().randomInRange(0, 99) >
        BONUS_PROBABILITY) {
        return;
    }

    std::vector<sf::Vector2i> availableCells;
    sf::Vector2i destination;

    for (int i = -3; i <= 3; i++) {
        for (int j = -3; j <= 3; j++) {
            destination.x = pos.x + i;
            destination.y = pos.y + j;

            if ((destination.x >= 0 && destination.x < width) &&
                (destination.y >= 0 && destination.y < height)) {
                if (board[destination.y][destination.x]->type ==
                    CellType::Normal) {
                    availableCells.push_back(destination);
                }
            }
        }
    }

    if (availableCells.empty()) return;

    std::mt19937& rng = RandomGenerator::getInstance().getRng();
    std::shuffle(availableCells.begin(), availableCells.end(), rng);
    sf::Vector2i target = availableCells[0];

    std::shared_ptr<Cell> bonusCell =
        CellFactory::createRandomBonus(bonusColor);
    // Сохраняем указатель на объект клетки для его активации
    bonusesToActivate.push_back({target, bonusCell});
    board[target.y][target.x] = bonusCell;
}

void BonusManager::executeBonus(
    const sf::Vector2i& pos, std::shared_ptr<Cell> bonus,
    std::vector<std::vector<std::shared_ptr<Cell>>>& board, int width,
    int height) {
    if (!bonus || !bonus->isBonus()) return;
    // Вызываем полиморфный execute, который знает, что делать
    std::static_pointer_cast<BonusCell>(bonus)->execute(pos, board);
}