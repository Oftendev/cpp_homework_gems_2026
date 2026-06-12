#include "GemsGame.hpp"

#include <algorithm>
#include <queue>

#include "CellFactory.hpp"
#include "ColorBonusCell.hpp"
#include "DestroyBonusCell.hpp"
#include "RandomGenerator.hpp"

GemsGame::GemsGame(int w, int h, int cellSize)
    : width(w), height(h), cellSize(cellSize), firstIsSwapped(false) {
    board.resize(height, std::vector<std::shared_ptr<Cell>>(width));
    createBoard();
}

void GemsGame::createBoard() {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            board[y][x] = CellFactory::createNormalCell(
                RandomGenerator::getInstance().randomColor());
        }
    }
    deleteMatchesAndDropCycle();
    currentState = GameState::WaitingForInput;
}

bool GemsGame::isCellSelected(int x, int y) const {
    return firstIsSwapped && firstSwapped.x == x && firstSwapped.y == y;
}

void GemsGame::switchPause() {
    isPaused = !isPaused;
    if (!isPaused) {
        stateClock.restart();  // Чтобы после паузы время сбросилось
    }
}

void GemsGame::handleClick(int posX, int posY) {
    // Блокируем действия, еслисостояние не waitingForInput
    if (currentState != GameState::WaitingForInput) return;
    // Блокируем действия, если играна паузе
    if (isPaused) return;
    // Делим нацело для определениянажатой клетки
    int x = posX / cellSize;
    int y = posY / cellSize;
    if (x < 0 || x >= width || y < 0 || y >= height) return;
    // Проверяем, первый ли этовыбор, или второй
    if (firstIsSwapped) {
        // Выбираем вторую клетку
        sf::Vector2i secondSwapped = {x, y};

        if (firstSwapped == secondSwapped) {
            // Нажали второй раз навыделенную ячейку ->отменили выделение
            firstIsSwapped = false;
        } else if (Cell::areNeighbors(firstSwapped, secondSwapped)) {
            // Если мы выбрали соседа
            swapCells(firstSwapped, secondSwapped);
            firstIsSwapped = false;
        } else {
            // Если кликнули надругую клетку (не соседа)- делаем новой
            // выделенной(т.е. первой)
            firstSwapped = secondSwapped;
        }
    } else {
        // Выбираем первую клеткку
        firstSwapped = {x, y};
        firstIsSwapped = true;
    }
}

void GemsGame::swapCells(sf::Vector2i p1, sf::Vector2i p2) {
    std::swap(board[p1.y][p1.x], board[p2.y][p2.x]);

    // Вместо запуска полного цикла найдем совпадения и изменим состояние
    currentMatches = groupFinder.findGroupCells(board, width, height);
    if (!currentMatches.empty()) {
        // Если совпадения есть, то переходим в состояние Показать Совпадения
        currentState = GameState::ShowingMatches;
        stateClock.restart();  // Перезапускаем таймер
    } else {
        // Совпадений нет - возвращаем клетки обратно
        std::swap(board[p1.y][p1.x], board[p2.y][p2.x]);
        // Очищаем список совпадений
        currentMatches.clear();
        // Состояние остаётся WaitingForInput
    }
}

void GemsGame::update() {
    // Игрок ещё не походил - ничего не обновляем
    if (currentState == GameState::WaitingForInput) return;

    // Игра на паузе - также ничего не делаем
    if (isPaused) return;

    // Время таймера ещё не >= DELAY, выходим
    if (stateClock.getElapsedTime().asSeconds() < DELAY_SECONDS) return;

    // Игрок уже походил, получив комбинацию и время таймера прошло
    switch (currentState) {
        case GameState::ShowingMatches: {
            // 1) Сохраняем инфу об удалённых клетках для PlacingBonuses и
            // чистим поле
            deletedCellsInfo.clear();
            for (sf::Vector2i v : currentMatches) {
                deletedCellsInfo.push_back({v, board[v.y][v.x]->color});
                board[v.y][v.x] =
                    CellFactory::createEmptyCell();  // Очищаем клетку
            }

            // Переходим к следующему состоянию - FirstFalling
            currentState = GameState::FirstFalling;
            // Перезапускаем таймер
            stateClock.restart();
            break;
        }
        case GameState::FirstFalling: {
            // 2) Клетки "упали" и насыпались новые сверху
            fallHandler.fallAndAdd(board, width, height);

            // Переходим к состоянию генерации бонусов
            currentState = GameState::PlacingBonuses;
            stateClock.restart();
            break;
        }
        case GameState::PlacingBonuses: {
            // 3) Расставляем на упавшем поле бонусы
            bonusesToActivate.clear();
            for (const auto& [position, color] : deletedCellsInfo) {
                BonusManager.placeBonus(position, color, board, width, height,
                                        bonusesToActivate);
            }
            // Обнуляем индекс для следущего состояния, которое будет по нему
            // ходить
            currentBonusIndex = 0;

            // Переходим к состоянию активации бонусов
            currentState = GameState::ActivatingBonuses;
            stateClock.restart();
            break;
        }
        case GameState::ActivatingBonuses: {
            // 4) Активируем бонусы по одному с паузой
            if (currentBonusIndex < bonusesToActivate.size()) {
                // Активируем один конкретный бонус (по факту - код, который был
                // в activateBonuses, но без цикла, т.к. update уже происходит в
                // цикле отрисовки)
                auto& [pos, bonus] = bonusesToActivate[currentBonusIndex];
                BonusManager.executeBonus(pos, bonus, board, width, height);
                currentBonusIndex++;  // Переходим к следуюшему бонусу
                stateClock
                    .restart();  // Перезапускаем таймер для слежующего бонуса
            } else {
                // Все бонусы из списка активированы -> переходим ко второму
                // падению
                currentState = GameState::SecondFalling;
                stateClock.restart();
            }
            break;
        }
        case GameState::SecondFalling: {
            // 5) Второе падение (после бонусов)
            fallHandler.fallAndAdd(board, width, height);

            // ОТсюда можно выйти либо к состоянию ShowingMatches (если группы
            // ещё есть), либо к WaitingForInput (если всё ок и можно делать
            // следующий ход)
            currentMatches = groupFinder.findGroupCells(board, width, height);
            if (!currentMatches.empty()) {
                // Нашли новые клетки в группах - запускаем цикл заново
                currentState = GameState::ShowingMatches;
            } else {
                // Если новы групп нет - даём ход игроку
                currentState = GameState::WaitingForInput;
            }
            stateClock.restart();
            break;
        }
        default:
            break;
    }
}

bool GemsGame::deleteMatchesAndDropCycle() {
    bool has_changed = false;
    while (true) {
        // 1) Находим группы клеток
        std::vector<sf::Vector2i> matches =
            groupFinder.findGroupCells(board, width, height);
        if (matches.empty()) break;  // Не нашли групп
        has_changed = true;
        // 2) Сохраняем эти клетки с цветами в списке (список пар (вектор,
        // цвет)) и удаляем их с поля
        std::vector<std::pair<sf::Vector2i, Color>> deleted;
        for (sf::Vector2i p : matches) {
            deleted.push_back({p, board[p.y][p.x]->color});
            board[p.y][p.x] = CellFactory::createEmptyCell();
        }
        // 3) Первое падение (бонусов пока нет)
        fallHandler.fallAndAdd(board, width, height);
        // 4) Генерируем бонусы для каждой позиции удалённой клетки (но не
        // активируем) (т.к. клетки до этого уже упали, то пустых мест не будет)
        // std::vector<std::pair<sf::Vector2i, std::shared_ptr<Cell>>>
        // tempBonusesToActivate;
        for (const auto& [position, color] : deleted) {
            BonusManager.placeBonus(position, color, board, width, height,
                                    bonusesToActivate);
        }

        // 5) Активируем бонусы по списку
        for (const auto& [pos, bonus] : bonusesToActivate) {
            BonusManager.executeBonus(pos, bonus, board, width, height);
        }
        // 6) Второе падение (уже после активации бонусов)
        fallHandler.fallAndAdd(board, width, height);
        // 7) Цикл повторяется (т.к. могли появиться новые группы)
        bonusesToActivate.clear();  // Очищаем список бонусов
    }
    return has_changed;
}