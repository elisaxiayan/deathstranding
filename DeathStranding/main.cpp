/*
 * ========================================
 *  简化版《搁浅》游戏 - Death Stranding
 * ========================================
 *
 * 功能说明:
 * 1. 登录/注册系统(链表实现)
 * 2. 5行9列棋盘布局
 * 3. 快递员山姆(WASD移动,J近战,K射击)
 * 4. 6种不同敌人(强盗、BT、狙击手、坦克、刺客、召唤师)
 * 5. 子弹发射动画
 * 6. 文件读写(保存用户数据)
 * 7. 排行榜(选择排序)
 * 8. 按钮区(开始、暂停、结束)
 * 9. 信息显示区(玩家名、分数、倒计时等)
 * 10. 通关条件:100分
 */
#define _CRT_SECURE_NO_WARNINGS
#include <graphics.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <windows.h>

 // ==================== 常量定义 ====================
#define WINDOW_WIDTH 1200      // 窗口宽度
#define WINDOW_HEIGHT 700      // 窗口高度
#define GRID_ROWS 5            // 棋盘行数
#define GRID_COLS 9            // 棋盘列数
#define CELL_SIZE 70           // 格子大小
#define GAME_AREA_X 50         // 游戏区X坐标
#define GAME_AREA_Y 100        // 游戏区Y坐标
#define MAX_BULLETS 30         // 最大子弹数
#define MAX_USERNAME 20        // 最大用户名长度
#define MAX_PASSWORD 20        // 最大密码长度
#define WIN_SCORE 100          // 通关分数
#define GAME_TIME 180          // 游戏时间(秒)

// 地图元素类型
typedef enum {
    EMPTY,
    WALL
} CellType;

// 装备类型枚举
typedef enum {
    WEAPON_NORMAL = 0,
    WEAPON_SUPER = 1,
    WEAPON_ULTIMATE = 2
} WeaponType;

// 护甲类型枚举
typedef enum {
    ARMOR_NONE = 0,
    ARMOR_LIGHT = 1,
    ARMOR_HEAVY = 2
} ArmorType;

// ==================== 结构体定义 ====================

// 用户节点(链表)
typedef struct UserNode {
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    int highScore; // 最高分
    int totalScore; // 总积分
    int unlockedLevels;
    int levelScores[5]; // 各关卡最高分
    WeaponType currentWeapon;
    ArmorType currentArmor;
    struct UserNode* next;
} UserNode;

// 位置结构
typedef struct {
    int row;
    int col;
} Position;

// 快递员山姆
typedef struct {
    Position pos;
    int health;
    int score;
    int direction;
    WeaponType weapon;
    ArmorType armor;
} Sam;

// 合作者
typedef struct {
    Position pos;
    int health;
    int direction;
    WeaponType weapon;
    ArmorType armor;
} Partner;

// 敌人类型枚举
typedef enum {
    ENEMY_BANDIT,
    ENEMY_BT,
    ENEMY_SNIPER,
    ENEMY_TANK,
    ENEMY_ASSASSIN,
    ENEMY_SUMMONER
} EnemyType;

// 敌人结构体
typedef struct {
    Position pos;
    EnemyType type;
    int health;
    int maxHealth;
    int shootTimer;
    int moveTimer;
    int summonTimer;
    bool active;
} Enemy;

// 子弹结构
typedef struct {
    float x, y;
    float dx, dy;
    int active;
    int fromPlayer;
    int frame;
} Bullet;

// 按钮结构
typedef struct {
    int x, y;
    int width, height;
    TCHAR text[20];
    int enabled;
} Button;

// 游戏状态枚举
typedef enum {
    STATE_LOGIN,
    STATE_REGISTER,
    STATE_MENU,
    STATE_MODE_SELECT,
    STATE_LEVEL_SELECT,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_WIN,
    STATE_LOSE,
    STATE_RANKING,
    STATE_SHOP,
    STATE_EXIT
} GameState;

// 游戏模式枚举
typedef enum {
    MODE_SINGLE,
    MODE_DOUBLE
} GameMode;

// 游戏状态结构体
typedef struct {
    UserNode* userListHead;
    char currentUser[MAX_USERNAME];
    Enemy enemies[20]; // 最多20个敌人
    GameState gameState;
    GameMode gameMode;
    int gameTimer;
    int frameCount;
    int currentLevel;
    int levelScoreThreshold;
    CellType map[GRID_ROWS][GRID_COLS];
    bool levelCompleted;
    // 登录注册相关
    char loginUsername[MAX_USERNAME];
    char loginPassword[MAX_PASSWORD];
    char registerUsername[MAX_USERNAME];
    char registerPassword[MAX_PASSWORD];
    int loginInputMode;
    int registerInputMode;
    int loginUsernameLen;
    int loginPasswordLen;
    int registerUsernameLen;
    int registerPasswordLen;
    bool loginError;
    bool registerError;
    char errorMessage[100];
    // 游戏对象
    Sam sam;
    Partner partner;
    Bullet bullets[MAX_BULLETS];
    bool initialized;
    int lastSecond;
    // 按键状态追踪，用于防止重复触发
    bool keyStates[256];
} GameContext;



// ==================== 函数声明 ====================
void saveUsersToFile(GameContext* ctx);
void loadUsersFromFile(GameContext* ctx);
void saveGameState(GameContext* ctx);
bool loadGameState(GameContext* ctx);
UserNode* findUser(GameContext* ctx, const char* username);
int registerUser(GameContext* ctx, const char* username, const char* password);
int loginUser(GameContext* ctx, const char* username, const char* password);
void updateHighScore(GameContext* ctx, const char* username, int score, int level);
int getHighScore(GameContext* ctx, const char* username);
void showRanking(GameContext* ctx);
void freeUserList(GameContext* ctx);
void addEnemy(GameContext* ctx, int row, int col, EnemyType type);
void removeEnemy(GameContext* ctx, int index);
void clearEnemyList(GameContext* ctx);
int countEnemies(GameContext* ctx);
Enemy* getEnemyAt(GameContext* ctx, int row, int col);
void initSam(GameContext* ctx, Sam* sam);
void initBullets(GameContext* ctx, Bullet bullets[]);
void spawnEnemies(GameContext* ctx);
void moveSam(GameContext* ctx, Sam* sam, int drow, int dcol);
void meleeAttack(GameContext* ctx, Sam* sam);
void shootBullet(GameContext* ctx, Bullet bullets[], Sam* sam);
void enemyShootBullet(GameContext* ctx, Bullet bullets[], Enemy* enemy, Sam* sam);
void updateBullets(GameContext* ctx, Bullet bullets[], Sam* sam);
void updateEnemies(GameContext* ctx, Bullet bullets[], Sam* sam);
void drawGrid(GameContext* ctx);
void drawSam(GameContext* ctx, Sam* sam);
void drawEnemies(GameContext* ctx);
void drawBullets(GameContext* ctx, Bullet bullets[]);
void drawButton(GameContext* ctx, Button* btn);
int isMouseOnButton(GameContext* ctx, Button* btn, int mouseX, int mouseY);
void drawInfoArea(GameContext* ctx, Sam* sam);
void drawUI(GameContext* ctx, Sam* sam, Partner* partner, Bullet bullets[]);
void loginScreen(GameContext* ctx);
void registerScreen(GameContext* ctx);
void menuScreen(GameContext* ctx);
void levelSelectScreen(GameContext* ctx);
void gameLoop(GameContext* ctx);
void pauseScreen(GameContext* ctx);
void winScreen(GameContext* ctx, int score);
void loseScreen(GameContext* ctx);
void rankingScreen(GameContext* ctx);
void shopScreen(GameContext* ctx);
void buyWeapon(GameContext* ctx, Sam* sam, WeaponType weapon);
void buyArmor(GameContext* ctx, Sam* sam, ArmorType armor);
void initMap(GameContext* ctx);

// ==================== 文件操作函数 ====================

void saveUsersToFile(GameContext* ctx) {
    FILE* file = fopen("users.dat", "w");
    if (file == NULL) return;

    UserNode* current = ctx->userListHead;
    while (current != NULL) {
        fprintf(file, "%s %s %d %d %d %d %d %d %d %d %d %d\n", 
                current->username, current->password, 
                current->highScore, current->totalScore, current->unlockedLevels, 
                current->currentWeapon, current->currentArmor, 
                current->levelScores[0], current->levelScores[1], current->levelScores[2], current->levelScores[3], current->levelScores[4]);
        current = current->next;
    }
    fclose(file);
}

void loadUsersFromFile(GameContext* ctx) {
    FILE* file = fopen("users.dat", "r");
    if (file == NULL) return;

    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    int highScore;
    int totalScore;
    int unlockedLevels;
    int currentWeapon;
    int currentArmor;
    int levelScores[5];

    while (fscanf(file, "%s %s %d %d %d %d %d %d %d %d %d %d", username, password, &highScore, &totalScore, &unlockedLevels, 
                  &currentWeapon, &currentArmor, &levelScores[0], &levelScores[1], &levelScores[2], &levelScores[3], &levelScores[4]) == 12) {
        UserNode* newNode = (UserNode*)malloc(sizeof(UserNode));
        strcpy(newNode->username, username);
        strcpy(newNode->password, password);
        newNode->highScore = highScore;
        newNode->totalScore = totalScore;
        newNode->unlockedLevels = (unlockedLevels > 0) ? unlockedLevels : 1; // 确保至少解锁第一关
        newNode->currentWeapon = (WeaponType)currentWeapon;
        newNode->currentArmor = (ArmorType)currentArmor;
        for (int i = 0; i < 5; i++) {
            newNode->levelScores[i] = levelScores[i];
        }
        newNode->next = ctx->userListHead;
        ctx->userListHead = newNode;
    }
    fclose(file);
}

// 保存游戏状态
void saveGameState(GameContext* ctx) {
    FILE* file = fopen("savegame.dat", "w");
    if (file == NULL) return;

    // 保存游戏上下文
    fprintf(file, "%d %d %d %d %d\n", 
            ctx->gameState, 
            ctx->gameMode, 
            ctx->gameTimer, 
            ctx->currentLevel, 
            ctx->levelScoreThreshold);

    // 保存山姆状态
    fprintf(file, "%d %d %d %d %d %d %d\n", 
            ctx->sam.pos.row, 
            ctx->sam.pos.col, 
            ctx->sam.health, 
            ctx->sam.score, 
            ctx->sam.direction, 
            ctx->sam.weapon, 
            ctx->sam.armor);

    // 保存敌人状态
    int enemyCount = 0;
    for (int i = 0; i < 20; i++) {
        if (ctx->enemies[i].active) {
            enemyCount++;
        }
    }
    fprintf(file, "%d\n", enemyCount);
    for (int i = 0; i < 20; i++) {
        if (ctx->enemies[i].active) {
            fprintf(file, "%d %d %d %d %d %d %d %d\n", 
                    ctx->enemies[i].pos.row, 
                    ctx->enemies[i].pos.col, 
                    ctx->enemies[i].type, 
                    ctx->enemies[i].health, 
                    ctx->enemies[i].maxHealth, 
                    ctx->enemies[i].shootTimer, 
                    ctx->enemies[i].moveTimer, 
                    ctx->enemies[i].summonTimer);
        }
    }

    // 保存地图
    for (int i = 0; i < GRID_ROWS; i++) {
        for (int j = 0; j < GRID_COLS; j++) {
            fprintf(file, "%d ", ctx->map[i][j]);
        }
        fprintf(file, "\n");
    }

    fclose(file);
}

// 加载游戏状态
bool loadGameState(GameContext* ctx) {
    FILE* file = fopen("savegame.dat", "r");
    if (file == NULL) return false;

    // 加载游戏上下文
    int gameState, gameMode, gameTimer, currentLevel, levelScoreThreshold;
    if (fscanf(file, "%d %d %d %d %d", &gameState, &gameMode, &gameTimer, &currentLevel, &levelScoreThreshold) != 5) {
        fclose(file);
        return false;
    }
    ctx->gameState = (GameState)gameState;
    ctx->gameMode = (GameMode)gameMode;
    ctx->gameTimer = gameTimer;
    ctx->currentLevel = currentLevel;
    ctx->levelScoreThreshold = levelScoreThreshold;

    // 加载山姆状态
    int row, col, health, score, direction, weapon, armor;
    if (fscanf(file, "%d %d %d %d %d %d %d", &row, &col, &health, &score, &direction, &weapon, &armor) != 7) {
        fclose(file);
        return false;
    }
    ctx->sam.pos.row = row;
    ctx->sam.pos.col = col;
    ctx->sam.health = health;
    ctx->sam.score = score;
    ctx->sam.direction = direction;
    ctx->sam.weapon = (WeaponType)weapon;
    ctx->sam.armor = (ArmorType)armor;

    // 加载敌人状态
    clearEnemyList(ctx);
    int enemyCount;
    if (fscanf(file, "%d", &enemyCount) != 1) {
        fclose(file);
        return false;
    }
    for (int i = 0; i < enemyCount; i++) {
        int eRow, eCol, eType, eHealth, eMaxHealth, eShootTimer, eMoveTimer, eSummonTimer;
        if (fscanf(file, "%d %d %d %d %d %d %d %d", &eRow, &eCol, &eType, &eHealth, &eMaxHealth, &eShootTimer, &eMoveTimer, &eSummonTimer) != 8) {
            fclose(file);
            return false;
        }
        for (int j = 0; j < 20; j++) {
            if (!ctx->enemies[j].active) {
                ctx->enemies[j].pos.row = eRow;
                ctx->enemies[j].pos.col = eCol;
                ctx->enemies[j].type = (EnemyType)eType;
                ctx->enemies[j].health = eHealth;
                ctx->enemies[j].maxHealth = eMaxHealth;
                ctx->enemies[j].shootTimer = eShootTimer;
                ctx->enemies[j].moveTimer = eMoveTimer;
                ctx->enemies[j].summonTimer = eSummonTimer;
                ctx->enemies[j].active = true;
                break;
            }
        }
    }

    // 加载地图
    for (int i = 0; i < GRID_ROWS; i++) {
        for (int j = 0; j < GRID_COLS; j++) {
            int cellType;
            if (fscanf(file, "%d", &cellType) != 1) {
                fclose(file);
                return false;
            }
            ctx->map[i][j] = (CellType)cellType;
        }
    }

    fclose(file);
    ctx->initialized = true;
    ctx->lastSecond = (int)time(NULL);
    return true;
}

// ==================== 用户管理函数 ====================

UserNode* findUser(GameContext* ctx, const char* username) {
    UserNode* current = ctx->userListHead;
    while (current != NULL) {
        if (strcmp(current->username, username) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

int registerUser(GameContext* ctx, const char* username, const char* password) {
    if (findUser(ctx, username) != NULL) return 0;

    UserNode* newNode = (UserNode*)malloc(sizeof(UserNode));
    strcpy(newNode->username, username);
    strcpy(newNode->password, password);
    newNode->highScore = 0;
    newNode->totalScore = 0; // 初始积分
    newNode->unlockedLevels = 1; // 初始解锁第一关
    for (int i = 0; i < 5; i++) {
        newNode->levelScores[i] = 0;
    }
    newNode->currentWeapon = WEAPON_NORMAL;
    newNode->currentArmor = ARMOR_NONE;
    newNode->next = ctx->userListHead;//头插法
    ctx->userListHead = newNode;

    saveUsersToFile(ctx);
    return 1;
}

int loginUser(GameContext* ctx, const char* username, const char* password) {
    UserNode* user = findUser(ctx, username);
    if (user == NULL) return 0;
    if (strcmp(user->password, password) == 0) {
        strcpy(ctx->currentUser, username);
        return 1;
    }
    return 0;
}

void updateHighScore(GameContext* ctx, const char* username, int score, int level) {
    UserNode* user = findUser(ctx, username);
    if (user != NULL) {
        // 更新关卡最高分
        if (score > user->levelScores[level - 1]) {
            user->levelScores[level - 1] = score;
        }
        // 更新总最高分
        if (score > user->highScore) {
            user->highScore = score;
        }
        saveUsersToFile(ctx);
    }
}

int getHighScore(GameContext* ctx, const char* username) {
    UserNode* user = findUser(ctx, username);
    return (user != NULL) ? user->highScore : 0;
}



void showRanking(GameContext* ctx) {
    int userCount = 0;
    UserNode* current = ctx->userListHead;
    while (current != NULL) {
        userCount++;
        current = current->next;
    }

    if (userCount == 0) {
        settextstyle(30, 0, _T("宋体"));
        settextcolor(WHITE);
        outtextxy(400, 300, _T("暂无排行数据"));
        return;
    }

    settextstyle(40, 0, _T("黑体"));
    settextcolor(YELLOW);
    outtextxy(450, 50, _T("关卡排行榜"));

    settextstyle(25, 0, _T("宋体"));
    
    // 显示每个关卡的排行榜
    for (int level = 1; level <= 5; level++) {
        // 显示关卡标题
        settextcolor(GREEN);
        TCHAR levelText[50];
        swprintf_s(levelText, 50, _T("关卡 %d"), level);
        outtextxy(150, 120 + (level - 1) * 100, levelText);
        
        // 使用(选择排序)找出前3名
        UserNode* top1 = NULL;
        UserNode* top2 = NULL;
        UserNode* top3 = NULL;
        
        current = ctx->userListHead;
        while (current != NULL) {
            int score = current->levelScores[level - 1];
            
            if (top1 == NULL || score > top1->levelScores[level - 1]) {
                top3 = top2;
                top2 = top1;
                top1 = current;
            } else if (top2 == NULL || score > top2->levelScores[level - 1]) {
                top3 = top2;
                top2 = current;
            } else if (top3 == NULL || score > top3->levelScores[level - 1]) {
                top3 = current;
            }
            
            current = current->next;
        }
        
        // 显示前3名
        UserNode* topUsers[] = {top1, top2, top3};
        for (int i = 0; i < 3; i++) {
            if (topUsers[i] != NULL) {
                if (i == 0) settextcolor(RGB(255, 215, 0));
                else if (i == 1) settextcolor(RGB(192, 192, 192));
                else if (i == 2) settextcolor(RGB(205, 127, 50));
                else settextcolor(WHITE);

                TCHAR text[100];
                swprintf_s(text, 100, _T("%d: %S - %d分"), i + 1, topUsers[i]->username, topUsers[i]->levelScores[level - 1]);
                outtextxy(250, 150 + (level - 1) * 100 + i * 30, text);
            }
        }
    }

    settextcolor(LIGHTGRAY);
    settextstyle(20, 0, _T("宋体"));
    outtextxy(450, 650, _T("按任意键返回..."));
}

void freeUserList(GameContext* ctx) {
    UserNode* current = ctx->userListHead;
    while (current != NULL) {
        UserNode* temp = current;
        current = current->next;
        free(temp);
    }
    ctx->userListHead = NULL;
}

// ==================== 敌人管理函数 ====================

void addEnemy(GameContext* ctx, int row, int col, EnemyType type) {
    for (int i = 0; i < 20; i++) {
        if (!ctx->enemies[i].active) {
            ctx->enemies[i].pos.row = row;
            ctx->enemies[i].pos.col = col;
            ctx->enemies[i].type = type;
            ctx->enemies[i].shootTimer = 0;
            ctx->enemies[i].moveTimer = 0;
            ctx->enemies[i].summonTimer = 0;
            ctx->enemies[i].active = true;

            switch (type) {
            case ENEMY_BANDIT:
                ctx->enemies[i].health = 1;
                ctx->enemies[i].maxHealth = 1;
                break;
            case ENEMY_BT:
                ctx->enemies[i].health = 2;
                ctx->enemies[i].maxHealth = 2;
                break;
            case ENEMY_SNIPER:
                ctx->enemies[i].health = 1;
                ctx->enemies[i].maxHealth = 1;
                break;
            case ENEMY_TANK:
                ctx->enemies[i].health = 5;
                ctx->enemies[i].maxHealth = 5;
                break;
            case ENEMY_ASSASSIN:
                ctx->enemies[i].health = 1;
                ctx->enemies[i].maxHealth = 1;
                break;
            case ENEMY_SUMMONER:
                ctx->enemies[i].health = 3;
                ctx->enemies[i].maxHealth = 3;
                break;
            }
            break;
        }
    }
}

void removeEnemy(GameContext* ctx, int index) {
    if (index >= 0 && index < 20) {
        ctx->enemies[index].active = false;
    }
}

void clearEnemyList(GameContext* ctx) {
    for (int i = 0; i < 20; i++) {
        ctx->enemies[i].active = false;
    }
}

int countEnemies(GameContext* ctx) {
    int count = 0;
    for (int i = 0; i < 20; i++) {
        if (ctx->enemies[i].active) {
            count++;
        }
    }
    return count;
}

Enemy* getEnemyAt(GameContext* ctx, int row, int col) {
    for (int i = 0; i < 20; i++) {
        if (ctx->enemies[i].active && 
            ctx->enemies[i].pos.row == row && 
            ctx->enemies[i].pos.col == col) {
            return &ctx->enemies[i];
        }
    }
    return NULL;
}

// ==================== 游戏逻辑函数 ====================

void initSam(GameContext* ctx, Sam* sam) {
    sam->pos.row = GRID_ROWS / 2;
    sam->pos.col = GRID_COLS / 2;
    sam->health = 100;
    sam->score = 0;
    sam->direction = 2;
    sam->weapon = WEAPON_NORMAL;
    sam->armor = ARMOR_NONE;
}

void initBullets(GameContext* ctx, Bullet bullets[]) {
    int i;
    for (i = 0; i < MAX_BULLETS; i++) {
        bullets[i].active = 0;
    }
}

void spawnEnemies(GameContext* ctx) {
    // 根据关卡调整敌人数量，降低前面关卡难度
    int enemyCount;
    switch (ctx->currentLevel) {
    case 1:
        enemyCount = 1 + rand() % 2; // 1-2个敌人
        break;
    case 2:
        enemyCount = 2 + rand() % 2; // 2-3个敌人
        break;
    case 3:
        enemyCount = 3 + rand() % 2; // 3-4个敌人
        break;
    case 4:
        enemyCount = 4 + rand() % 2; // 4-5个敌人
        break;
    case 5:
        enemyCount = 5 + rand() % 2; // 5-6个敌人
        break;
    default:
        enemyCount = 2 + rand() % 2;
    }
    
    int i;

    for (i = 0; i < enemyCount; i++) {
        int row = rand() % GRID_ROWS;
        int col = rand() % GRID_COLS;

        if (row == GRID_ROWS / 2 && col == GRID_COLS / 2) continue;
        if (ctx->map[row][col] == WALL) continue;

        EnemyType type;
        int randValue = rand() % 100;
        // 根据关卡调整强大敌人的出现概率，降低整体难度
        int strongEnemyChance = 5 + (ctx->currentLevel - 1) * 8;
        if (randValue < strongEnemyChance) {
            // 随着关卡增加，更可能出现强大的敌人
            type = (EnemyType)(3 + rand() % 3); // TANK, ASSASSIN, SUMMONER
        } else {
            type = (EnemyType)(rand() % 3); // BANDIT, BT, SNIPER
        }
        addEnemy(ctx, row, col, type);
    }
}

void moveSam(GameContext* ctx, Sam* sam, int drow, int dcol) {
    int newRow = sam->pos.row + drow;
    int newCol = sam->pos.col + dcol;

    if (newRow >= 0 && newRow < GRID_ROWS && newCol >= 0 && newCol < GRID_COLS && ctx->map[newRow][newCol] != WALL) {
        // 检查是否与Partner重合
        if (ctx->gameMode != MODE_DOUBLE || (newRow != ctx->partner.pos.row || newCol != ctx->partner.pos.col)) {
            sam->pos.row = newRow;
            sam->pos.col = newCol;

            if (drow == -1) sam->direction = 0;
            else if (dcol == 1) sam->direction = 1;
            else if (drow == 1) sam->direction = 2;
            else if (dcol == -1) sam->direction = 3;
        }
    }
}

void meleeAttack(GameContext* ctx, Sam* sam) {
    int targetRow = sam->pos.row;
    int targetCol = sam->pos.col;

    switch (sam->direction) {
    case 0: targetRow--; break;
    case 1: targetCol++; break;
    case 2: targetRow++; break;
    case 3: targetCol--; break;
    }

    for (int i = 0; i < 20; i++) {
        if (ctx->enemies[i].active && 
            ctx->enemies[i].pos.row == targetRow && 
            ctx->enemies[i].pos.col == targetCol) {
            ctx->enemies[i].health--;
            if (ctx->enemies[i].health <= 0) {
                int baseScore = 40;
                switch (sam->weapon) {
                case WEAPON_SUPER: baseScore += 20;
                    break;
                case WEAPON_ULTIMATE: baseScore += 40;
                    break;
                default:
                    break;
                }
                sam->score += baseScore;
                Beep(600, 100);
                removeEnemy(ctx, i);
            }
            break;
        }
    }
}

void shootBullet(GameContext* ctx, Bullet bullets[], Sam* sam) {
    int i;
    for (i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) {
            bullets[i].x = (float)(GAME_AREA_X + sam->pos.col * CELL_SIZE + CELL_SIZE / 2);
            bullets[i].y = (float)(GAME_AREA_Y + sam->pos.row * CELL_SIZE + CELL_SIZE / 2);

            float speed = 8.0f;
            switch (sam->weapon) {
            case WEAPON_SUPER: speed = 10.0f;
                break;
            case WEAPON_ULTIMATE: speed = 12.0f;
                break;
            default:
                break;
            }

            bullets[i].dx = 0;
            bullets[i].dy = 0;
            switch (sam->direction) {
            case 0: bullets[i].dy = -speed; break;
            case 1: bullets[i].dx = speed; break;
            case 2: bullets[i].dy = speed; break;
            case 3: bullets[i].dx = -speed; break;
            }

            bullets[i].active = 1;
            bullets[i].fromPlayer = 1;
            bullets[i].frame = 0;
            Beep(800, 50);
            break;
        }
    }
}

void enemyShootBullet(GameContext* ctx, Bullet bullets[], Enemy* enemy, Sam* sam) {
    int i;
    for (i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) {
            bullets[i].x = (float)(GAME_AREA_X + enemy->pos.col * CELL_SIZE + CELL_SIZE / 2);
            bullets[i].y = (float)(GAME_AREA_Y + enemy->pos.row * CELL_SIZE + CELL_SIZE / 2);

            float dx = (float)(sam->pos.col - enemy->pos.col);
            float dy = (float)(sam->pos.row - enemy->pos.row);
            float dist = sqrtf(dx * dx + dy * dy);

            if (dist > 0) {
                bullets[i].dx = (dx / dist) * 5.0f;
                bullets[i].dy = (dy / dist) * 5.0f;
            }

            bullets[i].active = 1;
            bullets[i].fromPlayer = 0;
            bullets[i].frame = 0;
            Beep(400, 50);
            break;
        }
    }
}

void updateBullets(GameContext* ctx, Bullet bullets[], Sam* sam) {
    int i;
    for (i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            bullets[i].x += bullets[i].dx;
            bullets[i].y += bullets[i].dy;
            bullets[i].frame++;

            if (bullets[i].x < GAME_AREA_X || bullets[i].x > GAME_AREA_X + GRID_COLS * CELL_SIZE ||
                bullets[i].y < GAME_AREA_Y || bullets[i].y > GAME_AREA_Y + GRID_ROWS * CELL_SIZE) {
                bullets[i].active = 0;
                continue;
            }
            
            // 检查子弹是否碰到墙壁
            int cellRow = (int)((bullets[i].y - GAME_AREA_Y) / CELL_SIZE);
            int cellCol = (int)((bullets[i].x - GAME_AREA_X) / CELL_SIZE);
            if (cellRow >= 0 && cellRow < GRID_ROWS && cellCol >= 0 && cellCol < GRID_COLS && ctx->map[cellRow][cellCol] == WALL) {
                bullets[i].active = 0;
                continue;
            }

            if (bullets[i].fromPlayer) {
                for (int j = 0; j < 20; j++) {
                    if (ctx->enemies[j].active) {
                        Enemy* current = &ctx->enemies[j];
                        int enemyX = GAME_AREA_X + current->pos.col * CELL_SIZE + CELL_SIZE / 2;
                        int enemyY = GAME_AREA_Y + current->pos.row * CELL_SIZE + CELL_SIZE / 2;

                        float dist = sqrtf(powf(bullets[i].x - enemyX, 2.0f) + powf(bullets[i].y - enemyY, 2.0f));

                        if (dist < 30) {
                            bullets[i].active = 0;
                            current->health--;

                            if (current->health <= 0) {
                                int baseScore = 25;
                                switch (sam->weapon) {
                                case WEAPON_SUPER: baseScore += 15;
                                    break;
                                case WEAPON_ULTIMATE: baseScore += 30;
                                    break;
                                default:
                                    break;
                                }
                                sam->score += baseScore;
                                Beep(500, 80);
                                removeEnemy(ctx, j);
                            }
                            break;
                        }
                    }
                }
            }
            else {
                // 敌人的子弹只会伤害玩家，不会伤害合作者
                int samX = GAME_AREA_X + sam->pos.col * CELL_SIZE + CELL_SIZE / 2;
                int samY = GAME_AREA_Y + sam->pos.row * CELL_SIZE + CELL_SIZE / 2;

                float dist = sqrtf(powf(bullets[i].x - samX, 2.0f) + powf(bullets[i].y - samY, 2.0f));

                if (dist < 30) {
                    bullets[i].active = 0;
                    int scorePenalty = 15;
                    int healthPenalty = 10;
                    switch (sam->armor) {
                    case ARMOR_LIGHT: 
                        scorePenalty = 10;
                        healthPenalty = 7;
                        break;
                    case ARMOR_HEAVY: 
                        scorePenalty = 5;
                        healthPenalty = 5;
                        break;
                    default:
                        break;
                    }
                    sam->score -= scorePenalty;
                    sam->health -= healthPenalty;
                    if (sam->score < 0) sam->score = 0;
                    Beep(300, 100);
                }
            }
        }
    }
}

void updateEnemies(GameContext* ctx, Bullet bullets[], Sam* sam) {
    for (int i = 0; i < 20; i++) {
        if (ctx->enemies[i].active) {
            Enemy* current = &ctx->enemies[i];
            current->moveTimer++;
            current->shootTimer++;
            current->summonTimer++;

            switch (current->type) {
            case ENEMY_BANDIT: {
                if (current->moveTimer > 60) {
                    int drow = 0, dcol = 0;

                    if (sam->pos.row > current->pos.row) drow = 1;
                    else if (sam->pos.row < current->pos.row) drow = -1;

                    if (sam->pos.col > current->pos.col) dcol = 1;
                    else if (sam->pos.col < current->pos.col) dcol = -1;

                    if (rand() % 2 == 0 && drow != 0) {
                        int newRow = current->pos.row + drow;
                        int newCol = current->pos.col;
                        if (newRow >= 0 && newRow < GRID_ROWS && 
                            getEnemyAt(ctx, newRow, newCol) == NULL && 
                            ctx->map[newRow][newCol] != WALL &&
                            !(newRow == sam->pos.row && newCol == sam->pos.col)) {
                            current->pos.row = newRow;
                        }
                    }
                    else if (dcol != 0) {
                        int newRow = current->pos.row;
                        int newCol = current->pos.col + dcol;
                        if (newCol >= 0 && newCol < GRID_COLS && 
                            getEnemyAt(ctx, newRow, newCol) == NULL && 
                            ctx->map[newRow][newCol] != WALL &&
                            !(newRow == sam->pos.row && newCol == sam->pos.col)) {
                            current->pos.col = newCol;
                        }
                    }

                    current->moveTimer = 0;
                }

                if (abs(current->pos.row - sam->pos.row) <= 1 && abs(current->pos.col - sam->pos.col) <= 1) {
                    if (current->pos.row != sam->pos.row || current->pos.col != sam->pos.col) {
                        if (current->shootTimer > 90) {
                            int scorePenalty = 15;
                            int healthPenalty = 15;
                            switch (sam->armor) {
                            case ARMOR_LIGHT: 
                                scorePenalty = 10;
                                healthPenalty = 10;
                                break;
                            case ARMOR_HEAVY: 
                                scorePenalty = 5;
                                healthPenalty = 7;
                                break;
                            default:
                                break;
                            }
                            sam->score -= scorePenalty;
                            sam->health -= healthPenalty;
                            if (sam->score < 0) sam->score = 0;
                            Beep(250, 100);
                            current->shootTimer = 0;
                        }
                    }
                }
                break;
            }

            case ENEMY_BT: {
                if (current->shootTimer > 120) {
                    enemyShootBullet(ctx, bullets, current, sam);
                    current->shootTimer = 0;
                }
                break;
            }

            case ENEMY_SNIPER: {
                if (current->shootTimer > 150) {
                    enemyShootBullet(ctx, bullets, current, sam);
                    current->shootTimer = 0;
                }
                break;
            }

            case ENEMY_TANK: {
                if (current->moveTimer > 120) {
                    int drow = 0;

                    if (sam->pos.row > current->pos.row) drow = 1;
                    else if (sam->pos.row < current->pos.row) drow = -1;

                    if (drow != 0) {
                        int newRow = current->pos.row + drow;
                        int newCol = current->pos.col;
                        if (newRow >= 0 && newRow < GRID_ROWS && 
                            getEnemyAt(ctx, newRow, newCol) == NULL && 
                            ctx->map[newRow][newCol] != WALL &&
                            !(newRow == sam->pos.row && newCol == sam->pos.col)) {
                            current->pos.row = newRow;
                        }
                    }

                    current->moveTimer = 0;
                }

                if (abs(current->pos.row - sam->pos.row) <= 1 && abs(current->pos.col - sam->pos.col) <= 1) {
                    if (current->pos.row != sam->pos.row || current->pos.col != sam->pos.col) {
                        if (current->shootTimer > 100) {
                            int scorePenalty = 20;
                            int healthPenalty = 25;
                            switch (sam->armor) {
                            case ARMOR_LIGHT: 
                                scorePenalty = 15;
                                healthPenalty = 18;
                                break;
                            case ARMOR_HEAVY: 
                                scorePenalty = 10;
                                healthPenalty = 12;
                                break;
                            default:
                                break;
                            }
                            sam->score -= scorePenalty;
                            sam->health -= healthPenalty;
                            if (sam->score < 0) sam->score = 0;
                            Beep(200, 120);
                            current->shootTimer = 0;
                        }
                    }
                }
                break;
            }

            case ENEMY_ASSASSIN: {
                if (current->moveTimer > 30) {
                    int drow = 0, dcol = 0;

                    if (sam->pos.row > current->pos.row) drow = 1;
                    else if (sam->pos.row < current->pos.row) drow = -1;

                    if (sam->pos.col > current->pos.col) dcol = 1;
                    else if (sam->pos.col < current->pos.col) dcol = -1;

                    if (dcol != 0) {
                        int newRow = current->pos.row;
                        int newCol = current->pos.col + dcol;
                        if (newCol >= 0 && newCol < GRID_COLS && 
                            getEnemyAt(ctx, newRow, newCol) == NULL && 
                            ctx->map[newRow][newCol] != WALL &&
                            !(newRow == sam->pos.row && newCol == sam->pos.col)) {
                            current->pos.col = newCol;
                        }
                    }
                    else if (drow != 0) {
                        int newRow = current->pos.row + drow;
                        int newCol = current->pos.col;
                        if (newRow >= 0 && newRow < GRID_ROWS && 
                            getEnemyAt(ctx, newRow, newCol) == NULL && 
                            ctx->map[newRow][newCol] != WALL &&
                            !(newRow == sam->pos.row && newCol == sam->pos.col)) {
                            current->pos.row = newRow;
                        }
                    }

                    current->moveTimer = 0;
                }

                if (abs(current->pos.row - sam->pos.row) <= 1 && abs(current->pos.col - sam->pos.col) <= 1) {
                    if (current->pos.row != sam->pos.row || current->pos.col != sam->pos.col) {
                        if (current->shootTimer > 60) {
                            int scorePenalty = 15;
                            int healthPenalty = 12;
                            switch (sam->armor) {
                            case ARMOR_LIGHT: 
                                scorePenalty = 10;
                                healthPenalty = 8;
                                break;
                            case ARMOR_HEAVY: 
                                scorePenalty = 5;
                                healthPenalty = 5;
                                break;
                            default:
                                break;
                            }
                            sam->score -= scorePenalty;
                            sam->health -= healthPenalty;
                            if (sam->score < 0) sam->score = 0;
                            Beep(280, 80);
                            current->shootTimer = 0;
                        }
                    }
                }
                break;
            }

            case ENEMY_SUMMONER: {
                if (current->shootTimer > 100) {
                    enemyShootBullet(ctx, bullets, current, sam);
                    current->shootTimer = 0;
                }

                if (current->summonTimer > 200 && countEnemies(ctx) < 10) {
                    int summonRow = current->pos.row + (rand() % 3 - 1);
                    int summonCol = current->pos.col + (rand() % 3 - 1);

                    if (summonRow >= 0 && summonRow < GRID_ROWS &&
                        summonCol >= 0 && summonCol < GRID_COLS &&
                        getEnemyAt(ctx, summonRow, summonCol) == NULL &&
                        ctx->map[summonRow][summonCol] != WALL) {
                        addEnemy(ctx, summonRow, summonCol, ENEMY_BANDIT);
                        Beep(500, 100);
                    }

                    current->summonTimer = 0;
                }
                break;
            }
            }
        }
    }
}

// ==================== 绘制函数 ====================

void drawGrid(GameContext* ctx) {
    int i, j;
    for (i = 0; i < GRID_ROWS; i++) {
        for (j = 0; j < GRID_COLS; j++) {
            int x = GAME_AREA_X + j * CELL_SIZE;
            int y = GAME_AREA_Y + i * CELL_SIZE;

            setlinecolor(RGB(100, 100, 100));
            setlinestyle(PS_SOLID, 2);
            rectangle(x, y, x + CELL_SIZE, y + CELL_SIZE);

            if (ctx->map[i][j] == WALL) {
                setfillcolor(RGB(80, 80, 80));
                solidrectangle(x + 2, y + 2, x + CELL_SIZE - 2, y + CELL_SIZE - 2);
                
                // 绘制墙壁纹理
                setfillcolor(RGB(60, 60, 60));
                int brickSize = 15;
                for (int bx = 0; bx < CELL_SIZE - 4; bx += brickSize) {
                    for (int by = 0; by < CELL_SIZE - 4; by += brickSize) {
                        solidrectangle(x + 2 + bx, y + 2 + by, x + 2 + bx + brickSize - 2, y + 2 + by + brickSize - 2);
                    }
                }
            } else {
                setfillcolor(RGB(40, 40, 50));
                solidrectangle(x + 2, y + 2, x + CELL_SIZE - 2, y + CELL_SIZE - 2);
            }
        }
    }
}

void drawSam(GameContext* ctx, Sam* sam) {
    int x = GAME_AREA_X + sam->pos.col * CELL_SIZE + CELL_SIZE / 2;
    int y = GAME_AREA_Y + sam->pos.row * CELL_SIZE + CELL_SIZE / 2;

    setfillcolor(RGB(0, 150, 255));
    solidcircle(x, y, 25);

    setlinecolor(YELLOW);
    setlinestyle(PS_SOLID, 3);
    int dx = 0, dy = 0;
    switch (sam->direction) {
    case 0: dy = -20; break;
    case 1: dx = 20; break;
    case 2: dy = 20; break;
    case 3: dx = -20; break;
    }
    line(x, y, x + dx, y + dy);

    settextstyle(16, 0, _T("宋体"));
    settextcolor(WHITE);
    outtextxy(x - 20, y - 45, _T("山姆"));
}

void drawPartner(Partner* partner) {
    int x = GAME_AREA_X + partner->pos.col * CELL_SIZE + CELL_SIZE / 2;
    int y = GAME_AREA_Y + partner->pos.row * CELL_SIZE + CELL_SIZE / 2;

    setfillcolor(RGB(255, 100, 0));
    solidcircle(x, y, 25);

    setlinecolor(YELLOW);
    setlinestyle(PS_SOLID, 3);
    int dx = 0, dy = 0;
    switch (partner->direction) {
    case 0: dy = -20; break;
    case 1: dx = 20; break;
    case 2: dy = 20; break;
    case 3: dx = -20; break;
    }
    line(x, y, x + dx, y + dy);

    settextstyle(16, 0, _T("宋体"));
    settextcolor(WHITE);
    outtextxy(x - 20, y - 45, _T("合作者"));
}

void drawEnemies(GameContext* ctx) {
    int i;

    for (int j = 0; j < 20; j++) {
        if (ctx->enemies[j].active) {
            Enemy* current = &ctx->enemies[j];
            int x = GAME_AREA_X + current->pos.col * CELL_SIZE + CELL_SIZE / 2;
            int y = GAME_AREA_Y + current->pos.row * CELL_SIZE + CELL_SIZE / 2;

            switch (current->type) {
            case ENEMY_BANDIT:
                setfillcolor(RED);
                solidrectangle(x - 20, y - 20, x + 20, y + 20);
                settextstyle(14, 0, _T("宋体"));
                settextcolor(WHITE);
                outtextxy(x - 20, y - 40, _T("强盗"));
                break;

            case ENEMY_BT: {
                setfillcolor(RGB(150, 0, 150));
                solidcircle(x, y, 22);

                setlinecolor(RGB(100, 0, 100));
                setlinestyle(PS_SOLID, 2);
                for (i = 0; i < 6; i++) {
                    float angle = i * 3.14159f / 3;
                    line(x, y,
                        x + (int)(cos(angle) * 30),
                        y + (int)(sin(angle) * 30));
                }

                settextstyle(14, 0, _T("宋体"));
                settextcolor(WHITE);
                outtextxy(x - 15, y - 40, _T("BT"));
                break;
            }

            case ENEMY_SNIPER: {
                setfillcolor(GREEN);
                POINT trianglePts[3];
                trianglePts[0].x = x;
                trianglePts[0].y = y - 22;
                trianglePts[1].x = x - 20;
                trianglePts[1].y = y + 22;
                trianglePts[2].x = x + 20;
                trianglePts[2].y = y + 22;
                solidpolygon(trianglePts, 3);

                settextstyle(12, 0, _T("宋体"));
                settextcolor(WHITE);
                outtextxy(x - 24, y - 40, _T("狙击手"));
                break;
            }

            case ENEMY_TANK: {
                setfillcolor(RGB(255, 140, 0));
                solidrectangle(x - 28, y - 28, x + 28, y + 28);

                float healthPercent = (float)current->health / current->maxHealth;
                setlinecolor(RGB(50, 50, 50));
                setlinestyle(PS_SOLID, 4);
                line(x - 28, y - 35, x + 28, y - 35);
                setlinecolor(GREEN);
                line(x - 28, y - 35,
                    x - 28 + (int)(56 * healthPercent), y - 35);

                settextstyle(14, 0, _T("宋体"));
                settextcolor(WHITE);
                outtextxy(x - 20, y - 50, _T("坦克"));
                break;
            }

            case ENEMY_ASSASSIN: {
                setfillcolor(RGB(50, 50, 50));
                POINT diamondPts[4];
                diamondPts[0].x = x;
                diamondPts[0].y = y - 24;
                diamondPts[1].x = x + 24;
                diamondPts[1].y = y;
                diamondPts[2].x = x;
                diamondPts[2].y = y + 24;
                diamondPts[3].x = x - 24;
                diamondPts[3].y = y;
                solidpolygon(diamondPts, 4);

                settextstyle(14, 0, _T("宋体"));
                settextcolor(WHITE);
                outtextxy(x - 20, y - 40, _T("刺客"));
                break;
            }

            case ENEMY_SUMMONER: {
                setfillcolor(BLUE);
                POINT starPts[10];
                for (i = 0; i < 10; i++) {
                    float angle = i * 3.14159f / 5;
                    int radius = (i % 2 == 0) ? 25 : 12;
                    starPts[i].x = x + (int)(cos(angle) * radius);
                    starPts[i].y = y + (int)(sin(angle) * radius);
                }
                solidpolygon(starPts, 10);

                settextstyle(12, 0, _T("宋体"));
                settextcolor(WHITE);
                outtextxy(x - 24, y - 40, _T("召唤师"));
                break;
            }
            }
        }
    }
}

void drawBullets(GameContext* ctx, Bullet bullets[]) {
    int i;
    for (i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            if (bullets[i].fromPlayer) {
                setfillcolor(YELLOW);
            }
            else {
                setfillcolor(RED);
            }
            solidcircle((int)bullets[i].x, (int)bullets[i].y, 5);
        }
    }
}

void drawButton(GameContext* ctx, Button* btn) {
    if (btn->enabled) {
        setfillcolor(RGB(70, 130, 180));
        setlinecolor(WHITE);
    }
    else {
        setfillcolor(RGB(100, 100, 100));
        setlinecolor(RGB(150, 150, 150));
    }

    setlinestyle(PS_SOLID, 2);
    fillrectangle(btn->x, btn->y, btn->x + btn->width, btn->y + btn->height);

    settextstyle(20, 0, _T("黑体"));
    settextcolor(WHITE);
    int textWidth = textwidth(btn->text);
    int textHeight = textheight(btn->text);
    outtextxy(btn->x + (btn->width - textWidth) / 2,
        btn->y + (btn->height - textHeight) / 2,
        btn->text);
}

int isMouseOnButton(GameContext* ctx, Button* btn, int mouseX, int mouseY) {
    return (mouseX >= btn->x && mouseX <= btn->x + btn->width &&
        mouseY >= btn->y && mouseY <= btn->y + btn->height);
}

void drawInfoArea(GameContext* ctx, Sam* sam) {
    setfillcolor(RGB(30, 30, 40));
    solidrectangle(700, 100, 1150, 650);

    setlinecolor(RGB(100, 100, 100));
    setlinestyle(PS_SOLID, 2);
    rectangle(700, 100, 1150, 650);

    settextstyle(25, 0, _T("黑体"));
    settextcolor(YELLOW);
    outtextxy(850, 120, _T("游戏信息"));

    settextstyle(20, 0, _T("宋体"));
    settextcolor(WHITE);

    TCHAR text[100];
    swprintf_s(text, 100, _T("玩家: %S"), ctx->currentUser);
    outtextxy(730, 180, text);

    swprintf_s(text, 100, _T("分数: %d"), sam->score);
    outtextxy(730, 220, text);

    swprintf_s(text, 100, _T("生命值: %d"), sam->health);
    if (sam->health > 60) settextcolor(GREEN);
    else if (sam->health > 30) settextcolor(YELLOW);
    else settextcolor(RED);
    outtextxy(730, 260, text);

    settextcolor(WHITE);
    swprintf_s(text, 100, _T("剩余时间: %d秒"), ctx->gameTimer);
    outtextxy(730, 300, text);

    swprintf_s(text, 100, _T("敌人数量: %d"), countEnemies(ctx));
    outtextxy(730, 340, text);

    // 显示关卡信息
    swprintf_s(text, 100, _T("当前关卡: %d"), ctx->currentLevel);
    outtextxy(730, 380, text);
    
    swprintf_s(text, 100, _T("下一关卡: %d分"), ctx->levelScoreThreshold);
    outtextxy(730, 410, text);

    // 显示装备信息
    settextcolor(YELLOW);
    swprintf_s(text, 100, _T("当前装备:"));
    outtextxy(730, 450, text);
    
    settextcolor(WHITE);
    const wchar_t* weaponName[] = {L"普通", L"超级", L"终极"};
    const wchar_t* armorName[] = {L"无", L"轻甲", L"重甲"};
    swprintf_s(text, 100, _T("武器: %s"), weaponName[sam->weapon]);
    outtextxy(730, 480, text);
    
    swprintf_s(text, 100, _T("护甲: %s"), armorName[sam->armor]);
    outtextxy(730, 510, text);

    settextstyle(18, 0, _T("宋体"));
    settextcolor(LIGHTGRAY);
    outtextxy(730, 550, _T("操作说明:"));
    outtextxy(730, 580, _T("WASD - 移动"));
    outtextxy(730, 610, _T("J - 近战攻击"));
    outtextxy(730, 640, _T("K - 射击"));
    
    if (ctx->gameMode == MODE_DOUBLE) {
        outtextxy(730, 670, _T("合作者:"));
        outtextxy(730, 700, _T("方向键 - 移动"));
        outtextxy(730, 730, _T("1 - 近战攻击"));
        outtextxy(730, 760, _T("2 - 射击"));
    }
}

void drawUI(GameContext* ctx, Sam* sam, Partner* partner, Bullet bullets[]) {
    cleardevice();

    drawGrid(ctx);
    drawSam(ctx, sam);
    if (partner != NULL) {
        drawPartner(partner);
    }
    drawEnemies(ctx);
    drawBullets(ctx, bullets);

    Button pauseBtn = { 50, 20, 100, 50, _T("暂停"), 1 };
    Button endBtn = { 160, 20, 100, 50, _T("结束"), 1 };

    drawButton(ctx, &pauseBtn);
    drawButton(ctx, &endBtn);

    drawInfoArea(ctx, sam);

    FlushBatchDraw();
}

void loginScreen(GameContext* ctx) {
    cleardevice();
    setbkcolor(RGB(20, 20, 30));

    settextstyle(50, 0, _T("黑体"));
    settextcolor(YELLOW);
    outtextxy(400, 100, _T("《搁浅》游戏"));

    settextstyle(25, 0, _T("宋体"));
    settextcolor(WHITE);
    outtextxy(400, 250, _T("用户名:"));
    outtextxy(400, 320, _T("密码:"));

    char username[MAX_USERNAME] = "";
    char password[MAX_PASSWORD] = "";
    bool showError = false;

    setfillcolor(RGB(50, 50, 60));
    solidrectangle(520, 245, 720, 285);
    solidrectangle(520, 315, 720, 355);

    Button loginBtn = { 450, 420, 120, 50, _T("登录"), 1 };
    Button registerBtn = { 600, 420, 120, 50, _T("注册"), 1 };

    drawButton(ctx, &loginBtn);
    drawButton(ctx, &registerBtn);
    FlushBatchDraw();

    int inputMode = 0;
    int usernameLen = 0;
    int passwordLen = 0;
    int i;

    while (1) {
        if (_kbhit()) {
            char ch = _getch();

            if (ch == '\r') {
                if (inputMode == 0 && usernameLen > 0) {
                    inputMode = 1;
                }
                else if (inputMode == 1 && passwordLen > 0) {
                    if (loginUser(ctx, username, password)) {
                            ctx->gameState = STATE_MENU;
                            return;
                        }
                        else {
                            showError = true;
                        }
                }
            }
            else if (ch == '\b') {
                if (inputMode == 0 && usernameLen > 0) {
                    username[--usernameLen] = '\0';
                }
                else if (inputMode == 1 && passwordLen > 0) {
                    password[--passwordLen] = '\0';
                }
            }
            else if (ch == '\t') {
                inputMode = 1 - inputMode;
            }
            else {
                if (inputMode == 0 && usernameLen < MAX_USERNAME - 1) {
                    username[usernameLen++] = ch;
                    username[usernameLen] = '\0';
                }
                else if (inputMode == 1 && passwordLen < MAX_PASSWORD - 1) {
                    password[passwordLen++] = ch;
                    password[passwordLen] = '\0';
                }
            }
        }

        if (MouseHit()) {
            MOUSEMSG msg = GetMouseMsg();
            if (msg.uMsg == WM_LBUTTONDOWN) {
                if (isMouseOnButton(ctx, &loginBtn, msg.x, msg.y)) {
                    if (usernameLen > 0 && passwordLen > 0) {
                        if (loginUser(ctx, username, password)) {
                            ctx->gameState = STATE_MENU;
                            return;
                        }
                        else {
                            showError = true;
                        }
                    }
                }
                else if (isMouseOnButton(ctx, &registerBtn, msg.x, msg.y)) {
                    ctx->gameState = STATE_REGISTER;
                    return;
                }
                else if (msg.x >= 520 && msg.x <= 720 && msg.y >= 245 && msg.y <= 285) {
                    inputMode = 0;
                }
                else if (msg.x >= 520 && msg.x <= 720 && msg.y >= 315 && msg.y <= 355) {
                    inputMode = 1;
                }
            }
        }

        cleardevice();
        setbkcolor(RGB(20, 20, 30));

        settextstyle(50, 0, _T("黑体"));
        settextcolor(YELLOW);
        outtextxy(400, 100, _T("《搁浅》游戏"));

        settextstyle(25, 0, _T("宋体"));
        settextcolor(WHITE);
        outtextxy(400, 250, _T("用户名:"));
        outtextxy(400, 320, _T("密码:"));

        setfillcolor(RGB(50, 50, 60));
        solidrectangle(520, 245, 720, 285);
        solidrectangle(520, 315, 720, 355);

        settextstyle(20, 0, _T("宋体"));
        settextcolor(WHITE);

        TCHAR wUsername[MAX_USERNAME];
        TCHAR wPassword[MAX_PASSWORD];
        MultiByteToWideChar(CP_ACP, 0, username, -1, wUsername, MAX_USERNAME);
        outtextxy(530, 253, wUsername);

        for (i = 0; i < passwordLen; i++) {
            wPassword[i] = L'*';
        }
        wPassword[passwordLen] = L'\0';
        outtextxy(530, 323, wPassword);

        if (showError) {
            settextcolor(RED);
            outtextxy(450, 380, _T("用户名或密码错误!"));
        }

        drawButton(ctx, &loginBtn);
        drawButton(ctx, &registerBtn);
        FlushBatchDraw();

        Sleep(10);
    }
}

void registerScreen(GameContext* ctx) {
    cleardevice();

    settextstyle(50, 0, _T("黑体"));
    settextcolor(YELLOW);
    outtextxy(450, 100, _T("用户注册"));

    settextstyle(25, 0, _T("宋体"));
    settextcolor(WHITE);
    outtextxy(400, 250, _T("用户名:"));
    outtextxy(400, 320, _T("密码:"));

    char username[MAX_USERNAME] = "";
    char password[MAX_PASSWORD] = "";
    bool showError = false;
    bool showSuccess = false;

    setfillcolor(RGB(50, 50, 60));
    solidrectangle(520, 245, 720, 285);
    solidrectangle(520, 315, 720, 355);

    Button registerBtn = { 450, 420, 120, 50, _T("注册"), 1 };
    Button backBtn = { 600, 420, 120, 50, _T("返回"), 1 };

    drawButton(ctx, &registerBtn);
    drawButton(ctx, &backBtn);
    FlushBatchDraw();

    int inputMode = 0;
    int usernameLen = 0;
    int passwordLen = 0;
    int i;

    while (1) {
        if (_kbhit()) {
            char ch = _getch();

            if (ch == '\r') {
                if (inputMode == 0 && usernameLen > 0) {
                    inputMode = 1;
                }
                else if (inputMode == 1 && passwordLen > 0) {
                    if (registerUser(ctx, username, password)) {
                        showSuccess = true;
                        Sleep(1000);
                        ctx->gameState = STATE_LOGIN;
                        return;
                    }
                    else {
                        showError = true;
                    }
                }
            }
            else if (ch == '\b') {
                if (inputMode == 0 && usernameLen > 0) {
                    username[--usernameLen] = '\0';
                }
                else if (inputMode == 1 && passwordLen > 0) {
                    password[--passwordLen] = '\0';
                }
            }
            else if (ch == '\t') {
                inputMode = 1 - inputMode;
            }
            else {
                if (inputMode == 0 && usernameLen < MAX_USERNAME - 1) {
                    username[usernameLen++] = ch;
                    username[usernameLen] = '\0';
                }
                else if (inputMode == 1 && passwordLen < MAX_PASSWORD - 1) {
                    password[passwordLen++] = ch;
                    password[passwordLen] = '\0';
                }
            }
        }

        if (MouseHit()) {
            MOUSEMSG msg = GetMouseMsg();
            if (msg.uMsg == WM_LBUTTONDOWN) {
                if (isMouseOnButton(ctx, &registerBtn, msg.x, msg.y)) {
                    if (usernameLen > 0 && passwordLen > 0) {
                        if (registerUser(ctx, username, password)) {
                            showSuccess = true;
                            Sleep(1000);
                            ctx->gameState = STATE_LOGIN;
                            return;
                        }
                        else {
                            showError = true;
                        }
                    }
                }
                else if (isMouseOnButton(ctx, &backBtn, msg.x, msg.y)) {
                    ctx->gameState = STATE_LOGIN;
                    return;
                }
                else if (msg.x >= 520 && msg.x <= 720 && msg.y >= 245 && msg.y <= 285) {
                    inputMode = 0;
                }
                else if (msg.x >= 520 && msg.x <= 720 && msg.y >= 315 && msg.y <= 355) {
                    inputMode = 1;
                }
            }
        }

        cleardevice();
        setbkcolor(RGB(20, 20, 30));

        settextstyle(50, 0, _T("黑体"));
        settextcolor(YELLOW);
        outtextxy(450, 100, _T("用户注册"));

        settextstyle(25, 0, _T("宋体"));
        settextcolor(WHITE);
        outtextxy(400, 250, _T("用户名:"));
        outtextxy(400, 320, _T("密码:"));

        setfillcolor(RGB(50, 50, 60));
        solidrectangle(520, 245, 720, 285);
        solidrectangle(520, 315, 720, 355);

        settextstyle(20, 0, _T("宋体"));
        settextcolor(WHITE);

        TCHAR wUsername[MAX_USERNAME];
        TCHAR wPassword[MAX_PASSWORD];
        MultiByteToWideChar(CP_ACP, 0, username, -1, wUsername, MAX_USERNAME);
        outtextxy(530, 253, wUsername);

        for (i = 0; i < passwordLen; i++) {
            wPassword[i] = L'*';
        }
        wPassword[passwordLen] = L'\0';
        outtextxy(530, 323, wPassword);

        if (showError) {
            settextcolor(RED);
            outtextxy(450, 380, _T("用户名已存在!"));
        }
        if (showSuccess) {
            settextcolor(GREEN);
            outtextxy(450, 380, _T("注册成功!"));
        }

        drawButton(ctx, &registerBtn);
        drawButton(ctx, &backBtn);
        FlushBatchDraw();

        Sleep(10);
    }
}

void menuScreen(GameContext* ctx) {
    cleardevice();

    settextstyle(60, 0, _T("黑体"));
    settextcolor(YELLOW);
    outtextxy(350, 80, _T("《搁浅》主菜单"));

    TCHAR welcomeText[100];
    swprintf_s(welcomeText, 100, _T("欢迎, %S!"), ctx->currentUser);
    settextstyle(25, 0, _T("宋体"));
    settextcolor(WHITE);
    outtextxy(480, 160, welcomeText);

    // 检查是否存在存档文件
    FILE* saveFile = fopen("savegame.dat", "r");
    bool hasSave = (saveFile != NULL);
    if (saveFile != NULL) {
        fclose(saveFile);
    }

    Button startBtn = { 450, 220, 300, 60, _T("开始游戏"), 1 };
    Button continueBtn = { 450, 300, 300, 60, _T("继续游戏"), hasSave };
    Button shopBtn = { 450, 380, 300, 60, _T("装备商店"), 1 };
    Button rankingBtn = { 450, 460, 300, 60, _T("排行榜"), 1 };
    Button exitBtn = { 450, 540, 300, 60, _T("退出游戏"), 1 };

    drawButton(ctx, &startBtn);
    drawButton(ctx, &continueBtn);
    drawButton(ctx, &shopBtn);
    drawButton(ctx, &rankingBtn);
    drawButton(ctx, &exitBtn);
    FlushBatchDraw();

    while (1) {
        if (MouseHit()) {
            MOUSEMSG msg = GetMouseMsg();
            if (msg.uMsg == WM_LBUTTONDOWN) {
                if (isMouseOnButton(ctx, &startBtn, msg.x, msg.y)) {
                    ctx->gameState = STATE_MODE_SELECT;
                    return;
                }
                else if (isMouseOnButton(ctx, &continueBtn, msg.x, msg.y) && hasSave) {
                    if (loadGameState(ctx)) {
                        ctx->gameState = STATE_PLAYING;
                        return;
                    }
                }
                else if (isMouseOnButton(ctx, &shopBtn, msg.x, msg.y)) {
                    ctx->gameState = STATE_SHOP;
                    return;
                }
                else if (isMouseOnButton(ctx, &rankingBtn, msg.x, msg.y)) {
                    ctx->gameState = STATE_RANKING;
                    return;
                }
                else if (isMouseOnButton(ctx, &exitBtn, msg.x, msg.y)) {
                    ctx->gameState = STATE_EXIT;
                    return;
                }
            }
        }
        Sleep(10);
    }
}

void initPartner(Partner* partner) {
    partner->pos.row = GRID_ROWS / 2;
    partner->pos.col = GRID_COLS / 2 + 1;
    partner->health = 100;
    partner->direction = 2;
    partner->weapon = WEAPON_NORMAL;
    partner->armor = ARMOR_NONE;
}

void movePartner(GameContext* ctx, Partner* partner, int drow, int dcol) {
    int newRow = partner->pos.row + drow;
    int newCol = partner->pos.col + dcol;

    if (newRow >= 0 && newRow < GRID_ROWS && newCol >= 0 && newCol < GRID_COLS && ctx->map[newRow][newCol] != WALL) {
        // 检查是否与Sam重合
        if (newRow != ctx->sam.pos.row || newCol != ctx->sam.pos.col) {
            partner->pos.row = newRow;
            partner->pos.col = newCol;

            if (drow == -1) partner->direction = 0;
            else if (dcol == 1) partner->direction = 1;
            else if (drow == 1) partner->direction = 2;
            else if (dcol == -1) partner->direction = 3;
        }
    }
}

void partnerMeleeAttack(GameContext* ctx, Partner* partner, Sam* sam) {
    int targetRow = partner->pos.row;
    int targetCol = partner->pos.col;

    switch (partner->direction) {
    case 0: targetRow--; break;
    case 1: targetCol++; break;
    case 2: targetRow++; break;
    case 3: targetCol--; break;
    }

    for (int i = 0; i < 20; i++) {
        if (ctx->enemies[i].active && 
            ctx->enemies[i].pos.row == targetRow && 
            ctx->enemies[i].pos.col == targetCol) {
            ctx->enemies[i].health--;
            if (ctx->enemies[i].health <= 0) {
                int baseScore = 40;
                switch (partner->weapon) {
                case WEAPON_SUPER: baseScore += 20;
                    break;
                case WEAPON_ULTIMATE: baseScore += 40;
                    break;
                default:
                    break;
                }
                sam->score += baseScore;
                Beep(600, 100);
                removeEnemy(ctx, i);
            }
            break;
        }
    }
}

void partnerShootBullet(GameContext* ctx, Bullet bullets[], Partner* partner, Sam* sam) {
    int i;
    for (i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) {
            bullets[i].x = (float)(GAME_AREA_X + partner->pos.col * CELL_SIZE + CELL_SIZE / 2);
            bullets[i].y = (float)(GAME_AREA_Y + partner->pos.row * CELL_SIZE + CELL_SIZE / 2);

            float speed = 8.0f;
            switch (partner->weapon) {
            case WEAPON_SUPER: speed = 10.0f;
                break;
            case WEAPON_ULTIMATE: speed = 12.0f;
                break;
            default:
                break;
            }

            bullets[i].dx = 0;
            bullets[i].dy = 0;
            switch (partner->direction) {
            case 0: bullets[i].dy = -speed; break;
            case 1: bullets[i].dx = speed; break;
            case 2: bullets[i].dy = speed; break;
            case 3: bullets[i].dx = -speed; break;
            }

            bullets[i].active = 1;
            bullets[i].fromPlayer = 1;
            bullets[i].frame = 0;
            Beep(800, 50);
            break;
        }
    }
}

void gameLoop(GameContext* ctx) {
    if (!ctx->initialized) {
        initSam(ctx, &ctx->sam);
        if (ctx->gameMode == MODE_DOUBLE) {
            initPartner(&ctx->partner);
        }
        initBullets(ctx, ctx->bullets);
        initMap(ctx);
        
        // 从用户数据中加载装备
        UserNode* user = findUser(ctx, ctx->currentUser);
        if (user != NULL) {
            ctx->sam.score = 0; // 每次游戏开始时得分重置为0
            ctx->sam.weapon = user->currentWeapon; // 加载用户购买的武器
            ctx->sam.armor = user->currentArmor; // 加载用户购买的护甲
            if (ctx->gameMode == MODE_DOUBLE) {
                ctx->partner.weapon = user->currentWeapon; // 合作者使用相同的武器
                ctx->partner.armor = user->currentArmor; // 合作者使用相同的护甲
            }
        }
        
        spawnEnemies(ctx);
        ctx->initialized = true;
        ctx->lastSecond = (int)time(NULL);
    }

    Button pauseBtn = { 50, 20, 100, 50, _T("暂停"), 1 };
    Button endBtn = { 160, 20, 100, 50, _T("结束"), 1 };

    // 使用GetAsyncKeyState处理键盘输入，配合按键状态追踪防止重复触发
    bool currentKeyState;
    
    // 第一个玩家控制（WASD键）
    currentKeyState = (GetAsyncKeyState('W') & 0x8000) != 0;
    if (currentKeyState && !ctx->keyStates['W']) { moveSam(ctx, &ctx->sam, -1, 0); ctx->keyStates['W'] = true; }
    else if (!currentKeyState) { ctx->keyStates['W'] = false; }
    
    currentKeyState = (GetAsyncKeyState('S') & 0x8000) != 0;
    if (currentKeyState && !ctx->keyStates['S']) { moveSam(ctx, &ctx->sam, 1, 0); ctx->keyStates['S'] = true; }
    else if (!currentKeyState) { ctx->keyStates['S'] = false; }
    
    currentKeyState = (GetAsyncKeyState('A') & 0x8000) != 0;
    if (currentKeyState && !ctx->keyStates['A']) { moveSam(ctx, &ctx->sam, 0, -1); ctx->keyStates['A'] = true; }
    else if (!currentKeyState) { ctx->keyStates['A'] = false; }
    
    currentKeyState = (GetAsyncKeyState('D') & 0x8000) != 0;
    if (currentKeyState && !ctx->keyStates['D']) { moveSam(ctx, &ctx->sam, 0, 1); ctx->keyStates['D'] = true; }
    else if (!currentKeyState) { ctx->keyStates['D'] = false; }
    
    currentKeyState = (GetAsyncKeyState('J') & 0x8000) != 0;
    if (currentKeyState && !ctx->keyStates['J']) { meleeAttack(ctx, &ctx->sam); ctx->keyStates['J'] = true; }
    else if (!currentKeyState) { ctx->keyStates['J'] = false; }
    
    currentKeyState = (GetAsyncKeyState('K') & 0x8000) != 0;
    if (currentKeyState && !ctx->keyStates['K']) { shootBullet(ctx, ctx->bullets, &ctx->sam); ctx->keyStates['K'] = true; }
    else if (!currentKeyState) { ctx->keyStates['K'] = false; }
    
    // 双人模式下，第二个玩家控制（方向键和数字键）
    if (ctx->gameMode == MODE_DOUBLE) {
        currentKeyState = (GetAsyncKeyState(VK_UP) & 0x8000) != 0;
        if (currentKeyState && !ctx->keyStates[VK_UP]) { movePartner(ctx, &ctx->partner, -1, 0); ctx->keyStates[VK_UP] = true; }
        else if (!currentKeyState) { ctx->keyStates[VK_UP] = false; }
        
        currentKeyState = (GetAsyncKeyState(VK_DOWN) & 0x8000) != 0;
        if (currentKeyState && !ctx->keyStates[VK_DOWN]) { movePartner(ctx, &ctx->partner, 1, 0); ctx->keyStates[VK_DOWN] = true; }
        else if (!currentKeyState) { ctx->keyStates[VK_DOWN] = false; }
        
        currentKeyState = (GetAsyncKeyState(VK_LEFT) & 0x8000) != 0;
        if (currentKeyState && !ctx->keyStates[VK_LEFT]) { movePartner(ctx, &ctx->partner, 0, -1); ctx->keyStates[VK_LEFT] = true; }
        else if (!currentKeyState) { ctx->keyStates[VK_LEFT] = false; }
        
        currentKeyState = (GetAsyncKeyState(VK_RIGHT) & 0x8000) != 0;
        if (currentKeyState && !ctx->keyStates[VK_RIGHT]) { movePartner(ctx, &ctx->partner, 0, 1); ctx->keyStates[VK_RIGHT] = true; }
        else if (!currentKeyState) { ctx->keyStates[VK_RIGHT] = false; }
        
        currentKeyState = (GetAsyncKeyState('1') & 0x8000) != 0;
        if (currentKeyState && !ctx->keyStates['1']) { partnerMeleeAttack(ctx, &ctx->partner, &ctx->sam); ctx->keyStates['1'] = true; }
        else if (!currentKeyState) { ctx->keyStates['1'] = false; }
        
        currentKeyState = (GetAsyncKeyState('2') & 0x8000) != 0;
        if (currentKeyState && !ctx->keyStates['2']) { partnerShootBullet(ctx, ctx->bullets, &ctx->partner, &ctx->sam); ctx->keyStates['2'] = true; }
        else if (!currentKeyState) { ctx->keyStates['2'] = false; }
    } else {
        // 单人模式下，方向键也可以控制Sam移动
        currentKeyState = (GetAsyncKeyState(VK_UP) & 0x8000) != 0;
        if (currentKeyState && !ctx->keyStates[VK_UP]) { moveSam(ctx, &ctx->sam, -1, 0); ctx->keyStates[VK_UP] = true; }
        else if (!currentKeyState) { ctx->keyStates[VK_UP] = false; }
        
        currentKeyState = (GetAsyncKeyState(VK_DOWN) & 0x8000) != 0;
        if (currentKeyState && !ctx->keyStates[VK_DOWN]) { moveSam(ctx, &ctx->sam, 1, 0); ctx->keyStates[VK_DOWN] = true; }
        else if (!currentKeyState) { ctx->keyStates[VK_DOWN] = false; }
        
        currentKeyState = (GetAsyncKeyState(VK_LEFT) & 0x8000) != 0;
        if (currentKeyState && !ctx->keyStates[VK_LEFT]) { moveSam(ctx, &ctx->sam, 0, -1); ctx->keyStates[VK_LEFT] = true; }
        else if (!currentKeyState) { ctx->keyStates[VK_LEFT] = false; }
        
        currentKeyState = (GetAsyncKeyState(VK_RIGHT) & 0x8000) != 0;
        if (currentKeyState && !ctx->keyStates[VK_RIGHT]) { moveSam(ctx, &ctx->sam, 0, 1); ctx->keyStates[VK_RIGHT] = true; }
        else if (!currentKeyState) { ctx->keyStates[VK_RIGHT] = false; }
    }

    if (MouseHit()) {
        MOUSEMSG msg = GetMouseMsg();
        if (msg.uMsg == WM_LBUTTONDOWN) {
            if (isMouseOnButton(ctx, &pauseBtn, msg.x, msg.y)) {
                ctx->gameState = STATE_PAUSED;
                return;
            }
            else if (isMouseOnButton(ctx, &endBtn, msg.x, msg.y)) {
                updateHighScore(ctx, ctx->currentUser, ctx->sam.score, ctx->currentLevel);
                
                // 检查是否达到通关分数
                int winScore[] = {40, 80, 120, 160, 200};
                UserNode* user = findUser(ctx, ctx->currentUser);
                if (ctx->sam.score >= winScore[ctx->currentLevel - 1]) {
                    // 解锁下一关卡
                    if (user != NULL && ctx->currentLevel < 5) {
                        // 只有当新的关卡数大于当前已解锁的关卡数时，才更新 unlockedLevels
                        if (ctx->currentLevel + 1 > user->unlockedLevels) {
                            user->unlockedLevels = ctx->currentLevel + 1;
                            saveUsersToFile(ctx);
                        }
                    }
                    
                    // 显示胜利界面
                    int finalScore = ctx->sam.score;
                    // 重置武器和护甲为默认值
                    UserNode* user = findUser(ctx, ctx->currentUser);
                    if (user != NULL) {
                        user->currentWeapon = WEAPON_NORMAL;
                        user->currentArmor = ARMOR_NONE;
                        saveUsersToFile(ctx);
                    }
                    ctx->initialized = false;
                    clearEnemyList(ctx);
                    winScreen(ctx, finalScore);
                    return;
                } else {
                    // 将得分添加到总积分
                    if (user != NULL) {
                        user->totalScore += ctx->sam.score;
                        // 重置武器和护甲为默认值
                        user->currentWeapon = WEAPON_NORMAL;
                        user->currentArmor = ARMOR_NONE;
                        saveUsersToFile(ctx);
                    }
                    ctx->initialized = false;
                    clearEnemyList(ctx);
                    ctx->currentLevel = 1;
                    ctx->levelScoreThreshold = 100;
                    ctx->gameState = STATE_LOSE;
                    return;
                }
            }
        }
    }

    updateBullets(ctx, ctx->bullets, &ctx->sam);
    updateEnemies(ctx, ctx->bullets, &ctx->sam);

    if (ctx->gameMode == MODE_DOUBLE) {
        drawUI(ctx, &ctx->sam, &ctx->partner, ctx->bullets);
    } else {
        drawUI(ctx, &ctx->sam, NULL, ctx->bullets);
    }

    int currentSecond = (int)time(NULL);//倒计时 当前秒和上一秒不一样
    if (currentSecond != ctx->lastSecond) {
        ctx->gameTimer--;
        ctx->lastSecond = currentSecond;
    }

    if (countEnemies(ctx) < 3) {
        spawnEnemies(ctx);
    }
    
    // 关卡通关条件
    int winScore[] = {40, 80, 120, 160, 200};//
    // 调试信息：显示当前关卡和分数
    TCHAR debugText[100];
    swprintf_s(debugText, 100, _T("关卡: %d, 分数: %d, 目标: %d"), ctx->currentLevel, ctx->sam.score, winScore[ctx->currentLevel - 1]);
    outtextxy(730, 640, debugText);
    
    // 游戏结束条件：玩家主动结束或时间耗尽
    if (ctx->sam.health <= 0 || ctx->gameTimer <= 0) {
        updateHighScore(ctx, ctx->currentUser, ctx->sam.score, ctx->currentLevel);
        
        // 检查是否达到通关分数
        UserNode* user = findUser(ctx, ctx->currentUser);
        if (ctx->sam.score >= winScore[ctx->currentLevel - 1]) {
            // 解锁下一关卡
            if (user != NULL && ctx->currentLevel < 5) {
                // 只有当新的关卡数大于当前已解锁的关卡数时，才更新 unlockedLevels
                if (ctx->currentLevel + 1 > user->unlockedLevels) {
                    user->unlockedLevels = ctx->currentLevel + 1;
                    saveUsersToFile(ctx);
                }
            }
            
            // 显示胜利界面
            int finalScore = ctx->sam.score;
            // 重置武器和护甲为默认值
            UserNode* user = findUser(ctx, ctx->currentUser);
            if (user != NULL) {
                user->currentWeapon = WEAPON_NORMAL;
                user->currentArmor = ARMOR_NONE;
                saveUsersToFile(ctx);
            }
            ctx->initialized = false;
            clearEnemyList(ctx);
            winScreen(ctx, finalScore);
            return;
        } else {
            // 显示失败界面
            if (user != NULL) {
                user->totalScore += ctx->sam.score;
                // 重置武器和护甲为默认值
                user->currentWeapon = WEAPON_NORMAL;
                user->currentArmor = ARMOR_NONE;
                saveUsersToFile(ctx);
            }
            ctx->initialized = false;
            clearEnemyList(ctx);
            ctx->currentLevel = 1;
            ctx->levelScoreThreshold = 100;
            ctx->gameState = STATE_LOSE;
            return;
        }
    }

    Sleep(10);
}

void pauseScreen(GameContext* ctx) {
    cleardevice();

    settextstyle(50, 0, _T("黑体"));
    settextcolor(YELLOW);
    outtextxy(500, 200, _T("游戏暂停"));

    Button resumeBtn = { 450, 320, 300, 60, _T("继续游戏"), 1 };
    Button saveBtn = { 450, 410, 300, 60, _T("存档"), 1 };
    Button menuBtn = { 450, 500, 300, 60, _T("返回主菜单"), 1 };

    drawButton(ctx, &resumeBtn);
    drawButton(ctx, &saveBtn);
    drawButton(ctx, &menuBtn);
    FlushBatchDraw();

    while (1) {
        if (MouseHit()) {
            MOUSEMSG msg = GetMouseMsg();
            if (msg.uMsg == WM_LBUTTONDOWN) {
                if (isMouseOnButton(ctx, &resumeBtn, msg.x, msg.y)) {
                    ctx->gameState = STATE_PLAYING;
                    return;
                }
                else if (isMouseOnButton(ctx, &saveBtn, msg.x, msg.y)) {
                    saveGameState(ctx);
                    // 显示保存成功提示
                    settextcolor(GREEN);
                    settextstyle(25, 0, _T("宋体"));
                    outtextxy(500, 280, _T("存档成功!"));
                    FlushBatchDraw();
                    Sleep(1000);
                    // 重绘界面
                    cleardevice();
                    settextstyle(50, 0, _T("黑体"));
                    settextcolor(YELLOW);
                    outtextxy(500, 200, _T("游戏暂停"));
                    drawButton(ctx, &resumeBtn);
                    drawButton(ctx, &saveBtn);
                    drawButton(ctx, &menuBtn);
                    FlushBatchDraw();
                }
                else if (isMouseOnButton(ctx, &menuBtn, msg.x, msg.y)) {
                    clearEnemyList(ctx);
                    ctx->gameState = STATE_MENU;
                    return;
                }
            }
        }
        Sleep(10);
    }
}

void winScreen(GameContext* ctx, int score) {
    cleardevice();

    settextstyle(60, 0, _T("黑体"));
    settextcolor(YELLOW);
    outtextxy(450, 200, _T("胜利!"));

    settextstyle(30, 0, _T("宋体"));
    settextcolor(WHITE);
    outtextxy(400, 320, _T("恭喜通关!"));
    
    // 显示当前局的分数
    TCHAR scoreText[100];
    swprintf_s(scoreText, 100, _T("本轮分数: %d"), score);
    outtextxy(450, 380, scoreText);
    
    // 显示获得的积分
    int bonus = 0;
    switch (ctx->currentLevel) {
    case 1: bonus = 40; break;
    case 2: bonus = 80; break;
    case 3: bonus = 120; break;
    case 4: bonus = 160; break;
    case 5: bonus = 200; break;
    }
    swprintf_s(scoreText, 100, _T("获得积分: %d"), bonus);
    outtextxy(450, 420, scoreText);

    settextcolor(LIGHTGRAY);
    settextstyle(20, 0, _T("宋体"));
    outtextxy(450, 500, _T("按任意键返回主菜单..."));
    FlushBatchDraw();
    
    // 延长显示时间，等待3秒后再显示按任意键提示
    Sleep(3000);

    _getch();
    ctx->gameState = STATE_MENU;
}

void loseScreen(GameContext* ctx) {
    cleardevice();

    settextstyle(60, 0, _T("黑体"));
    settextcolor(RED);
    outtextxy(450, 200, _T("游戏结束"));

    settextstyle(30, 0, _T("宋体"));
    settextcolor(WHITE);
    outtextxy(400, 320, _T("再接再厉!"));

    settextcolor(LIGHTGRAY);
    settextstyle(20, 0, _T("宋体"));
    outtextxy(450, 500, _T("按任意键返回主菜单..."));
    FlushBatchDraw();

    _getch();
    ctx->gameState = STATE_MENU;
}

void buyWeapon(GameContext* ctx, Sam* sam, WeaponType weapon) {
    int cost = 0;
    switch (weapon) {
    case WEAPON_SUPER:
        cost = 100;
        break;
    case WEAPON_ULTIMATE:
        cost = 200;
        break;
    default:
        return;
    }
    
    if (sam->score >= cost) {
        sam->score -= cost;
        sam->weapon = weapon;
        Beep(900, 100);
    }
}

void buyArmor(GameContext* ctx, Sam* sam, ArmorType armor) {
    int cost = 0;
    switch (armor) {
    case ARMOR_LIGHT:
        cost = 80;
        break;
    case ARMOR_HEAVY:
        cost = 150;
        break;
    default:
        return;
    }
    
    if (sam->score >= cost) {
        sam->score -= cost;
        sam->armor = armor;
        Beep(900, 100);
    }
}

void initMap(GameContext* ctx) {
    // 初始化所有格子为空白
    int i, j;
    for (i = 0; i < GRID_ROWS; i++) {
        for (j = 0; j < GRID_COLS; j++) {
            ctx->map[i][j] = EMPTY;
        }
    }
    
    // 根据当前关卡生成不同的墙壁布局
    switch (ctx->currentLevel) {
    case 1:
        // 简单布局，只有少量墙壁
        ctx->map[1][2] = WALL;
        ctx->map[3][6] = WALL;
        ctx->map[2][4] = WALL;
        break;
    case 2:
        // 中等布局，更多墙壁
        ctx->map[0][3] = WALL;
        ctx->map[0][5] = WALL;
        ctx->map[2][1] = WALL;
        ctx->map[2][7] = WALL;
        ctx->map[4][3] = WALL;
        ctx->map[4][5] = WALL;
        break;
    case 3:
        // 复杂布局，大量墙壁
        ctx->map[0][1] = WALL;
        ctx->map[0][7] = WALL;
        ctx->map[1][3] = WALL;
        ctx->map[1][5] = WALL;
        ctx->map[2][2] = WALL;
        ctx->map[2][6] = WALL;
        ctx->map[3][1] = WALL;
        ctx->map[3][7] = WALL;
        ctx->map[4][3] = WALL;
        ctx->map[4][5] = WALL;
        break;
    default:
        // 随机布局
        for (i = 0; i < GRID_ROWS; i++) {
            for (j = 0; j < GRID_COLS; j++) {
                if (rand() % 10 < 3) {
                    ctx->map[i][j] = WALL;
                }
            }
        }
        break;
    }
    
    // 确保玩家初始位置和周围没有墙壁
    ctx->map[GRID_ROWS / 2][GRID_COLS / 2] = EMPTY;
    int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    for (i = 0; i < 4; i++) {
        int newRow = GRID_ROWS / 2 + directions[i][0];
        int newCol = GRID_COLS / 2 + directions[i][1];
        if (newRow >= 0 && newRow < GRID_ROWS && newCol >= 0 && newCol < GRID_COLS) {
            ctx->map[newRow][newCol] = EMPTY;
        }
    }
}

void shopScreen(GameContext* ctx) {
    UserNode* user = findUser(ctx, ctx->currentUser);
    if (user == NULL) {
        ctx->gameState = STATE_MENU;
        return;
    }
    
    // 临时存储当前装备和积分
    static int tempScore = 0;
    static WeaponType tempWeapon = WEAPON_NORMAL;
    static ArmorType tempArmor = ARMOR_NONE;
    static bool initialized = false;
    
    if (!initialized) {
        tempScore = user->totalScore; // 使用用户的总积分
        tempWeapon = user->currentWeapon; // 从用户数据中加载当前武器
        tempArmor = user->currentArmor; // 从用户数据中加载当前护甲
        initialized = true;
    }
    
    cleardevice();
    setbkcolor(RGB(20, 20, 30));
    
    settextstyle(50, 0, _T("黑体"));
    settextcolor(YELLOW);
    outtextxy(450, 50, _T("装备商店"));
    
    settextstyle(25, 0, _T("宋体"));
    settextcolor(WHITE);
    TCHAR scoreText[100];
    swprintf_s(scoreText, 100, _T("当前积分: %d"), tempScore);
    outtextxy(500, 150, scoreText);
    
    // 武器购买区域
    settextstyle(30, 0, _T("黑体"));
    settextcolor(GREEN);
    outtextxy(300, 220, _T("武器"));
    
    Button normalWeaponBtn = { 200, 280, 200, 50, _T("普通武器 (0)"), 1 };
    Button superWeaponBtn = { 200, 350, 200, 50, _T("超级武器 (100)"), 1 };
    Button ultimateWeaponBtn = { 200, 420, 200, 50, _T("终极武器 (200)"), 1 };
    
    // 护甲购买区域
    settextcolor(BLUE);
    outtextxy(700, 220, _T("护甲"));
    
    Button noArmorBtn = { 600, 280, 200, 50, _T("无护甲 (0)"), 1 };
    Button lightArmorBtn = { 600, 350, 200, 50, _T("轻甲 (80)"), 1 };
    Button heavyArmorBtn = { 600, 420, 200, 50, _T("重甲 (150)"), 1 };
    
    Button backBtn = { 450, 520, 300, 60, _T("返回主菜单"), 1 };
    
    drawButton(ctx, &normalWeaponBtn);
    drawButton(ctx, &superWeaponBtn);
    drawButton(ctx, &ultimateWeaponBtn);
    drawButton(ctx, &noArmorBtn);
    drawButton(ctx, &lightArmorBtn);
    drawButton(ctx, &heavyArmorBtn);
    drawButton(ctx, &backBtn);
    
    // 显示当前装备
    settextstyle(20, 0, _T("宋体"));
    settextcolor(LIGHTGRAY);
    outtextxy(200, 500, _T("当前装备:"));
    
    TCHAR weaponText[50];
    switch (tempWeapon) {
    case WEAPON_NORMAL: swprintf_s(weaponText, 50, _T("武器: 普通")); break;
    case WEAPON_SUPER: swprintf_s(weaponText, 50, _T("武器: 超级")); break;
    case WEAPON_ULTIMATE: swprintf_s(weaponText, 50, _T("武器: 终极")); break;
    }
    outtextxy(320, 500, weaponText);
    
    TCHAR armorText[50];
    switch (tempArmor) {
    case ARMOR_NONE: swprintf_s(armorText, 50, _T("护甲: 无")); break;
    case ARMOR_LIGHT: swprintf_s(armorText, 50, _T("护甲: 轻甲")); break;
    case ARMOR_HEAVY: swprintf_s(armorText, 50, _T("护甲: 重甲")); break;
    }
    outtextxy(500, 500, armorText);
    
    // 显示装备效果说明
    settextstyle(18, 0, _T("宋体"));
    settextcolor(LIGHTGRAY);
    outtextxy(200, 550, _T("装备效果:"));
    outtextxy(220, 580, _T("普通武器: 基础伤害"));
    outtextxy(220, 610, _T("超级武器: 子弹速度+25%, 得分+50%"));
    outtextxy(220, 640, _T("终极武器: 子弹速度+50%, 得分+100%"));
    outtextxy(620, 580, _T("无护甲: 基础防护"));
    outtextxy(620, 610, _T("轻甲: 伤害-30%, 扣分-33%"));
    outtextxy(620, 640, _T("重甲: 伤害-50%, 扣分-66%"));
    
    FlushBatchDraw();
    
    while (1) {
        if (MouseHit()) {
            MOUSEMSG msg = GetMouseMsg();
            if (msg.uMsg == WM_LBUTTONDOWN) {
                if (isMouseOnButton(ctx, &normalWeaponBtn, msg.x, msg.y)) {
                    tempWeapon = WEAPON_NORMAL;
                    Beep(900, 100);
                    // 立即保存到用户数据
                    user->totalScore = tempScore;
                    user->currentWeapon = tempWeapon;
                    user->currentArmor = tempArmor;
                    saveUsersToFile(ctx);
                }
                else if (isMouseOnButton(ctx, &superWeaponBtn, msg.x, msg.y)) {
                    if (tempScore >= 100) {
                        tempScore -= 100;
                        tempWeapon = WEAPON_SUPER;
                        Beep(900, 100);
                        // 立即保存到用户数据
                        user->totalScore = tempScore;
                        user->currentWeapon = tempWeapon;
                        user->currentArmor = tempArmor;
                        saveUsersToFile(ctx);
                    }
                }
                else if (isMouseOnButton(ctx, &ultimateWeaponBtn, msg.x, msg.y)) {
                    if (tempScore >= 200) {
                        tempScore -= 200;
                        tempWeapon = WEAPON_ULTIMATE;
                        Beep(900, 100);
                        // 立即保存到用户数据
                        user->totalScore = tempScore;
                        user->currentWeapon = tempWeapon;
                        user->currentArmor = tempArmor;
                        saveUsersToFile(ctx);
                    }
                }
                else if (isMouseOnButton(ctx, &noArmorBtn, msg.x, msg.y)) {
                    tempArmor = ARMOR_NONE;
                    Beep(900, 100);
                    // 立即保存到用户数据
                    user->totalScore = tempScore;
                    user->currentWeapon = tempWeapon;
                    user->currentArmor = tempArmor;
                    saveUsersToFile(ctx);
                }
                else if (isMouseOnButton(ctx, &lightArmorBtn, msg.x, msg.y)) {
                    if (tempScore >= 80) {
                        tempScore -= 80;
                        tempArmor = ARMOR_LIGHT;
                        Beep(900, 100);
                        // 立即保存到用户数据
                        user->totalScore = tempScore;
                        user->currentWeapon = tempWeapon;
                        user->currentArmor = tempArmor;
                        saveUsersToFile(ctx);
                    }
                }
                else if (isMouseOnButton(ctx, &heavyArmorBtn, msg.x, msg.y)) {
                    if (tempScore >= 150) {
                        tempScore -= 150;
                        tempArmor = ARMOR_HEAVY;
                        Beep(900, 100);
                        // 立即保存到用户数据
                        user->totalScore = tempScore;
                        user->currentWeapon = tempWeapon;
                        user->currentArmor = tempArmor;
                        saveUsersToFile(ctx);
                    }
                }
                else if (isMouseOnButton(ctx, &backBtn, msg.x, msg.y)) {
                    initialized = false;
                    ctx->gameState = STATE_MENU;
                    return;
                }
                
                // 重绘界面
                cleardevice();
                setbkcolor(RGB(20, 20, 30));
                
                settextstyle(50, 0, _T("黑体"));
                settextcolor(YELLOW);
                outtextxy(450, 50, _T("装备商店"));
                
                settextstyle(25, 0, _T("宋体"));
                settextcolor(WHITE);
                swprintf_s(scoreText, 100, _T("当前积分: %d"), tempScore);
                outtextxy(500, 150, scoreText);
                
                settextstyle(30, 0, _T("黑体"));
                settextcolor(GREEN);
                outtextxy(300, 220, _T("武器"));
                
                drawButton(ctx, &normalWeaponBtn);
                drawButton(ctx, &superWeaponBtn);
                drawButton(ctx, &ultimateWeaponBtn);
                
                settextcolor(BLUE);
                outtextxy(700, 220, _T("护甲"));
                
                drawButton(ctx, &noArmorBtn);
                drawButton(ctx, &lightArmorBtn);
                drawButton(ctx, &heavyArmorBtn);
                drawButton(ctx, &backBtn);
                
                settextstyle(20, 0, _T("宋体"));
                settextcolor(LIGHTGRAY);
                outtextxy(200, 500, _T("当前装备:"));
                
                switch (tempWeapon) {
                case WEAPON_NORMAL: swprintf_s(weaponText, 50, _T("武器: 普通")); break;
                case WEAPON_SUPER: swprintf_s(weaponText, 50, _T("武器: 超级")); break;
                case WEAPON_ULTIMATE: swprintf_s(weaponText, 50, _T("武器: 终极")); break;
                }
                outtextxy(320, 500, weaponText);
                
                switch (tempArmor) {
                case ARMOR_NONE: swprintf_s(armorText, 50, _T("护甲: 无")); break;
                case ARMOR_LIGHT: swprintf_s(armorText, 50, _T("护甲: 轻甲")); break;
                case ARMOR_HEAVY: swprintf_s(armorText, 50, _T("护甲: 重甲")); break;
                }
                outtextxy(500, 500, armorText);
                
                // 显示装备效果说明
                settextstyle(18, 0, _T("宋体"));
                settextcolor(LIGHTGRAY);
                outtextxy(200, 550, _T("装备效果:"));
                outtextxy(220, 580, _T("普通武器: 基础伤害"));
                outtextxy(220, 610, _T("超级武器: 子弹速度+25%, 得分+50%"));
                outtextxy(220, 640, _T("终极武器: 子弹速度+50%, 得分+100%"));
                outtextxy(620, 580, _T("无护甲: 基础防护"));
                outtextxy(620, 610, _T("轻甲: 伤害-30%, 扣分-33%"));
                outtextxy(620, 640, _T("重甲: 伤害-50%, 扣分-66%"));
                
                FlushBatchDraw();
            }
        }
        Sleep(10);
    }
}

void modeSelectScreen(GameContext* ctx) {
    cleardevice();
    setbkcolor(RGB(20, 20, 30));
    
    settextstyle(50, 0, _T("黑体"));
    settextcolor(YELLOW);
    outtextxy(450, 100, _T("选择游戏模式"));
    
    Button singleModeBtn = {350, 250, 500, 80, _T("单人模式"), 1};
    Button doubleModeBtn = {350, 380, 500, 80, _T("双人模式"), 1};
    Button backBtn = {450, 520, 300, 60, _T("返回主菜单"), 1};
    
    drawButton(ctx, &singleModeBtn);
    drawButton(ctx, &doubleModeBtn);
    drawButton(ctx, &backBtn);
    FlushBatchDraw();
    
    while (1) {
        if (MouseHit()) {
            MOUSEMSG msg = GetMouseMsg();
            if (msg.uMsg == WM_LBUTTONDOWN) {
                if (isMouseOnButton(ctx, &singleModeBtn, msg.x, msg.y)) {
                    ctx->gameMode = MODE_SINGLE;
                    ctx->gameState = STATE_LEVEL_SELECT;
                    return;
                }
                else if (isMouseOnButton(ctx, &doubleModeBtn, msg.x, msg.y)) {
                    ctx->gameMode = MODE_DOUBLE;
                    ctx->gameState = STATE_LEVEL_SELECT;
                    return;
                }
                else if (isMouseOnButton(ctx, &backBtn, msg.x, msg.y)) {
                    ctx->gameState = STATE_MENU;
                    return;
                }
            }
        }
        Sleep(10);
    }
}

void levelSelectScreen(GameContext* ctx) {
    cleardevice();
    setbkcolor(RGB(20, 20, 30));
    
    settextstyle(50, 0, _T("黑体"));
    settextcolor(YELLOW);
    outtextxy(450, 50, _T("选择关卡"));
    
    UserNode* user = findUser(ctx, ctx->currentUser);
    int unlockedLevels = 1; // 默认至少解锁第一关
    if (user != NULL) {
        if (user->unlockedLevels > 0) {
            unlockedLevels = user->unlockedLevels;
        }
    }
    
    Button levelButtons[5];
    int winScores[] = {40, 80, 120, 160, 200};
    
    for (int i = 0; i < 5; i++) {
        int level = i + 1;
        levelButtons[i].x = 200 + (i % 2) * 400;
        levelButtons[i].y = 200 + (i / 2) * 150;
        levelButtons[i].width = 300;
        levelButtons[i].height = 100;
        levelButtons[i].enabled = (level <= unlockedLevels);
        
        TCHAR text[50];
        if (level <= unlockedLevels) {
            int highScore = (user != NULL) ? user->levelScores[i] : 0;
            swprintf_s(text, 50, _T("关卡 %d (目标: %d分)"), level, winScores[i]);
        } else {
            swprintf_s(text, 50, _T("关卡 %d (未解锁)"), level);
        }
        _tcscpy(levelButtons[i].text, text);
    }
    
    Button backBtn = {450, 550, 300, 60, _T("返回模式选择"), 1};
    
    // 绘制关卡按钮
    for (int i = 0; i < 5; i++) {
        drawButton(ctx, &levelButtons[i]);
        
        // 显示关卡最高分
        if (i < unlockedLevels && user != NULL) {
            TCHAR scoreText[50];
            swprintf_s(scoreText, 50, _T("最高分: %d"), user->levelScores[i]);
            settextstyle(20, 0, _T("宋体"));
            settextcolor(LIGHTGRAY);
            outtextxy(levelButtons[i].x + 10, levelButtons[i].y + 70, scoreText);
        }
    }
    
    drawButton(ctx, &backBtn);
    FlushBatchDraw();
    
    while (1) {
        if (MouseHit()) {
            MOUSEMSG msg = GetMouseMsg();
            if (msg.uMsg == WM_LBUTTONDOWN) {
                // 检查关卡按钮点击
                for (int i = 0; i < 5; i++) {
                    if (isMouseOnButton(ctx, &levelButtons[i], msg.x, msg.y) && levelButtons[i].enabled) {
                        ctx->currentLevel = i + 1;
                        ctx->levelScoreThreshold = 100;
                        ctx->gameState = STATE_PLAYING;
                        ctx->gameTimer = GAME_TIME;
                        return;
                    }
                }
                
                // 检查返回按钮点击
                if (isMouseOnButton(ctx, &backBtn, msg.x, msg.y)) {
                    ctx->gameState = STATE_MODE_SELECT;
                    return;
                }
            }
        }
        Sleep(10);
    }
}

void rankingScreen(GameContext* ctx) {
    cleardevice();
    showRanking(ctx);
    FlushBatchDraw();
    _getch();
    ctx->gameState = STATE_MENU;
}

int main() {
    initgraph(WINDOW_WIDTH, WINDOW_HEIGHT);
    setbkcolor(RGB(20, 20, 30));
    cleardevice();
    BeginBatchDraw();

    srand((unsigned int)time(NULL));

    // 创建GameContext对象
    GameContext gameContext;
    // 初始化GameContext
    gameContext.userListHead = NULL;
    gameContext.currentUser[0] = '\0';
    gameContext.gameState = STATE_LOGIN;
    gameContext.gameMode = MODE_SINGLE;
    gameContext.gameTimer = GAME_TIME;
    gameContext.frameCount = 0;
    gameContext.currentLevel = 1;
    gameContext.levelScoreThreshold = 100;
    gameContext.levelCompleted = false;
    gameContext.loginUsername[0] = '\0';
    gameContext.loginPassword[0] = '\0';
    gameContext.registerUsername[0] = '\0';
    gameContext.registerPassword[0] = '\0';
    gameContext.loginInputMode = 0;
    gameContext.registerInputMode = 0;
    gameContext.loginUsernameLen = 0;
    gameContext.loginPasswordLen = 0;
    gameContext.registerUsernameLen = 0;
    gameContext.registerPasswordLen = 0;
    gameContext.loginError = false;
    gameContext.registerError = false;
    gameContext.errorMessage[0] = '\0';
    gameContext.initialized = false;
    gameContext.lastSecond = (int)time(NULL);
    // 初始化敌人列表，确保所有敌人的active字段为false
    for (int i = 0; i < 20; i++) {
        gameContext.enemies[i].active = false;
    }

    loadUsersFromFile(&gameContext);

    bool running = true;
    while (running) {
        switch (gameContext.gameState) {
        case STATE_LOGIN:
            loginScreen(&gameContext);
            break;
        case STATE_REGISTER:
            registerScreen(&gameContext);
            break;
        case STATE_MENU:
            menuScreen(&gameContext);
            break;
        case STATE_MODE_SELECT:
            modeSelectScreen(&gameContext);
            break;
        case STATE_LEVEL_SELECT:
            levelSelectScreen(&gameContext);
            break;
        case STATE_PLAYING:
            gameLoop(&gameContext);
            break;
        case STATE_PAUSED:
            pauseScreen(&gameContext);
            break;
        case STATE_LOSE:
            loseScreen(&gameContext);
            break;
        case STATE_WIN:
            winScreen(&gameContext, 0);
            break;
        case STATE_RANKING:
            rankingScreen(&gameContext);
            break;
        case STATE_SHOP:
            shopScreen(&gameContext);
            break;
        case STATE_EXIT:
            running = false;
            break;
        }
    }

    saveUsersToFile(&gameContext); // 确保退出前保存所有数据
    freeUserList(&gameContext);
    EndBatchDraw();
    closegraph();
    return 0;
}