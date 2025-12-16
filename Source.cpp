#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <vector>
#include <queue>
#include <map>
#include <string>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <optional>
#include <cmath>

using namespace std;
struct ScoreNode {
    int score;
    int level;
    ScoreNode* next;

    ScoreNode(int s, int l) : score(s), level(l), next(nullptr) {}
};

class ScoreHistory {
private:
    ScoreNode* head;
    int count;

public:
    ScoreHistory() : head(nullptr), count(0) {}

    void addScore(int score, int level) {
        ScoreNode* newNode = new ScoreNode(score, level);
        newNode->next = head;
        head = newNode;
        count++;
    }

    int getHighScore() {
        int maxScore = 0;
        ScoreNode* current = head;
        while (current != nullptr) {
            if (current->score > maxScore) {
                maxScore = current->score;
            }
            current = current->next;
        }
        return maxScore;
    }

    int getCount() const { return count; }

    ~ScoreHistory() {
        while (head != nullptr) {
            ScoreNode* temp = head;
            head = head->next;
            delete temp;
        }
    }
};

struct MenuNode {
    string text;
    int action;
    MenuNode* left;
    MenuNode* right;

    MenuNode(const string& t, int a) : text(t), action(a), left(nullptr), right(nullptr) {}
};

class MenuTree {
private:
    MenuNode* root;

    MenuNode* insert(MenuNode* node, const string& text, int action) {
        if (node == nullptr) {
            return new MenuNode(text, action);
        }

        if (action < node->action) {
            node->left = insert(node->left, text, action);
        }
        else {
            node->right = insert(node->right, text, action);
        }
        return node;
    }

    void inOrder(MenuNode* node, vector<string>& items) {
        if (node != nullptr) {
            inOrder(node->left, items);
            items.push_back(node->text);
            inOrder(node->right, items);
        }
    }

public:
    MenuTree() : root(nullptr) {
        root = insert(root, "Play Game", 0);
        insert(root, "Quit", 1);
    }

    vector<string> getMenuItems() {
        vector<string> items;
        inOrder(root, items);
        return items;
    }

    ~MenuTree() {
    }
};

enum CellType {
    WALL = '#',
    EMPTY = ' ',
    FOOD = '.',
    POWER_PELLET = 'O'
};

enum GameState {
    MENU,
    PLAYING,
    GAME_OVER,
    VICTORY,
    PAUSED
};

class Maze {
private:
    vector<vector<char>> grid;
    int rows, cols;
    int foodCount;

public:
    Maze() : rows(0), cols(0), foodCount(0) {
        initializeDefaultMaze();
    }

    void initializeDefaultMaze() {
        rows = 31;
        cols = 28;
        grid.resize(rows, vector<char>(cols, WALL));

        vector<string> defaultMaze = {
             "############################",
             "#............##............#",
             "#.####.#####.##.#####.####.#",
             "#O####.#####.##.#####.####O#",
             "#.####.#####.##.#####.####.#",
             "#..........................#",
             "#.####.##.########.##.####.#",
             "#.####.##.########.##.####.#",
             "#......##....##....##......#",
             "######.#####.##.#####.######",
             "     #.#####.##.#####.#     ",
             "     #.##..........##.#     ",
             "     #.##.###  ###.##.#     ",
             "######.##.#      #.##.######",
             "      ....#      #....      ",
             "######.##.#      #.##.######",
             "     #.##.########.##.#     ",
             "     #.##..........##.#     ",
             "     #.##.########.##.#     ",
             "######.##.########.##.######",
             "#............##............#",
             "#.####.#####.##.#####.####.#",
             "#.####.#####.##.#####.####.#",
             "#O..##................##..O#",
             "###.##.##.########.##.##.###",
             "#......##....##....##......#",
             "#.##########.##.##########.#",
             "#.##########.##.##########.#",
             "#..........................#",
             "#.##########.##.##########.#",
             "############################"
        };

        foodCount = 0;
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols && j < (int)defaultMaze[i].length(); j++) {
                char cell = defaultMaze[i][j];
                if (cell == ' ' || cell == '.') {
                    grid[i][j] = FOOD;
                    foodCount++;
                }
                else if (cell == 'O') {
                    grid[i][j] = POWER_PELLET;
                    foodCount++;
                }
                else {
                    grid[i][j] = WALL;
                }
            }
        }
    }

    char getCell(int row, int col) const {
        if (row < 0 || row >= rows || col < 0 || col >= cols) return WALL;
        return grid[row][col];
    }

    void setCell(int row, int col, char value) {
        if (row >= 0 && row < rows && col >= 0 && col < cols) {
            grid[row][col] = value;
        }
    }

    bool isValidPosition(int row, int col) const {
        return row >= 0 && row < rows && col >= 0 && col < cols;
    }

    bool isWalkable(int row, int col) const {
        if (!isValidPosition(row, col)) return false;
        return grid[row][col] != WALL;
    }

    vector<pair<int, int>> getNeighbors(int row, int col) const {
        vector<pair<int, int>> neighbors;
        int directions[4][2] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };

        for (int i = 0; i < 4; i++) {
            int newRow = row + directions[i][0];
            int newCol = col + directions[i][1];

            if (newRow < 0) newRow = rows - 1;
            if (newRow >= rows) newRow = 0;
            if (newCol < 0) newCol = cols - 1;
            if (newCol >= cols) newCol = 0;

            if (isWalkable(newRow, newCol)) {
                neighbors.push_back({ newRow, newCol });
            }
        }

        return neighbors;
    }

    int getRows() const { return rows; }
    int getCols() const { return cols; }
    int getFoodCount() const { return foodCount; }
    void decrementFoodCount() { foodCount--; }
};

class Player {
private:
    int row, col;
    int score;
    int lives;
    char direction;
    bool poweredUp;
    int powerUpTimer;

public:
    Player(int startRow = 23, int startCol = 14)
        : row(startRow), col(startCol), score(0), lives(3),
        direction('R'), poweredUp(false), powerUpTimer(0) {
    }

    void move(char dir, const Maze& maze) {
        direction = dir;
        int newRow = row;
        int newCol = col;

        switch (dir) {
        case 'U': newRow--; break;
        case 'D': newRow++; break;
        case 'L': newCol--; break;
        case 'R': newCol++; break;
        }

        if (newRow < 0) newRow = maze.getRows() - 1;
        if (newRow >= maze.getRows()) newRow = 0;
        if (newCol < 0) newCol = maze.getCols() - 1;
        if (newCol >= maze.getCols()) newCol = 0;

        if (maze.isWalkable(newRow, newCol)) {
            row = newRow;
            col = newCol;
        }
    }

    pair<int, int> getPosition() const { return { row, col }; }
    int getRow() const { return row; }
    int getCol() const { return col; }
    int getScore() const { return score; }
    int getLives() const { return lives; }
    char getDirection() const { return direction; }
    bool isPoweredUp() const { return poweredUp; }

    void addScore(int points) { score += points; }
    void loseLife() { lives--; }

    void activatePowerUp() {
        poweredUp = true;
        powerUpTimer = 300;
    }

    void updatePowerUp() {
        if (poweredUp) {
            powerUpTimer--;
            if (powerUpTimer <= 0) {
                poweredUp = false;
            }
        }
    }

    void reset(int startRow, int startCol) {
        row = startRow;
        col = startCol;
        direction = 'R';
        poweredUp = false;
        powerUpTimer = 0;
    }
};

class Ghost {
private:
    int row, col;
    int startRow, startCol;
    char color;
    bool isScared;
    int scaredTimer;

    vector<pair<int, int>> findPathBFS(int targetRow, int targetCol, const Maze& maze) const {
        queue<pair<int, int>> q;
        map<pair<int, int>, pair<int, int>> parent;
        map<pair<int, int>, bool> visited;

        q.push({ row, col });
        visited[{row, col}] = true;
        parent[{row, col}] = { -1, -1 };

        while (!q.empty()) {
            auto current = q.front();
            q.pop();

            if (current.first == targetRow && current.second == targetCol) {
                vector<pair<int, int>> path;
                auto node = current;
                while (parent[node].first != -1) {
                    path.push_back(node);
                    node = parent[node];
                }
                reverse(path.begin(), path.end());
                return path;
            }

            auto neighbors = maze.getNeighbors(current.first, current.second);
            for (const auto& neighbor : neighbors) {
                if (!visited[neighbor]) {
                    visited[neighbor] = true;
                    parent[neighbor] = current;
                    q.push(neighbor);
                }
            }
        }

        return {};
    }

    pair<int, int> getNextMove(int targetRow, int targetCol, const Maze& maze) const {
        auto path = findPathBFS(targetRow, targetCol, maze);
        if (!path.empty()) {
            return path[0];
        }
        auto neighbors = maze.getNeighbors(row, col);
        if (!neighbors.empty()) {
            return neighbors[0];
        }
        return { row, col };
    }

public:
    Ghost(int startRow, int startCol, char color)
        : row(startRow), col(startCol), startRow(startRow), startCol(startCol),
        color(color), isScared(false), scaredTimer(0) {
    }

    void move(int targetRow, int targetCol, const Maze& maze, bool playerPoweredUp) {
        if (playerPoweredUp && !isScared) {
            setScared(true);
        }

        if (!playerPoweredUp && isScared) {
            setScared(false);
        }

        if (isScared) {
            auto neighbors = maze.getNeighbors(row, col);
            if (!neighbors.empty()) {
                int randomIndex = rand() % neighbors.size();
                row = neighbors[randomIndex].first;
                col = neighbors[randomIndex].second;
            }
        }
        else {
            auto nextPos = getNextMove(targetRow, targetCol, maze);
            row = nextPos.first;
            col = nextPos.second;
        }

        updateScared();
    }

    pair<int, int> getPosition() const { return { row, col }; }
    char getColor() const { return color; }
    bool getIsScared() const { return isScared; }

    void setScared(bool scared) {
        isScared = scared;
        if (scared) {
            scaredTimer = 300;
        }
        else {
            scaredTimer = 0;
        }
    }

    void updateScared() {
        if (isScared) {
            scaredTimer--;
            if (scaredTimer <= 0) {
                isScared = false;
            }
        }
    }

    void reset() {
        row = startRow;
        col = startCol;
        isScared = false;
        scaredTimer = 0;
    }
};

class Game {
private:
    Maze maze;
    Player player;
    vector<Ghost> ghosts;
    GameState state;

    ScoreHistory scoreHistory;
    MenuTree menuTree;

    int tickCount;
    int playerMoveTimer;
    int ghostMoveTimer;

    bool keysPressed[4];
    char pendingDirection;
    sf::RenderWindow window;
    sf::Font font;
    sf::Text scoreText;
    sf::Text livesText;
    sf::Text gameOverText;
    sf::Text victoryText;
    sf::Text pauseText;
    sf::Text instructionsText;

    static const int CELL_SIZE = 20;
    static const int WINDOW_WIDTH = 700;
    static const int WINDOW_HEIGHT = 750;

    bool loadFont() {
        const char* fontPaths[] = {
            "C:/Windows/Fonts/arial.ttf",
            "C:/Windows/Fonts/calibri.ttf",
            "C:/Windows/Fonts/consola.ttf",
            "C:/Windows/Fonts/tahoma.ttf",
            "C:/Windows/Fonts/verdana.ttf"
        };

        for (const char* path : fontPaths) {
            if (font.openFromFile(path)) {
                return true;
            }
        }
        return false;
    }

    sf::Text createText(const string& str, unsigned int size) const {
        sf::Text text(font, str, size);
        return text;
    }

    void handleCollisions() {
        auto playerPos = player.getPosition();

        char cell = maze.getCell(playerPos.first, playerPos.second);
        if (cell == FOOD) {
            maze.setCell(playerPos.first, playerPos.second, EMPTY);
            maze.decrementFoodCount();
            player.addScore(10);
        }
        else if (cell == POWER_PELLET) {
            maze.setCell(playerPos.first, playerPos.second, EMPTY);
            maze.decrementFoodCount();
            player.addScore(50);
            player.activatePowerUp();
            for (auto& ghost : ghosts) {
                ghost.setScared(true);
            }
        }

        for (auto& ghost : ghosts) {
            auto ghostPos = ghost.getPosition();
            if (playerPos.first == ghostPos.first && playerPos.second == ghostPos.second) {
                if (player.isPoweredUp() && ghost.getIsScared()) {
                    ghost.reset();
                    player.addScore(200);
                }
                else {
                    player.loseLife();
                    player.reset(23, 14);
                    for (auto& g : ghosts) {
                        g.reset();
                    }
                    if (player.getLives() <= 0) {
                        state = GAME_OVER;
                        scoreHistory.addScore(player.getScore(), 1);
                    }
                }
            }
        }
    }

    void updateGame() {
        if (state != PLAYING) return;

        tickCount++;

        player.updatePowerUp();

        const int PLAYER_MOVE_INTERVAL = 10;
        const int GHOST_MOVE_INTERVAL = 14;

        playerMoveTimer++;
        if (playerMoveTimer >= PLAYER_MOVE_INTERVAL) {
            if (keysPressed[0]) player.move('U', maze);
            else if (keysPressed[1]) player.move('D', maze);
            else if (keysPressed[2]) player.move('L', maze);
            else if (keysPressed[3]) player.move('R', maze);
            playerMoveTimer = 0;
        }

        ghostMoveTimer++;
        if (ghostMoveTimer >= GHOST_MOVE_INTERVAL) {
            auto playerPos = player.getPosition();
            for (auto& ghost : ghosts) {
                ghost.move(playerPos.first, playerPos.second, maze, player.isPoweredUp());
            }
            ghostMoveTimer = 0;
        }

        handleCollisions();

        if (maze.getFoodCount() == 0) {
            state = VICTORY;
            scoreHistory.addScore(player.getScore(), 1);
        }

        if (player.getLives() <= 0) {
            state = GAME_OVER;
        }
    }

    void processInput(sf::Keyboard::Key key, bool pressed) {
        switch (key) {
        case sf::Keyboard::Key::W:
        case sf::Keyboard::Key::Up:
            keysPressed[0] = pressed;
            if (pressed) pendingDirection = 'U';
            break;
        case sf::Keyboard::Key::S:
        case sf::Keyboard::Key::Down:
            keysPressed[1] = pressed;
            if (pressed) pendingDirection = 'D';
            break;
        case sf::Keyboard::Key::A:
        case sf::Keyboard::Key::Left:
            keysPressed[2] = pressed;
            if (pressed) pendingDirection = 'L';
            break;
        case sf::Keyboard::Key::D:
        case sf::Keyboard::Key::Right:
            keysPressed[3] = pressed;
            if (pressed) pendingDirection = 'R';
            break;
        case sf::Keyboard::Key::P:
            if (pressed && state == PLAYING) {
                state = PAUSED;
            }
            else if (pressed && state == PAUSED) {
                state = PLAYING;
            }
            break;
        case sf::Keyboard::Key::R:
            if (pressed && (state == GAME_OVER || state == VICTORY)) {
                reset();
            }
            break;
        default:
            break;
        }
    }

    void reset() {
        maze = Maze();
        player = Player(23, 14);
        ghosts.clear();
        ghosts.push_back(Ghost(14, 13, 'R'));
        ghosts.push_back(Ghost(14, 14, 'B'));
        ghosts.push_back(Ghost(14, 15, 'P'));
        ghosts.push_back(Ghost(14, 16, 'Y'));
        state = PLAYING;
        tickCount = 0;
        playerMoveTimer = 0;
        ghostMoveTimer = 0;
        pendingDirection = 'R';
        for (int i = 0; i < 4; i++) {
            keysPressed[i] = false;
        }
    }

    void drawMaze() {
        int offsetX = 50;
        int offsetY = 80;

        static sf::RectangleShape wallRect(sf::Vector2f(CELL_SIZE - 1, CELL_SIZE - 1));
        static sf::RectangleShape emptyRect(sf::Vector2f(CELL_SIZE - 1, CELL_SIZE - 1));
        static sf::CircleShape foodShape(2);
        static sf::CircleShape pelletShape(5);

        wallRect.setFillColor(sf::Color(33, 33, 222));
        emptyRect.setFillColor(sf::Color::Black);
        foodShape.setFillColor(sf::Color::White);
        pelletShape.setFillColor(sf::Color::White);

        for (int i = 0; i < maze.getRows(); i++) {
            for (int j = 0; j < maze.getCols(); j++) {
                int x = offsetX + j * CELL_SIZE;
                int y = offsetY + i * CELL_SIZE;

                char cell = maze.getCell(i, j);

                if (cell == WALL) {
                    wallRect.setPosition(sf::Vector2f(x, y));
                    window.draw(wallRect);
                }
                else if (cell == FOOD) {
                    emptyRect.setPosition(sf::Vector2f(x, y));
                    window.draw(emptyRect);
                    foodShape.setPosition(sf::Vector2f(x + CELL_SIZE / 2 - 2, y + CELL_SIZE / 2 - 2));
                    window.draw(foodShape);
                }
                else if (cell == POWER_PELLET) {
                    emptyRect.setPosition(sf::Vector2f(x, y));
                    window.draw(emptyRect);
                    pelletShape.setPosition(sf::Vector2f(x + CELL_SIZE / 2 - 5, y + CELL_SIZE / 2 - 5));
                    window.draw(pelletShape);
                }
                else {
                    emptyRect.setPosition(sf::Vector2f(x, y));
                    window.draw(emptyRect);
                }
            }
        }
    }

    void drawPlayer() {
        auto pos = player.getPosition();
        int offsetX = 50;
        int offsetY = 80;

        int x = offsetX + pos.second * CELL_SIZE + CELL_SIZE / 2;
        int y = offsetY + pos.first * CELL_SIZE + CELL_SIZE / 2;

        float radius = CELL_SIZE / 2 - 2;

        int mouthFrame = (tickCount / 3) % 4;
        float mouthAngle = (mouthFrame == 0 || mouthFrame == 2) ? 45.0f : 25.0f;

        char dir = player.getDirection();
        float startAngle = 0.0f;
        switch (dir) {
        case 'R': startAngle = 0.0f; break;
        case 'D': startAngle = 90.0f; break;
        case 'L': startAngle = 180.0f; break;
        case 'U': startAngle = 270.0f; break;
        }

        int numPoints = 32;
        sf::ConvexShape pacman;
        pacman.setPointCount(numPoints + 2);

        float centerX = x;
        float centerY = y;
        float startRad = startAngle * 3.14159f / 180.0f;
        float mouthRad = mouthAngle * 3.14159f / 180.0f;
        float totalAngle = 2.0f * 3.14159f - mouthRad;
        float angleStep = totalAngle / numPoints;
        float currentAngle = startRad + mouthRad / 2.0f;

        pacman.setPoint(0, sf::Vector2f(centerX, centerY));
        for (int i = 0; i <= numPoints; i++) {
            float px = centerX + radius * cos(currentAngle);
            float py = centerY + radius * sin(currentAngle);
            pacman.setPoint(i + 1, sf::Vector2f(px, py));
            currentAngle += angleStep;
        }

        pacman.setFillColor(sf::Color(255, 255, 0));
        window.draw(pacman);
    }

    void drawGhosts() {
        int offsetX = 50;
        int offsetY = 80;

        static sf::CircleShape ghostShape(CELL_SIZE / 2 - 2);

        for (const auto& ghost : ghosts) {
            auto pos = ghost.getPosition();
            int x = offsetX + pos.second * CELL_SIZE + CELL_SIZE / 2;
            int y = offsetY + pos.first * CELL_SIZE + CELL_SIZE / 2;

            if (ghost.getIsScared()) {
                ghostShape.setFillColor(sf::Color(0, 100, 255));
            }
            else {
                switch (ghost.getColor()) {
                case 'R': ghostShape.setFillColor(sf::Color::Red); break;
                case 'B': ghostShape.setFillColor(sf::Color::Cyan); break;
                case 'P': ghostShape.setFillColor(sf::Color(255, 192, 203)); break;
                case 'Y': ghostShape.setFillColor(sf::Color::Yellow); break;
                default: ghostShape.setFillColor(sf::Color::White); break;
                }
            }

            ghostShape.setPosition(sf::Vector2f(x - CELL_SIZE / 2 + 2, y - CELL_SIZE / 2 + 2));
            window.draw(ghostShape);
        }
    }

    void drawUI() {
        ostringstream scoreStream;
        scoreStream << "Score: " << player.getScore();
        scoreText.setString(scoreStream.str());

        ostringstream livesStream;
        livesStream << "Lives: " << player.getLives();
        livesText.setString(livesStream.str());

        ostringstream highScoreStream;
        highScoreStream << "High Score: " << scoreHistory.getHighScore();
        sf::Text highScoreText(font, highScoreStream.str(), 18);
        highScoreText.setFillColor(sf::Color::Green);
        highScoreText.setPosition(sf::Vector2f(10, 60));

        window.draw(scoreText);
        window.draw(livesText);
        window.draw(highScoreText);
        window.draw(instructionsText);

        if (player.isPoweredUp()) {
            sf::Text powerText(font, "POWERED UP!", 18);
            powerText.setFillColor(sf::Color::Magenta);
            powerText.setPosition(sf::Vector2f(10, 85));
            window.draw(powerText);
        }
    }

    void drawGameOver() {
        window.draw(gameOverText);

        ostringstream ss;
        ss << "Final Score: " << player.getScore();
        sf::Text finalScore(font, ss.str(), 24);
        finalScore.setFillColor(sf::Color::White);
        finalScore.setPosition(sf::Vector2f(WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2));
        window.draw(finalScore);

        sf::Text restart(font, "Press R to Restart or Q to Quit", 20);
        restart.setFillColor(sf::Color::Cyan);
        restart.setPosition(sf::Vector2f(WINDOW_WIDTH / 2 - 180, WINDOW_HEIGHT / 2 + 50));
        window.draw(restart);
    }

    void drawVictory() {
        window.draw(victoryText);

        ostringstream ss;
        ss << "Final Score: " << player.getScore();
        sf::Text finalScore(font, ss.str(), 24);
        finalScore.setFillColor(sf::Color::White);
        finalScore.setPosition(sf::Vector2f(WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2));
        window.draw(finalScore);

        sf::Text restart(font, "Press R to Restart or Q to Quit", 20);
        restart.setFillColor(sf::Color::Cyan);
        restart.setPosition(sf::Vector2f(WINDOW_WIDTH / 2 - 180, WINDOW_HEIGHT / 2 + 50));
        window.draw(restart);
    }

    void drawPause() {
        window.draw(pauseText);
    }

    void render() {
        window.clear(sf::Color::Black);

        if (state == PLAYING || state == PAUSED) {
            drawMaze();
            drawPlayer();
            drawGhosts();
            drawUI();
            if (state == PAUSED) {
                drawPause();
            }
        }
        else if (state == GAME_OVER) {
            drawMaze();
            drawUI();
            drawGameOver();
        }
        else if (state == VICTORY) {
            drawMaze();
            drawUI();
            drawVictory();
        }

        window.display();
    }

public:
    Game() : window(sf::VideoMode(sf::Vector2u(WINDOW_WIDTH, WINDOW_HEIGHT)), "Pac-Man Game - DSA Edition"),
        scoreText(font, "", 20),
        livesText(font, "", 20),
        gameOverText(font, "GAME OVER!", 40),
        victoryText(font, "VICTORY!", 40),
        pauseText(font, "PAUSED - Press P to Resume", 30),
        instructionsText(font, "WASD: Move | P: Pause | Q: Quit | R: Restart", 16),
        state(PLAYING), tickCount(0), playerMoveTimer(0), ghostMoveTimer(0), pendingDirection('R') {

        for (int i = 0; i < 4; i++) {
            keysPressed[i] = false;
        }

        srand(time(nullptr));

        player = Player(23, 14);
        ghosts.push_back(Ghost(14, 13, 'R'));
        ghosts.push_back(Ghost(14, 14, 'B'));
        ghosts.push_back(Ghost(14, 15, 'P'));
        ghosts.push_back(Ghost(14, 16, 'Y'));

        window.setFramerateLimit(0);
        window.setVerticalSyncEnabled(false);

        if (!loadFont()) {
            cerr << "Warning: Could not load font. Text may not display correctly." << endl;
        }

        scoreText = sf::Text(font, "", 20);
        scoreText.setFillColor(sf::Color::Yellow);
        scoreText.setPosition(sf::Vector2f(10, 10));

        livesText = sf::Text(font, "", 20);
        livesText.setFillColor(sf::Color::White);
        livesText.setPosition(sf::Vector2f(10, 35));

        gameOverText = sf::Text(font, "GAME OVER!", 40);
        gameOverText.setFillColor(sf::Color::Red);
        gameOverText.setPosition(sf::Vector2f(WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT / 2 - 50));

        victoryText = sf::Text(font, "VICTORY!", 40);
        victoryText.setFillColor(sf::Color::Green);
        victoryText.setPosition(sf::Vector2f(WINDOW_WIDTH / 2 - 120, WINDOW_HEIGHT / 2 - 50));

        pauseText = sf::Text(font, "PAUSED - Press P to Resume", 30);
        pauseText.setFillColor(sf::Color::Yellow);
        pauseText.setPosition(sf::Vector2f(WINDOW_WIDTH / 2 - 200, WINDOW_HEIGHT / 2 - 15));

        instructionsText = sf::Text(font, "WASD: Move | P: Pause | Q: Quit | R: Restart", 16);
        instructionsText.setFillColor(sf::Color::Cyan);
        instructionsText.setPosition(sf::Vector2f(10, WINDOW_HEIGHT - 30));
    }

    void run() {
        sf::Clock clock;
        sf::Time timeSinceLastUpdate = sf::Time::Zero;
        const sf::Time timePerFrame = sf::seconds(1.0f / 120.0f);

        while (window.isOpen()) {
            sf::Time elapsedTime = clock.restart();
            timeSinceLastUpdate += elapsedTime;

            while (optional<sf::Event> eventOpt = window.pollEvent()) {
                sf::Event event = *eventOpt;

                if (event.is<sf::Event::Closed>()) {
                    window.close();
                }

                if (auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
                    if (keyPressed->code == sf::Keyboard::Key::Q) {
                        window.close();
                    }
                    else {
                        processInput(keyPressed->code, true);
                    }
                }

                if (auto* keyReleased = event.getIf<sf::Event::KeyReleased>()) {
                    processInput(keyReleased->code, false);
                }
            }

            while (timeSinceLastUpdate > timePerFrame) {
                timeSinceLastUpdate -= timePerFrame;
                updateGame();
            }

            render();
        }
    }
};

int main() {
    try {
        Game game;
        game.run();
    }
    catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}

