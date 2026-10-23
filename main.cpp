#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <algorithm>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#endif

using namespace std;

const int WIDTH = 10;
const int HEIGHT = 20;

// Définition des pièces (grilles 4x4)
const vector<vector<string>> PIECES = {
    // I
    {
        "    ",
        "####",
        "    ",
        "    "
    },
    // J
    {
        "#   ",
        "### ",
        "    ",
        "    "
    },
    // L
    {
        "  # ",
        "### ",
        "    ",
        "    "
    },
    // O
    {
        " ## ",
        " ## ",
        "    ",
        "    "
    },
    // S
    {
        " ## ",
        "##  ",
        "    ",
        "    "
    },
    // T
    {
        " #  ",
        "### ",
        "    ",
        "    "
    },
    // Z
    {
        "##  ",
        " ## ",
        "    ",
        "    "
    }
};

class Tetris {
private:
    char board[HEIGHT][WIDTH];
    int score;
    int rowsHit;
    int level;
    int currentPiece;
    int nextPiece;
    int curX, curY;
    vector<string> currentShape;
    bool gameOver;

    void spawnPiece() {
        currentPiece = nextPiece;
        nextPiece = rand() % PIECES.size();
        currentShape = PIECES[currentPiece];
        curX = WIDTH / 2 - 2;
        curY = 0;

        if (checkCollision(curX, curY, currentShape)) {
            gameOver = true;
        }
    }

    bool checkCollision(int posX, int posY, const vector<string>& shape) {
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (shape[i][j] != ' ') {
                    int boardX = posX + j;
                    int boardY = posY + i;
                    if (boardX < 0 || boardX >= WIDTH || boardY >= HEIGHT) {
                        return true;
                    }
                    if (boardY >= 0 && board[boardY][boardX] != ' ') {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    void mergePiece() {
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (currentShape[i][j] != ' ') {
                    int boardX = curX + j;
                    int boardY = curY + i;
                    if (boardY >= 0 && boardY < HEIGHT && boardX >= 0 && boardX < WIDTH) {
                        board[boardY][boardX] = '#';
                    }
                }
            }
        }
    }

    void clearLines() {
        int linesCleared = 0;
        for (int i = HEIGHT - 1; i >= 0; i--) {
            bool full = true;
            for (int j = 0; j < WIDTH; j++) {
                if (board[i][j] == ' ') {
                    full = false;
                    break;
                }
            }
            if (full) {
                linesCleared++;
                for (int k = i; k > 0; k--) {
                    for (int j = 0; j < WIDTH; j++) {
                        board[k][j] = board[k - 1][j];
                    }
                }
                for (int j = 0; j < WIDTH; j++) {
                    board[0][j] = ' ';
                }
                i++;
            }
        }

        if (linesCleared > 0) {
            rowsHit += linesCleared;
            score += linesCleared * 100 * level;
            level = 1 + rowsHit / 10;
        }
    }

    void rotate() {
        vector<string> temp = currentShape;
        
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                currentShape[j][4 - 1 - i] = temp[i][j];
            }
        }

        if (checkCollision(curX, curY, currentShape)) {
            if (!checkCollision(curX - 1, curY, currentShape)) {
                curX--;
            } else if (!checkCollision(curX + 1, curY, currentShape)) {
                curX++;
            } else if (!checkCollision(curX, curY - 1, currentShape)) {
                curY--;
            } else {
                currentShape = temp;
            }
        }
    }

public:
    Tetris() {
        srand(time(0));
        score = 0;
        rowsHit = 0;
        level = 1;
        gameOver = false;
        
        for (int i = 0; i < HEIGHT; i++) {
            for (int j = 0; j < WIDTH; j++) {
                board[i][j] = ' ';
            }
        }
        nextPiece = rand() % PIECES.size();
        spawnPiece();
    }

    void run() {
        while (!gameOver) {
            drawUI();
            
            // Boucle de temporisation ultra-réactive (scrutation des touches toutes les 20 ms)
            int dropDelay = max(100, 600 - (level - 1) * 30);
            int elapsed = 0;
            while (elapsed < dropDelay && !gameOver) {
                handleInput();
                #ifdef _WIN32
                Sleep(20);
                #else
                usleep(20000);
                #endif
                elapsed += 20;
            }
            
            if (!checkCollision(curX, curY + 1, currentShape)) {
                curY++;
            } else {
                mergePiece();
                clearLines();
                spawnPiece();
            }
        }

        cout << "\n\n\t=== PARTIE TERMINEE ===" << endl;
        cout << "\tScore final : " << score << endl;
    }

    void drawUI() {
        #ifdef _WIN32
        COORD coord = {0, 0};
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
        #else
        cout << "\033[H\033[J";
        #endif

        cout << "\n";
        
        for (int i = 0; i < HEIGHT; i++) {
            // 1. Panneau de gauche (Statistiques)
            string leftStat = "                 ";
            if (i == 1) leftStat = " ROWS HIT: " + to_string(rowsHit);
            else if (i == 3) leftStat = " SCORE:    " + to_string(score);
            else if (i == 5) leftStat = " LEVEL:    " + to_string(level);
            while (leftStat.length() < 17) leftStat += " ";

            // 2. Plateau de jeu central
            string boardRow = "<!";
            for (int j = 0; j < WIDTH; j++) {
                bool isPieceCell = false;
                if (i >= curY && i < curY + 4 && j >= curX && j < curX + 4) {
                    if (currentShape[i - curY][j - curX] != ' ') {
                        isPieceCell = true;
                    }
                }

                if (isPieceCell || board[i][j] != ' ') {
                    boardRow += " #";
                } else {
                    boardRow += " .";
                }
            }
            boardRow += " !>";

            // 3. Panneau de droite (Prochaine pièce et contrôles AZERTY)
            string rightPanel = "";
            if (i == 1) rightPanel = "   NEXT PIECE:";
            else if (i >= 2 && i <= 5) {
                rightPanel = "   ";
                int pRow = i - 2;
                for (int c = 0; c < 4; c++) {
                    if (pRow < PIECES[nextPiece].size() && PIECES[nextPiece][pRow][c] != ' ') {
                        rightPanel += "##";
                    } else {
                        rightPanel += "  ";
                    }
                }
            }
            else if (i == 7) rightPanel = "   Z / UP: ROTATE";
            else if (i == 8) rightPanel = "   S / DOWN: SOFT DROP";
            else if (i == 9) rightPanel = "   Q / LEFT: MOVE LEFT";
            else if (i == 10) rightPanel = "   D / RIGHT: MOVE RIGHT";
            else if (i == 11) rightPanel = "   SPACE: HARD DROP";
            else if (i == 13) rightPanel = "   ESC: QUIT";

            cout << leftStat << boardRow << rightPanel << endl;
        }

        // Bordure inférieure
        cout << "                  <!====================!>" << endl;
        cout << "                   \\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\" << endl;
    }

    void handleInput() {
        #ifdef _WIN32
        // Utilisation de GetAsyncKeyState pour une réactivité instantanée sur clavier AZERTY et flèches
        if (GetAsyncKeyState(VK_LEFT) & 0x8000 || GetAsyncKeyState('Q') & 0x8000) {
            if (!checkCollision(curX - 1, curY, currentShape)) {
                curX--;
                drawUI();
                Sleep(60); // Anti-rebond fluide
            }
        }
        if (GetAsyncKeyState(VK_RIGHT) & 0x8000 || GetAsyncKeyState('D') & 0x8000) {
            if (!checkCollision(curX + 1, curY, currentShape)) {
                curX++;
                drawUI();
                Sleep(60);
            }
        }
        if (GetAsyncKeyState(VK_DOWN) & 0x8000 || GetAsyncKeyState('S') & 0x8000) {
            if (!checkCollision(curX, curY + 1, currentShape)) {
                curY++;
                score += 1;
                drawUI();
                Sleep(40);
            }
        }
        if (GetAsyncKeyState(VK_UP) & 0x8000 || GetAsyncKeyState('Z') & 0x8000) {
            rotate();
            drawUI();
            Sleep(150);
        }
        if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
            while (!checkCollision(curX, curY + 1, currentShape)) {
                curY++;
                score += 2;
            }
            mergePiece();
            clearLines();
            spawnPiece();
            drawUI();
            Sleep(200);
        }
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            exit(0);
        }
        #else
        if (_kbhit()) {
            char cmd = _getch();
            if (cmd == 'q' || cmd == 'Q') {
                if (!checkCollision(curX - 1, curY, currentShape)) curX--;
            } else if (cmd == 'd' || cmd == 'D') {
                if (!checkCollision(curX + 1, curY, currentShape)) curX++;
            } else if (cmd == 's' || cmd == 'S') {
                if (!checkCollision(curX, curY + 1, currentShape)) curY++;
            } else if (cmd == 'z' || cmd == 'Z') {
                rotate();
            } else if (cmd == 27) {
                exit(0);
            }
        }
        #endif
    }
};

int main() {
    Tetris game;
    game.run();
    return 0;
}