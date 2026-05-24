#include <iostream>
#include <cmath>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <algorithm>
#include <random>
#include <ctime>
#include <vector>

enum class CellType {
    Empty,
    Normal,
    ColorBonus,
    DestroyBonus
};

// Все доступные цвета
enum class Color{
    Red,
    Green,
    Blue,
    Yellow,
    Cyan,
    Magenta,
    Count  // количество
};

// Структура клетки
struct Cell {
    CellType type;
    Color color;
};

sf::Color convertToSFMLColor(Color c){
    switch(c) {
        case Color::Red: return sf::Color::Red;
        case Color::Green: return sf::Color::Green;
        case Color::Blue: return sf::Color::Blue;
        case Color::Yellow: return sf::Color::Yellow;
        case Color::Cyan: return sf::Color::Cyan;
        case Color::Magenta: return sf::Color::Magenta;
        default: return sf::Color::White;
    }
}

class GemsGame {
public:
    GemsGame(int w, int h, int cellSize)
        : width(w), height(h), cellSize(cellSize), firstIsSwapped(false) {
        // Инициализация генератора случайных чисел
        rng.seed(static_cast<unsigned int>(std::time(nullptr)));
        // std::srand(static_cast<unsigned int>(std::time(nullptr)));
        font.loadFromFile(PATH_TO_DATA "arial.ttf");
        board.resize(height, std::vector<Cell>(width));
        createBoard();
    }

    void createBoard() {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                board[y][x].type = CellType::Normal;
                board[y][x].color = randomColor();
            }
        }
        // Удаляем возможные начальные группы
        deleteMatchesAndDropCycle();
        currentState = GameState::WaitingForInput; // Задаём нач. состояние
    }

    // Отрисовка поля
    void draw(sf::RenderWindow& window) {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                // Создаём прямоугольники так, чтобы между ними было по 2 единицы расстояния
                sf::RectangleShape rect(sf::Vector2f(cellSize - 2, cellSize - 2));
                rect.setPosition(x * cellSize + 1, y * cellSize + 1);
                sf::Color fillColor;
                // Красим клетки в зависимости от типа
                if (board[y][x].type == CellType::Empty) {
                    fillColor = sf::Color(50, 50, 50);
                } else if (board[y][x].type == CellType::Normal) {
                    fillColor = convertToSFMLColor(board[y][x].color);
                } else if (board[y][x].type == CellType::ColorBonus) {
                    fillColor = sf::Color(240, 230, 140);  // хаки
                } else {
                    fillColor = sf::Color(0, 0, 128);  // тёмно-синий
                }
                rect.setFillColor(fillColor);
                rect.setOutlineColor(sf::Color::Black);
                rect.setOutlineThickness(-1); // Граница внутрь
                window.draw(rect);
                // Если первая выбранная клетка есть, и это - она, то затемнить
                if (firstIsSwapped && firstSwapped.x == x && firstSwapped.y == y) {
                    sf::RectangleShape selectMask(rect.getSize());
                    selectMask.setPosition(rect.getPosition());
                    // Чёрный полупрозрачный прямоугольник
                    selectMask.setFillColor(sf::Color(0, 0, 0, 100)); 
                    window.draw(selectMask);
                }                
                // Для бонусов нарисуем символы
                if (board[y][x].type == CellType::ColorBonus || board[y][x].type == CellType::DestroyBonus) {
                    sf::Text text;
                    text.setFont(font);
                    text.setCharacterSize(cellSize / 2);
                    text.setString(board[y][x].type == CellType::ColorBonus ? "C" : "B");
                    text.setFillColor(sf::Color::Black);
                    text.setPosition(x * cellSize + cellSize / 4, y * cellSize + cellSize / 4);
                    window.draw(text);                        
                }
            }
        }
    }

    void handleClick(int posX, int posY) {
        // Блокируем действия, если состояние не waitingForInput
        if (currentState != GameState::WaitingForInput) return;

        // Делим нацело для определения нажатой клетки
        int x = posX / cellSize;
        int y = posY / cellSize;

        if (x < 0 || x >= width || y < 0 || y >= height) return;
        // Проверяем, первый ли это выбор, или второй
        if(firstIsSwapped) {
            // Выбираем вторую клетку
            sf::Vector2i secondSwapped = {x, y};
            
            if (firstSwapped == secondSwapped) {
                // Нажали второй раз на выделенную ячейку -> отменили выделение
                firstIsSwapped = false;
            } else if (areNeighbors(firstSwapped, secondSwapped)) {
                // Если мы выбрали соседа
                swapCells(firstSwapped, secondSwapped);
                firstIsSwapped = false;
            } else {
                // Если кликнули на другую клетку (не соседа) - делаем новой выделенной (т.е. первой)
                firstSwapped = secondSwapped;
            }

        } else {
            // Выбираем первую клеткку
            firstSwapped = {x, y};
            firstIsSwapped = true;
        }
    }

    // Функция, которая будет вызываться каждый кадр и по прохождении DELAY будет двигать игру от состояния к состоянию
    // Т.е. мы успеем увидеть эти состояния
    void update() {
        // Игрок ещё не походил - ничего не обновляем
        if (currentState == GameState::WaitingForInput) return;

        // Время таймера ещё не >= DELAY, выходим
        if (stateClock.getElapsedTime() < DELAY) return;

        // Игрок уже походил, получив комбинацию и время таймера прошло
        switch (currentState) {
            case GameState::ShowingMatches: {
                // 1) Сохраняем инфу об удалённых клетках для PlacingBonuses и чистим поле
                deletedCellsInfo.clear();
                for (sf::Vector2i v: currentMatches) {
                    deletedCellsInfo.push_back({v, board[v.y][v.x].color});
                    board[v.y][v.x].type = CellType::Empty; // Очищаем клетку                    
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
                for (const auto& [position, color] :deletedCellsInfo) {
                    placeBonus(position, color, bonusesToActivate);
                }
                // Обнуляем индекс для следущего состояния, которое будет по нему ходить
                currentBonusIndex = 0;
                
                // Переходим к состоянию активации бонусов
                currentState = GameState::ActivatingBonuses;
                stateClock.restart();
                break;
            }
            case GameState::ActivatingBonuses: {
                // 4) Активируем бонусы по одному с паузой
                if (currentBonusIndex < bonusesToActivate.size()) {
                    // Активируем один конкретный бонус (по факту - код, который был в activateBonuses, но без цикла, т.к. update уже происходит в цикле отрисовки)
                    auto [pos, bonus] = bonusesToActivate[currentBonusIndex];
                    if (bonus.type == CellType::ColorBonus) {
                        executeColorBonus(pos, bonus.color);
                    } else if (bonus.type == CellType::DestroyBonus) {
                        executeBombBonus(pos);
                    }
                    currentBonusIndex++; // Переходим к следуюшему бонусу
                    stateClock.restart(); // Перезапускаем таймер для слежующего бонуса
                } else {
                    // Все бонусы из списка активированы -> переходим ко второму падению
                    currentState = GameState::SecondFalling;
                    stateClock.restart();
                }
                break;                
            }
            case GameState::SecondFalling: {
                // 5) Второе падение (после бонусов)
                fallAndAdd();
                
                // ОТсюда можно выйти либо к состоянию ShowingMatches (если группы ещё есть), либо к WaitingForInput (если всё ок и можно делать следующий ход)
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
            default: break;
        }
    }

private:
    enum class GameState {
        WaitingForInput, // Игрок может сделать ход
        ShowingMatches, // Нашли группы, показываем их, как уделённые ячейки (серые)
        FirstFalling, // Клетки после первого падения
        PlacingBonuses,  // Показываем поле с расставленными бонусами
        ActivatingBonuses,  // Активируем бонусы (по одному)
        SecondFalling   // Клетки после второго падения (все бонусы уже активировались)
    };
    int width, height; // Количество клеток по горизонтали и вертикали
    int cellSize;
    std::vector<std::vector<Cell>> board; // Поле клеток

    GameState currentState; // Текущее состояние игры
    sf::Clock stateClock; // Таймер для update функции
    sf::Time DELAY = sf::seconds(0.3f); // Задаём задержку между сменой состояний

    // Нужны списки для хранения данных между различными состояниями
    std::vector<sf::Vector2i> currentMatches; // Клетки в группах на данный момент
    std::vector<std::pair<sf::Vector2i, Color>> deletedCellsInfo; // Необходимая инфа об удалённых клетках для генерации бонусов
    std::vector<std::pair<sf::Vector2i, Cell>> bonusesToActivate; // Список всех созданных бонусов, которые нужно активировать
    int currentBonusIndex = 0; // Нужен для сохранения текущего для активации бонуса в списке для сотояния ActivatingBonuses
    
    

    bool firstIsSwapped; // Флаг при выборе первой клетки для swap
    sf::Vector2i firstSwapped; // Первая клетка для swap

    sf::Font font;
    std::mt19937 rng;


    const int COLORBONUS_RADIUS = 3;
    const int BONUS_PROBABILITY = 5; // в процентах

    // Генератор рандомного цвета
    Color randomColor() {
        std::uniform_int_distribution<int> dist(0, static_cast<int>(Color::Count) - 1);
        return static_cast<Color>(dist(rng));
    }
    // Генератор рандомных чисел в диапазоне
    int randomInRange(int a, int b){
        std::uniform_int_distribution<int> dist(a, b);
        return dist(rng);
    }

    // Проверка, явлеются ли 2 клетки соседями (сама с собой клетка - не сосед)
    bool areNeighbors(const sf::Vector2i& a, const sf::Vector2i& b) {
        return (std::abs(a.x - b.x) + std::abs(a.y - b.y)) == 1;
    }

    // Выполнение бонуса перекрашивания
    void executeColorBonus(const sf::Vector2i& pos, Color bonusColor){
        // Перекрашиваем клетку с бонусом
        board[pos.y][pos.x].type = CellType::Normal;
        board[pos.y][pos.x].color = bonusColor;

        std::vector<sf::Vector2i> availableCells;
        sf::Vector2i dist;
        for (int i = -COLORBONUS_RADIUS; i <= COLORBONUS_RADIUS; i++) {
            for (int j = -COLORBONUS_RADIUS; j <= COLORBONUS_RADIUS; j++) {
                dist.x = pos.x + i;
                dist.y = pos.y + j;
                // Выкидываем все координаты, не помещающиеся в сетку
                if ((dist.x >= 0 && dist.x < width) && (dist.y >= 0 && dist.y < height)) {
                    if (board[dist.y][dist.x].type == CellType::Normal && !areNeighbors(pos, dist)) {
                        // Если нормальная клетка и не сосед
                        availableCells.push_back({dist.x, dist.y});
                    }
                }
            }
        }
        // Перемешаем элементы списка
        std::shuffle(availableCells.begin(), availableCells.end(), rng);

        int paintCount = std::min(2, static_cast<int>(availableCells.size())); // Если как-то получится менее 2 возм. клеток
        // Выбираем первые 2 элемента
        for (int i = 0; i < paintCount; i++) {
            sf::Vector2i c = availableCells[i];
            board[c.y][c.x].type = CellType::Normal;
            board[c.y][c.x].color = bonusColor;
        }
    }

    // Выполнение бонуса бомбы
    void executeBombBonus(const sf::Vector2i& pos){
        // Очищаем клетку с бомбой
        board[pos.y][pos.x].type = CellType::Empty;
        
        std::vector<sf::Vector2i> availableCells;
        for (int xi = 0; xi < width; xi++) {
            for (int yi = 0; yi < height; yi++) {
                // Пропускаем саму клетку бомбы
                if (xi != pos.x || yi != pos.y){
                    if (board[yi][xi].type == CellType::Normal) {
                        availableCells.push_back({xi, yi});
                    }
                }
            }
        }

        // Перемешаем элементы списка
        std::shuffle(availableCells.begin(), availableCells.end(), rng);

        int destroyCount = std::min(4, static_cast<int>(availableCells.size()));
        // Выбираем первые 4 элемента
        for (int i = 0; i < destroyCount; i++) {
            sf::Vector2i c = availableCells[i];
            board[c.y][c.x].type = CellType::Empty;
        }
    }

    // Генерация бонуса (pos - позиция очищенной клетки)
    void placeBonus(const sf::Vector2i& pos, Color bonusColor, std::vector<std::pair<sf::Vector2i, Cell>>& bonusesToActivate){
        // Проверка вероятности
        if (randomInRange(0, 99) > BONUS_PROBABILITY) {
            return;
        }

        std::vector<sf::Vector2i> availableCells; // все допустимые для помещения бонуса клетки
        sf::Vector2i destination;
        for (int i = -3; i <= 3; i++) {
            for (int j = -3; j <= 3; j++) {
                destination.x = pos.x + i;
                destination.y = pos.y + j;
                // Выкидываем все координаты, не помещающиеся в сетку
                if ((destination.x >= 0 && destination.x < width) && (destination.y >= 0 && destination.y < height)) {
                    // Выкидываем все не нормальные клетки
                    if (board[destination.y][destination.x].type == CellType::Normal) {
                        // Если нормальная клетка
                        availableCells.push_back({destination.x, destination.y});
                    }
                }
            }
        }
        if (availableCells.size() == 0) return;
        // Выбираем один элемент из всех вариантов
        std::shuffle(availableCells.begin(), availableCells.end(), rng);
        sf::Vector2i target = availableCells[0];
        
        Cell bonusCell;
        bonusCell.type = (randomInRange(0, 1) == 0) ? CellType::ColorBonus : CellType::DestroyBonus; // Тип бонуса - 50 на 50
        bonusCell.color = bonusColor;
        
        // Сохраняем информацию о бонусе в переданном списке
        bonusesToActivate.push_back({target, bonusCell});

        // Ставим bonusCell на поле
        board[target.y][target.x] = bonusCell;
    }

    // Активация бонуса (по списку бонусов)
    void activateBonuses(const std::vector<std::pair<sf::Vector2i, Cell>>& bonuses) {
        for (const auto& [pos, bonus] : bonuses) {
            if (bonus.type == CellType::ColorBonus) {
                executeColorBonus(pos, bonus.color);
            } else if (bonus.type == CellType::DestroyBonus) {
                executeBombBonus(pos);
            }
        }
    }

    // Поиск всех клеток из групп из трёх и более одинаковых цветов (в ряд или в столбец)
    std::vector<sf::Vector2i> findGroupCells() {
        std::vector<sf::Vector2i> groupCells; // Все клетки, образующие группы
        std::vector<std::vector<bool>> grouped(height, std::vector<bool>(width, false)); // Отмечаем клетки, входящие в группы в таблице (чтобы избавиться от повторений клеток в Т подобных группах)

        // Проверяем группы по горизонтали
        for (int y = 0; y < height; y++) {
            int len = 1;
            for (int x = 1; x < width; x++) {
                if (board[y][x].type == CellType::Normal && 
                    board[y][x-1].type == CellType::Normal && 
                    board[y][x].color == board[y][x-1].color) {
                    // Уведичивем длину собираемой группы на 1
                    len++;
                } else {
                    // Собираемая группа закончилась
                    if (len >= 3) {
                        // Группа длинее 3 собралась
                        for (int i = x - len; i < x; i++) {
                            grouped[y][i] = true;
                            //groupCells.push_back({i, y});
                        }
                    }
                    len = 1;
                }
            }
            // Дошли до конца. Если группа длиннее 3 ещё собирается, записываем её.
            if (len >= 3) {
                for (int i = width - len; i < width; i++){
                    grouped[y][i] = true;
                    //groupCells.push_back({i, y});
                }
            }
        }
        
        // И аналогично группы по вертикали
        for (int x = 0; x < width; x++) {
            int len = 1;
            for (int y = 1; y < height; y++) {
                if (board[y][x].type == CellType::Normal &&
                    board[y-1][x].type == CellType::Normal &&
                    board[y][x].color == board[y-1][x].color) {
                    len++;
                } else {
                    if (len >= 3) {
                        for (int i = y - len; i < y; i++) {
                            grouped[i][x] = true;
                            //groupCells.push_back({x, i});
                        }
                    }
                    len = 1;
                }
            }
            if (len >= 3) {
                for (int i = height - len; i < height; i++) {
                    grouped[i][x] = true;
                    //groupCells.push_back({x, i});
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

    // "Схлопывание" пустых клеток, спуск непустых вниз и заполнение новыми сверху
    void fallAndAdd() {
        for (int x = 0; x < width; x++) {
            // Будем собирать по новому столбцу
            std::vector<Cell> newColumn;
            for (int y = height - 1; y >= 0; y--) {
                if (board[y][x].type != CellType::Empty) {
                    // Кладём в newColumn все клетки в столбце, кроме пустых
                    newColumn.push_back(board[y][x]);
                }
            }
            // В newcolumn докладываем рандомные (не бонусные) ячеёки
            for (int i = static_cast<int>(newColumn.size()); i < height; i++) {
                newColumn.push_back({CellType::Normal, randomColor()});
            }
            // Теперь копируем из newColunm в столбец board (но надо развернуть)
            for (int y = 0; y < height; y++){
                board[height - y - 1][x] = newColumn[y];
            }
        }
    }

    /* 
    Цикличный процесс:  поиск клеток из групп длиннее 3 -> 
                        пометка этих клеток на удаление ->
                        спуск клеток в board и добавление новых ->
                        генерация для каждой такой клетки бонуса ->
                        выполнение каждого бонуса ->
                        спуск клеток в board и добавление новых
    И так, пока больше не будет появляться групп, длиннее 3
    */
    bool deleteMatchesAndDropCycle() {
        bool has_changed = false;
        while (true) {
            // 1) Находим группы клеток
            std::vector<sf::Vector2i> matches = findGroupCells();
            if(matches.empty()) break; // Не нашли групп
            has_changed = true;

            // 2) Сохраняем эти клетки с цветами в списке (список пар (вектор, цвет)) и удаляем их с поля
            std::vector<std::pair<sf::Vector2i, Color>> deleted;
            for (sf::Vector2i p : matches) {
                deleted.push_back({p, board[p.y][p.x].color});
                board[p.y][p.x].type = CellType::Empty;                
            }

            // 3) Первое падение (бонусов пока нет) 
            fallAndAdd();

            // 4) Генерируем бонусы для каждой позиции удалённой клетки (но не активируем) (т.к. клетки до этого уже упали, то пустых мест не будет)
            std::vector<std::pair<sf::Vector2i, Cell>> bonusesToActivate;
            for (const auto& [position, color] : deleted){
                placeBonus(position, color, bonusesToActivate);
            }
            

            // 5) Активируем бонусы по списку
            activateBonuses(bonusesToActivate);

            // 6) Второе падение (уже после активации бонусов)
            fallAndAdd();

            // 7) Цикл повторяется (т.к. могли появиться новые группы)
        }
        return has_changed;
    }

    void swapCells(sf::Vector2i p1, sf::Vector2i p2) {
        std::swap(board[p1.y][p1.x], board[p2.y][p2.x]);
        
        // Вместо запуска полного цикла найдем совпадения и изменим состояние
        currentMatches = findGroupCells();

        if (!currentMatches.empty()) {
            // Если совпадения есть, то переходим в состояние Показать Совпадения
            currentState = GameState::ShowingMatches;
            stateClock.restart(); // Перезапускаем таймер
        }
    }
};

int main() {
    const int WIDTH = 8;
    const int HEIGHT = 8;
    const int CELL_SIZE = 80;
    
    sf::RenderWindow window(sf::VideoMode(WIDTH * CELL_SIZE, HEIGHT * CELL_SIZE), "GEMS project");
    window.setFramerateLimit(60);

    GemsGame game(WIDTH, HEIGHT, CELL_SIZE);
    
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    game.handleClick(event.mouseButton.x, event.mouseButton.y);
                }
            }
        }
        // Обновлем состояние игры
        game.update();

        window.clear(sf::Color::White);
        // Запускаем отрисовку поля
        game.draw(window);
        window.display();        
    }
}