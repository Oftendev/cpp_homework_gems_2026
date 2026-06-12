#include "GemsGame.hpp"

#include <algorithm>

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
    currentMatches = findGroupCells();
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

std::vector<sf::Vector2i> GemsGame::findGroupCells() {
    std::vector<sf::Vector2i> groupCells;  // Все клетки, образующие группы
    std::vector<std::vector<bool>> grouped(height,
                                           std::vector<bool>(width, false));

    // // Проверяем группы по горизонтали
    for (int y = 0; y < height; y++) {
        int len = 1;
        for (int x = 1; x < width; x++) {
            if (board[y][x]->type == CellType::Normal &&
                board[y][x - 1]->type == CellType::Normal &&
                board[y][x]->color == board[y][x - 1]->color) {
                // Уведичивем длину собираемой группы на 1
                len++;
            } else {
                // Собираемая группа закончилась
                if (len >= 3) {
                    // Группа длинее 3 собралась
                    for (int i = x - len; i < x; i++) {
                        grouped[y][i] = true;
                    }
                }
                len = 1;
            }
        }
        // Дошли до конца. Если группа длиннее 3 ещё собирается, записываем её.
        if (len >= 3) {
            for (int i = width - len; i < width; i++) {
                grouped[y][i] = true;
            }
        }
    }

    // // И аналогично группы по вертикали
    for (int x = 0; x < width; x++) {
        int len = 1;
        for (int y = 1; y < height; y++) {
            if (board[y][x]->type == CellType::Normal &&
                board[y - 1][x]->type == CellType::Normal &&
                board[y][x]->color == board[y - 1][x]->color) {
                len++;
            } else {
                if (len >= 3) {
                    for (int i = y - len; i < y; i++) {
                        grouped[i][x] = true;
                    }
                }
                len = 1;
            }
        }
        if (len >= 3) {
            for (int i = height - len; i < height; i++) {
                grouped[i][x] = true;
            }
        }
    }
    // Проходимся циклом по всем помеченным клеткам
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (grouped[y][x]) {
                groupCells.push_back({x, y});
            }
        }
    }

    return groupCells;
}

void GemsGame::fallAndAdd() {
    for (int x = 0; x < width; x++) {
        // Будем собирать по новому столбцу
        std::vector<std::shared_ptr<Cell>> newColumn;

        for (int y = height - 1; y >= 0; y--) {
            if (!board[y][x]->isEmpty()) {
                // Кладём в newColumn все клетки в столбце, кроме пустых
                newColumn.push_back(board[y][x]);
            }
        }
        // В newcolumn докладываем рандомные (не бонусные) ячеёки
        for (int i = static_cast<int>(newColumn.size()); i < height; i++) {
            newColumn.push_back(CellFactory::createNormalCell(
                RandomGenerator::getInstance().randomColor()));
        }
        // Теперь копируем из newColunm в столбец board (но надо развернуть)
        for (int y = 0; y < height; y++) {
            board[height - y - 1][x] = newColumn[y];
        }
    }
}

void GemsGame::placeBonus(const sf::Vector2i& pos, Color bonusColor) {
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

void GemsGame::executeBonus(const sf::Vector2i& pos,
                            std::shared_ptr<Cell> bonus) {
    if (!bonus || !bonus->isBonus()) return;
    // Вызываем полиморфный execute, который знает, что делать
    std::static_pointer_cast<BonusCell>(bonus)->execute(pos, board);
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
            fallAndAdd();

            // Переходим к состоянию генерации бонусов
            currentState = GameState::PlacingBonuses;
            stateClock.restart();
            break;
        }
        case GameState::PlacingBonuses: {
            // 3) Расставляем на упавшем поле бонусы
            bonusesToActivate.clear();
            for (const auto& [position, color] : deletedCellsInfo) {
                placeBonus(position, color);
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
                executeBonus(pos, bonus);
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
            fallAndAdd();

            // ОТсюда можно выйти либо к состоянию ShowingMatches (если группы
            // ещё есть), либо к WaitingForInput (если всё ок и можно делать
            // следующий ход)
            currentMatches = findGroupCells();
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
        std::vector<sf::Vector2i> matches = findGroupCells();
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
        fallAndAdd();
        // 4) Генерируем бонусы для каждой позиции удалённой клетки (но не
        // активируем) (т.к. клетки до этого уже упали, то пустых мест не будет)
        // std::vector<std::pair<sf::Vector2i, std::shared_ptr<Cell>>>
        // tempBonusesToActivate;
        for (const auto& [position, color] : deleted) {
            placeBonus(position, color);
        }

        // 5) Активируем бонусы по списку
        for (const auto& [pos, bonus] : bonusesToActivate) {
            executeBonus(pos, bonus);
        }
        // 6) Второе падение (уже после активации бонусов)
        fallAndAdd();
        // 7) Цикл повторяется (т.к. могли появиться новые группы)
        bonusesToActivate.clear();  // Очищаем список бонусов
    }
    return has_changed;
}