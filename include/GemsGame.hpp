#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

#include "Cell.hpp"

class GemsGame {
   public:
    GemsGame(int w, int h, int cellSize);
    void createBoard();
    void handleClick(int posX, int posY);
    void update();
    void switchPause();

    // Геттеры для рендерера
    const std::vector<std::vector<std::shared_ptr<Cell>>>& getBoard() const {
        return board;
    }
    bool isCellSelected(int posX, int posY) const;
    bool isPausedState() const { return isPaused; };
    int getCellSize() const { return cellSize; };
    int getWidth() const { return width; }
    int getHeight() const { return height; }

   private:
    enum class GameState {
        WaitingForInput,  // Игрок может сделать ход
        ShowingMatches,  // Нашли группы, показываем их, как уделённые ячейки
                         // (серые)
        FirstFalling,       // Клетки после первого падения
        PlacingBonuses,     // Показываем поле с расставленными бонусами
        ActivatingBonuses,  // Активируем бонусы (по одному)
        SecondFalling  // Клетки после второго падения (все бонусы уже
                       // активировались)
    };
    int width, height;  // Количество клеток по горизонтали и вертикали
    int cellSize;
    std::vector<std::vector<std::shared_ptr<Cell>>> board;  // Поле клеток

    GameState currentState;  // Текущее состояние игры
    sf::Clock stateClock;    // Таймер для update функции
    static constexpr float DELAY_SECONDS =
        0.3f;  // Задаём задержку между сменой состояний

    // Нужны списки для хранения данных между различными состояниями
    std::vector<sf::Vector2i>
        currentMatches;  // Клетки в группах на данный момент
    std::vector<std::pair<sf::Vector2i, Color>>
        deletedCellsInfo;  // Необходимая инфа об удалённых клетках для
                           // генерации бонусов
    std::vector<std::pair<sf::Vector2i, std::shared_ptr<Cell>>>
        bonusesToActivate;  // Список всех созданных бонусов, которые нужно
                            // активировать
    int currentBonusIndex =
        0;  // Нужен для сохранения текущего для активации бонуса в списке для
            // сотояния ActivatingBonuses

    bool isPaused = false;  // Перемеенна для паузы

    bool firstIsSwapped = false;  // Флаг при выборе первой клетки для swap
    sf::Vector2i firstSwapped;    // Первая клетка для swap
    static constexpr int BONUS_PROBABILITY = 5;

    // Всомогательные методы
    // Выполение бонуса
    void executeBonus(const sf::Vector2i& pos, std::shared_ptr<Cell> bonus);
    // Генерация бонуса (pos - позиция очищенной клетки)
    void placeBonus(const sf::Vector2i& pos, Color bonusColor);
    // Поиск всех клеток из групп из трёх и более одинаковых цветов (в ряд или в
    // столбец)
    std::vector<sf::Vector2i> findGroupCells();
    // "Схлопывание" пустых клеток, спуск непустых вниз и заполнение новыми
    // сверху
    void fallAndAdd();
    /*
    Цикличный процесс:  поиск клеток из групп длиннее 3 ->
                        пометка этих клеток на удаление ->
                        спуск клеток в board и добавление новых ->
                        генерация для каждой такой клетки бонуса ->
                        выполнение каждого бонуса ->
                        спуск клеток в board и добавление новых
    И так, пока больше не будет появляться групп, длиннее 3
    */
    bool deleteMatchesAndDropCycle();
    void swapCells(sf::Vector2i p1, sf::Vector2i p2);
};
