#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

//============================================================
//UNIVASF - Universidade Federal do Vale do São Francisco
//Algoritmos e Programação - FARMVILLE 2.0 (raylib-farm)
//============================================================

//============================================================
//Discente: Arthur Lincoln Pinheiro Herculino Almeida
//============================================================

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

// Configurações do grid de plantação
//Posição medida em cima do farm_map.png, pra cair certinho nos 6 canteiros do desenho
#define PLANTATION_GRID_ORIGIN_X 95
#define PLANTATION_GRID_ORIGIN_Y 31
#define PLANTATION_GRID_COLS 2
#define PLANTATION_GRID_ROWS 3
#define GRID_CELL_W 55 //canteiro não é quadrado, então largura e altura separadas
#define GRID_CELL_H 41

// Estrutura para armazenar informações dos itens
typedef struct {
    char name[30];
    int price;
    int sellPrice;
    int growthTime;
    float growthProgress;
    bool isGrowing;
    Vector2 position;
    int gridX, gridY;
} FarmItem;

// Variáveis globais
int quantidade[4][3] = {0};
int graosPlantados = 0;
int dinheiro = 500;
int level = 1;
int xp = 0;
int xpToNextLevel = 100;


//controle de V/F
bool plantationMenuOpen = false;
bool shopOpen = false;
bool barnPopupOpen = false;

float incomeTimer = 0;
float growthTimer = 0;
float passiveIncomeTimer = 0;

//Preço de compra da loja. Saiu de dentro do DrawShopMenu porque o celeiro também precisa
//(é o máximo que dá pra cobrar na venda)
int prices[4][3] = {
    {50, 40, 30},
    {20, 15, 25},
    {35, 20, 10},
    {45, 30, 25}
};

// O array crops agora usa o tamanho da grid de plantação
FarmItem crops[PLANTATION_GRID_COLS * PLANTATION_GRID_ROWS];
int cropsCount = 0;

// Variáveis estáticas para controle do campo de texto no celeiro
static int currentCategory = -1;
static int currentItem = -1;
static char priceText[10] = {0};
static int priceTextLength = 0;
static bool priceFieldActive = false;

// Prototipos de Função (SCRR)
void DrawVerticalGradientRect(float x, float y, float width, float height, Color topColor, Color bottomColor);
void DrawGrassBackground(void);
void AddXP(int amount);
int stringToInt(const char *str);
void DrawShopMenu(Rectangle menuArea, bool *shopOpen, float *scrollOffset);
void DrawBarnPopup(Rectangle area, bool *barnPopupOpen, float *scrollOffset);
void DrawPlantationMenu(Rectangle area, bool *plantationMenuOpen);
void DrawHUD(void);
void UpdateGame(float delta);
void PlantCrop(int gridX, int gridY);
void HarvestCrop(int index);

//Implementacao
int stringToInt(const char *str) {
    int result = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] >= '0' && str[i] <= '9') {
            result = result * 10 + (str[i] - '0');
        }
    }
    return result;
}
//Dá pra fazer com atoi tbm

void DrawGrassBackground(void) {
    DrawVerticalGradientRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
                             (Color){160, 220, 120, 255}, (Color){50, 120, 50, 255});
}


void DrawVerticalGradientRect(float x, float y, float width, float height,
                              Color topColor, Color bottomColor) {
    int steps = (int)height;
    for (int i = 0; i < steps; i++) {
        float t = (float)i / (steps - 1);
        Color c = {
            (unsigned char)((1 - t) * topColor.r + t * bottomColor.r),
            (unsigned char)((1 - t) * topColor.g + t * bottomColor.g),
            (unsigned char)((1 - t) * topColor.b + t * bottomColor.b),
            (unsigned char)((1 - t) * topColor.a + t * bottomColor.a)
        };
        DrawRectangle(x, y + i, width, 1, c);
    }
}


void DrawShopMenu(Rectangle menuArea, bool *shopOpen, float *scrollOffset) {
    const char *categories[] = {"Carnes", "Frutas", "Subprodutos", "Derivados"};
    const char *items[][3] = {
        {"Carne Bovina", "Carne Suína", "Frango"},
        {"Maçã", "Banana", "Uva"},
        {"Mel", "Ovos", "Grãos"},
        {"Queijo", "Manteiga", "Iogurte"}
    };
    int categoryCount = sizeof(categories) / sizeof(categories[0]); //simplificação de nome 1

    float wheel = GetMouseWheelMove(); //simplificação de nome 2
    
    *scrollOffset += wheel * 20.0f;

    float totalHeight = 0;
    for (int c = 0; c < categoryCount; c++) {
        totalHeight += 25 + 3 * 35 + 10;
    }

    float maxOffset = 0;
    float minOffset = -(totalHeight - (menuArea.height - 70));
    if (minOffset > maxOffset) minOffset = maxOffset;
    if (*scrollOffset > maxOffset) *scrollOffset = maxOffset;
    if (*scrollOffset < minOffset) *scrollOffset = minOffset;

    DrawRectangleRec(menuArea, (Color){245, 222, 179, 255});
    DrawRectangleLinesEx(menuArea, 3, BROWN);
    DrawText("Loja da Fazenda", menuArea.x + 90, menuArea.y + 10, 20, DARKGREEN);

    BeginScissorMode(menuArea.x + 5, menuArea.y + 45, menuArea.width - 10, menuArea.height - 55);
    float yOffset = menuArea.y + 50 + *scrollOffset;

    for (int c = 0; c < categoryCount; c++) {
        DrawText(categories[c], menuArea.x + 20, yOffset, 18, DARKBROWN);
        yOffset += 25;

        for (int i = 0; i < 3; i++) {
            Rectangle itemBtn = {menuArea.x + 20, yOffset, menuArea.width - 40, 30};
            bool canAfford = dinheiro >= prices[c][i];

            DrawRectangleRec(itemBtn, canAfford ? LIGHTGRAY : GRAY);
            DrawRectangleLinesEx(itemBtn, 1, GRAY);
            DrawText(TextFormat("%s - $%d", items[c][i], prices[c][i]), itemBtn.x + 10, itemBtn.y + 6, 16, canAfford ? BLACK : RED);

            if (CheckCollisionPointRec(GetMousePosition(), itemBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && canAfford) {
                quantidade[c][i]++;
                dinheiro -= prices[c][i];
                AddXP(2);
            }
            yOffset += 35;
        }
        yOffset += 10;
    }
    EndScissorMode();

    Rectangle closeBtn = {menuArea.x + menuArea.width - 30, menuArea.y + 10, 20, 20};
    DrawRectangleRec(closeBtn, RED);
    DrawText("X", closeBtn.x + 5, closeBtn.y + 2, 16, WHITE);
    if (CheckCollisionPointRec(GetMousePosition(), closeBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        *shopOpen = false;
    }
}

void DrawBarnPopup(Rectangle area, bool *barnPopupOpen, float *scrollOffset) {
    static int sellPrices[][3] = { // Mover para fora da função para persistir os valores
        {40, 30, 20},
        {15, 10, 20},
        {30, 15, 5},
        {35, 20, 15}
    };

    DrawRectangleRec(area, (Color){220, 220, 220, 240});
    DrawRectangleLinesEx(area, 3, DARKGRAY);
    DrawText("Inventário no Celeiro", area.x + 20, area.y + 10, 20, DARKGREEN);

    Rectangle closeBtn = {area.x + area.width - 30, area.y + 10, 20, 20};
    DrawRectangleRec(closeBtn, RED);
    DrawText("X", closeBtn.x + 5, closeBtn.y + 2, 16, WHITE);
    if (CheckCollisionPointRec(GetMousePosition(), closeBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        *barnPopupOpen = false;
        priceFieldActive = false;
        currentCategory = -1; // Reseta o estado
        currentItem = -1; // Reseta o estado
    }

    const char *categories[] = {"Carnes", "Frutas", "Subprodutos", "Derivados"};
    const char *items[][3] = {
        {"Carne Bovina", "Carne Suína", "Frango"},
        {"Maçã", "Banana", "Uva"},
        {"Mel", "Ovos", "Grãos"},
        {"Queijo", "Manteiga", "Iogurte"}
    };
    
    float wheel = GetMouseWheelMove();
    *scrollOffset += wheel * 20.0f;

    int contentHeight = 0;
    for (int c = 0; c < 4; c++) {
        contentHeight += 25 + 3 * 40 + 10;
    }

    float maxOffset = 0;
    float minOffset = -(contentHeight - (area.height - 50));
    if (minOffset > maxOffset) minOffset = maxOffset;
    if (*scrollOffset > maxOffset) *scrollOffset = maxOffset;
    if (*scrollOffset < minOffset) *scrollOffset = minOffset;

    BeginScissorMode(area.x + 10, area.y + 45, area.width - 20, area.height - 55);
    float yOffset = area.y + 50 + *scrollOffset;

    if (priceFieldActive && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        float activeFieldBaseY = area.y + 50 + *scrollOffset;
        for (int c_idx = 0; c_idx < currentCategory; c_idx++) {
            activeFieldBaseY += (25 + 3 * 40 + 10);
        }
        activeFieldBaseY += (25 + currentItem * 40); // Offset para a categoria e itens anteriores
        
        Rectangle activePriceField = {
            area.x + 80,
            activeFieldBaseY + 20, // Posição Y do campo de texto
            60, 18
        };

        if (!CheckCollisionPointRec(GetMousePosition(), activePriceField)) {
            priceFieldActive = false;
            // Processar o valor final ao desativar, se não foi pelo ENTER
            if (priceTextLength > 0) { //se for vazia n converte
                sellPrices[currentCategory][currentItem] = stringToInt(priceText);
                //Não deixa vender mais caro do que comprou, senão era só comprar e vender pra ter dinheiro infinito
                if (sellPrices[currentCategory][currentItem] > prices[currentCategory][currentItem])
                    sellPrices[currentCategory][currentItem] = prices[currentCategory][currentItem];
            } else {
                sellPrices[currentCategory][currentItem] = 0; 
            }
        }
    }


    for (int c = 0; c < 4; c++) {
        DrawText(categories[c], area.x + 20, yOffset, 18, DARKBROWN);
        yOffset += 25;
        
        for (int i = 0; i < 3; i++) {
            DrawText(TextFormat("%s: %d", items[c][i], quantidade[c][i]), area.x + 30, yOffset, 16, BLACK);
            
            DrawText("Preço:", area.x + 30, yOffset + 20, 14, DARKGRAY);
            Rectangle priceField = {area.x + 80, yOffset + 20, 60, 18};
            DrawRectangleRec(priceField, WHITE);
            DrawRectangleLinesEx(priceField, 1, DARKGRAY);
            
            bool thisFieldActive = (currentCategory == c && currentItem == i);
            
            if (CheckCollisionPointRec(GetMousePosition(), priceField) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                priceFieldActive = true;
                currentCategory = c;
                currentItem = i;
                snprintf(priceText, sizeof(priceText), "%d", sellPrices[c][i]);
                priceTextLength = strlen(priceText);
            }
            
            if (priceFieldActive && thisFieldActive) {
                int key = GetCharPressed();
                while (key > 0) {
                    if ((key >= '0' && key <= '9') && priceTextLength < (int)(sizeof(priceText) - 1)) {
                        priceText[priceTextLength] = (char)key;
                        priceTextLength++;
                        priceText[priceTextLength] = '\0';
                    }
                    key = GetCharPressed();
                }
                
                if (IsKeyPressed(KEY_BACKSPACE)) {
                    priceTextLength--;
                    if (priceTextLength < 0) priceTextLength = 0;
                    priceText[priceTextLength] = '\0';
                }
                
                if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_TAB)) {
                    priceFieldActive = false;
                    sellPrices[c][i] = stringToInt(priceText);
                    if (sellPrices[c][i] > prices[c][i]) sellPrices[c][i] = prices[c][i]; //mesmo limite de cima
                }
                
                DrawRectangleRec(priceField, LIGHTGRAY);
            }
            
            //Antes todo campo desenhava o priceText (que é só do campo sendo editado), por isso ficavam todos iguais/vazios
            if (priceFieldActive && thisFieldActive) {
                DrawText(priceText, priceField.x + 5, priceField.y + 2, 14, BLACK);
            } else {
                DrawText(TextFormat("%d", sellPrices[c][i]), priceField.x + 5, priceField.y + 2, 14, BLACK);
            }
            DrawText(TextFormat("máx $%d", prices[c][i]), priceField.x + 66, priceField.y + 3, 12, GRAY);
            
            if (quantidade[c][i] > 0) {
                Rectangle sellBtn = {area.x + area.width - 80, yOffset + 20, 50, 18};
                DrawRectangleRec(sellBtn, ColorAlpha(GREEN, 0.5f));
                DrawText("Vender", sellBtn.x + 5, sellBtn.y + 2, 14, DARKGREEN);
                
                if (CheckCollisionPointRec(GetMousePosition(), sellBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (quantidade[c][i] > 0) {
                        quantidade[c][i]--;
                        dinheiro += sellPrices[c][i];
                        AddXP(3);
                    }
                }
            }
            
            yOffset += 40;
        }
        yOffset += 10;
    }
    EndScissorMode();
}

void DrawPlantationMenu(Rectangle area, bool *plantationMenuOpen) {
    DrawRectangleRec(area, (Color){200, 250, 200, 255});
    DrawRectangleLinesEx(area, 3, DARKGREEN);
    DrawText("Plantação de Grãos", area.x + 20, area.y + 10, 18, DARKGREEN);

    Rectangle closeBtn = {area.x + area.width - 30, area.y + 10, 20, 20};
    DrawRectangleRec(closeBtn, RED);
    DrawText("X", closeBtn.x + 5, closeBtn.y + 2, 16, WHITE);
    if (CheckCollisionPointRec(GetMousePosition(), closeBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        *plantationMenuOpen = false;
    }

    Rectangle plantBtn = {area.x + 20, area.y + 50, area.width - 40, 30};
    bool canPlant = quantidade[2][2] > 0 && graosPlantados < (PLANTATION_GRID_COLS * PLANTATION_GRID_ROWS);
    
    DrawRectangleRec(plantBtn, canPlant ? LIGHTGRAY : GRAY);
    DrawRectangleLinesEx(plantBtn, 1, DARKGREEN);
    DrawText(TextFormat("Plantar Grão (+1) - Usa 1 grão"), plantBtn.x + 10, plantBtn.y + 5, 14, canPlant ? BLACK : RED);

    if (CheckCollisionPointRec(GetMousePosition(), plantBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && canPlant) {
        for (int y = 0; y < PLANTATION_GRID_ROWS; y++) {
            for (int x = 0; x < PLANTATION_GRID_COLS; x++) {
                bool cellOccupied = false;
                for (int i = 0; i < cropsCount; i++) {
                    if (crops[i].gridX == x && crops[i].gridY == y && crops[i].isGrowing) {
                        cellOccupied = true;
                        break;
                    }
                }
                if (!cellOccupied) {
                    PlantCrop(x, y);
                    return;
                }
            }
        }
    }

    char info[50];
    sprintf(info, "Grãos no inventário: %d", quantidade[2][2]);
    DrawText(info, area.x + 20, area.y + 95, 16, DARKBROWN);
    sprintf(info, "Grãos plantados: %d/%d", graosPlantados, (PLANTATION_GRID_COLS * PLANTATION_GRID_ROWS));
    DrawText(info, area.x + 20, area.y + 115, 16, DARKBROWN);
}

void DrawHUD(void) {
    // Dinheiro
    DrawRectangle(10, 10, 200, 30, ColorAlpha(BLACK, 0.5f));
    DrawText(TextFormat("Dinheiro: $%d", dinheiro), 20, 20, 20, YELLOW);
    
    // Nível e XP
    DrawRectangle(220, 10, 200, 30, ColorAlpha(BLACK, 0.5f));
    DrawText(TextFormat("Nivel:%d", level), 230, 20, 20, SKYBLUE);
    DrawText(TextFormat("XP:%d/%d", xp, xpToNextLevel), 330, 20, 20, GREEN);
    
    // Grãos no inventário (mantido para fácil acesso na HUD)
    DrawRectangle(430, 10, 200, 30, ColorAlpha(BLACK, 0.5f));
    DrawText(TextFormat("Grãos: %d", quantidade[2][2]), 440, 20, 20, BROWN);
}

void UpdateGame(float delta) {
    //5 $ a cada 30 segundos pra evitar softlocks
    passiveIncomeTimer += delta;
    if (passiveIncomeTimer >= 30.0f) {
        dinheiro += 5;
        if (dinheiro > 999999) dinheiro = 999999;
        passiveIncomeTimer = 0;
    }

    // Atualizar crescimento das plantações
    for (int i = 0; i < cropsCount; i++) {
        if (crops[i].isGrowing && crops[i].growthProgress < 1.0f) {
            crops[i].growthProgress += delta / crops[i].growthTime;
            if (crops[i].growthProgress > 1.0f) crops[i].growthProgress = 1.0f;
        }
    }
}

void PlantCrop(int gridX, int gridY) {
    if (cropsCount < (PLANTATION_GRID_COLS * PLANTATION_GRID_ROWS) && quantidade[2][2] > 0) {
        crops[cropsCount].isGrowing = true;
        crops[cropsCount].growthProgress = 0.0f;
        crops[cropsCount].growthTime = 10.0f;
        crops[cropsCount].gridX = gridX;
        crops[cropsCount].gridY = gridY;
        crops[cropsCount].position.x = PLANTATION_GRID_ORIGIN_X + gridX * GRID_CELL_W + 4;
        crops[cropsCount].position.y = PLANTATION_GRID_ORIGIN_Y + gridY * GRID_CELL_H + 4;
        strcpy(crops[cropsCount].name, "Grãos");
        crops[cropsCount].sellPrice = 25; // Preço de venda padrão para grãos colhidos (o grão custa 10, antes pagava 10 e não tinha lucro)
        
        quantidade[2][2]--;
        graosPlantados++;
        cropsCount++;
        
        AddXP(5);
    }
}

void HarvestCrop(int index) {
    if (index < 0 || index >= cropsCount) return;
    
    // Adiciona o dinheiro e XP ao colher
    dinheiro += crops[index].sellPrice;
    AddXP(10); // Ganha mais XP por colher
    
    graosPlantados--;
    
    // Remove o item do array movendo o último item para a posição do item removido
    if (index != cropsCount - 1) {
        crops[index] = crops[cropsCount - 1];
    }
    cropsCount--;
}

void AddXP(int amount) {
    xp += amount;
    if (xp >= xpToNextLevel) {
        xp -= xpToNextLevel;
        level++;
        xpToNextLevel = (int)(xpToNextLevel * 1.5f);
        if (xpToNextLevel == 0) xpToNextLevel = 100;
    }
}

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "FARMVILLE 2.0");
    SetTargetFPS(60);

    for (int i = 0; i < (PLANTATION_GRID_COLS * PLANTATION_GRID_ROWS); i++) {
        crops[i].isGrowing = false;
    }

    Image mapImage = LoadImage("resources/farm_map.png");
    if (mapImage.data == NULL) {
        TraceLog(LOG_WARNING, "farm_map.png not found. Generating a green background.");
        mapImage = GenImageColor(SCREEN_WIDTH, SCREEN_HEIGHT, (Color){100, 180, 80, 255});
    }

    ImageResize(&mapImage, SCREEN_WIDTH, SCREEN_HEIGHT);
    Texture2D mapTexture = LoadTextureFromImage(mapImage);
    UnloadImage(mapImage);

    Rectangle marketArea = {SCREEN_WIDTH/2 - 70, SCREEN_HEIGHT - 130, 90, 90};
    Rectangle barnArea = {marketArea.x + 150, marketArea.y - 400, 90, 90};
    Rectangle plantationArea = {PLANTATION_GRID_ORIGIN_X, PLANTATION_GRID_ORIGIN_Y,
                                PLANTATION_GRID_COLS * GRID_CELL_W, PLANTATION_GRID_ROWS * GRID_CELL_H}; //o clique é em cima dos canteiros

    float shopScrollOffset = 0;
    float barnScrollOffset = 0;

    while (!WindowShouldClose()) {
        float delta = GetFrameTime(); //temporizador com base em variavel. Dá pra usar em outras partes do codigo a partir do FPS
        UpdateGame(delta);

        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawGrassBackground();
        DrawTexture(mapTexture, 0, 0, WHITE);
        DrawHUD();

        // Desenhar a grid 2x2 para plantações
        for (int x = 0; x < PLANTATION_GRID_COLS; x++) {
            for (int y = 0; y < PLANTATION_GRID_ROWS; y++) {
                DrawRectangleLines(PLANTATION_GRID_ORIGIN_X + x * GRID_CELL_W, PLANTATION_GRID_ORIGIN_Y + y * GRID_CELL_H, GRID_CELL_W, GRID_CELL_H, ColorAlpha(BLACK, 0.3f));
            }
        }

        // Desenhar plantações
        for (int i = 0; i < cropsCount; i++) {
            if (crops[i].isGrowing) {
                Color cropColor = GREEN;
                if (crops[i].growthProgress < 0.33f) cropColor = ColorAlpha(GREEN, 0.3f);
                else if (crops[i].growthProgress < 0.66f) cropColor = ColorAlpha(GREEN, 0.6f);
                else cropColor = ColorAlpha(DARKGREEN, 0.9f);
                
                DrawRectangleV(crops[i].position, (Vector2){GRID_CELL_W-8, GRID_CELL_H-8}, cropColor);
                //Borda amarela quando tá pronta pra colher, senão o jogador não sabe que pode clicar
                bool pronta = crops[i].growthProgress >= 1.0f;
                DrawRectangleLinesEx((Rectangle){crops[i].position.x, crops[i].position.y, GRID_CELL_W-8, GRID_CELL_H-8}, pronta ? 2 : 1, pronta ? YELLOW : BLACK);
                
                // Barra de progresso
                //Fica dentro do canteiro (embaixo), antes ficava em cima e a linha de cima batia no HUD
                float progressWidth = (GRID_CELL_W-8) * crops[i].growthProgress;
                DrawRectangle(crops[i].position.x, crops[i].position.y + GRID_CELL_H - 14, GRID_CELL_W-8, 4, LIGHTGRAY);
                DrawRectangle(crops[i].position.x, crops[i].position.y + GRID_CELL_H - 14, progressWidth, 4, BLUE);
            }
        }

        // Controlar a abertura dos menus
        bool anyMenuOpen = shopOpen || barnPopupOpen || plantationMenuOpen;

        if (!anyMenuOpen) {
            //Colheita vem PRIMEIRO: o grid fica em cima da plantação, então se viesse depois
            //o clique sempre abria o menu e nunca colhia
            bool colheu = false;
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                Vector2 mousePos = GetMousePosition();
                
                // Verifica se o clique foi na área da grid de plantação
                if (mousePos.x >= PLANTATION_GRID_ORIGIN_X && mousePos.x < PLANTATION_GRID_ORIGIN_X + PLANTATION_GRID_COLS * GRID_CELL_W &&
                    mousePos.y >= PLANTATION_GRID_ORIGIN_Y && mousePos.y < PLANTATION_GRID_ORIGIN_Y + PLANTATION_GRID_ROWS * GRID_CELL_H) {
                    
                    int gridX = (mousePos.x - PLANTATION_GRID_ORIGIN_X) / GRID_CELL_W;
                    int gridY = (mousePos.y - PLANTATION_GRID_ORIGIN_Y) / GRID_CELL_H;
                    
                    // Tenta colher
                    for (int i = 0; i < cropsCount; i++) {
                        if (crops[i].gridX == gridX && crops[i].gridY == gridY && 
                            crops[i].isGrowing && crops[i].growthProgress >= 1.0f) {
                            HarvestCrop(i);
                            colheu = true;
                            break; // Colhe apenas uma planta por clique
                        }
                    }
                }
            }

            // Lógica para abrir menus
            if (!colheu) {
                if (CheckCollisionPointRec(GetMousePosition(), marketArea) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    shopOpen = true;
                } else if (CheckCollisionPointRec(GetMousePosition(), barnArea) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    barnPopupOpen = true;
                } else if (CheckCollisionPointRec(GetMousePosition(), plantationArea) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    plantationMenuOpen = true;
                }
            }
        }
        // Se algum menu está aberto, tratamos os cliques DENTRO dele
        else {
            if (shopOpen) {
                Rectangle menuArea = {150, 80, 500, 440};
                DrawShopMenu(menuArea, &shopOpen, &shopScrollOffset);
            }

            if (barnPopupOpen) {
                Rectangle barnPopupArea = {barnArea.x, barnArea.y - 10, 300, 350};
                DrawBarnPopup(barnPopupArea, &barnPopupOpen, &barnScrollOffset);
            }

            if (plantationMenuOpen) {
                Rectangle plantationMenuArea = {plantationArea.x, plantationArea.y - 10, 300, 200};
                DrawPlantationMenu(plantationMenuArea, &plantationMenuOpen);
            }
        }

        EndDrawing();
    }

    UnloadTexture(mapTexture);
    CloseWindow();
    return 0;
}