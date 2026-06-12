#include "CellFactory.hpp"

#include "RandomGenerator.hpp"

std::shared_ptr<Cell> CellFactory::createNormalCell(Color color) {
    return std::make_shared<NormalCell>(color);
}

std::shared_ptr<Cell> CellFactory::createEmptyCell() {
    return std::make_shared<Cell>();
}

std::shared_ptr<Cell> CellFactory::createRandomBonus(Color color) {
    if (RandomGenerator::getInstance().randomInRange(0, 1) == 0) {
        return std::make_shared<ColorBonusCell>(color);
    } else {
        return std::make_shared<DestroyBonusCell>();
    }
}
