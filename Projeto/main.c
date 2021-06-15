#include "raylib.h"
#include <stdlib.h>     
#include <stdio.h>
#include <time.h>
#include <math.h>

#define WP_GUN_ID 1;
#define WP_MACHINEGUN_ID 2;
#define WP_SWORD_ID 3;

// Enums
enum CHARACTER_STATE{IDLE, WALKING, HURT, JUMPING, FALLING, SHOOTING, DYING};

// Structs

typedef struct player
{
    int maxHP;
    int currentHP;
    Vector2 position;
    Vector2 velocity;
    int currentWeaponID;
    int currentAmmo;
    float walkSpeed;
    float sprintSpeed;
    float jumpSpeed;
    bool isGrounded;

    // Animation
    Rectangle currentAnimationFrameRect;
    enum CHARACTER_STATE currentAnimationState;
    int isFacingRight;
    float animationFrameSpeed;
    int animationFrameWidth;
    int animationFrameHeight;
    int currentAnimationFrame;
    float timeSinceLastFrame;
    float characterWidthScale;
    float characterHeightScale;

}  Player;

typedef struct enemy
{
    int maxHP;
    int currentHP;
    Vector2 position;
    float verticalSpeed;

} Enemy;

typedef struct props {
    Rectangle rect;
    int canBeStepped;
    int blockPlayer;
    int isInvisible;

} Props;


typedef struct weapon 
{
    int id;
    int maxAmmo;
    int baseDamage;

} Weapon;

// Headers
void CreatePlayer(Player *player, int maxHP, Vector2 position, float imageWidth, float imageHeight, int imageFramesCount);
void UpdatePlayer(Player *player, float delta, Props *props, int framesCounter);
void UpdateProps(Player *player, Props *props, float delta, int framesCounter);
void UpdateClampedCameraPlayer(Camera2D *camera, Player *player, Props *props, float delta, int width, int height);

// Environment handlers
float GRAVITY = 400;
int numBackgroundRendered = 3;


int main(void)
{
    // Window
    const int screenWidth = 800;
    const int screenHeight = 450;
    //SetConfigFlags(FLAG_FULLSCREEN_MODE); // Fullscreen

    InitWindow(screenWidth, screenHeight, "Project N30-N");
    SetTargetFPS(60);      // Set our game to run at 60 frames-per-second
    int framesCounter = 0; // Animation handler

    // Load assets
    Texture2D background = LoadTexture("textures/cyberpunk_street_background.png");
    Texture2D midground = LoadTexture("textures/cyberpunk_street_midground.png");
    Texture2D foreground = LoadTexture("textures/cyberpunk_street_foreground.png");
    Texture2D character = LoadTexture("textures/hero_atlas.png");        // Texture loading
    //Texture2D character = LoadTexture("textures/walk.png");        // Texture loading

    // Environment Init
    Props envProps[1] = {
        {{0,430,screenWidth,5}, 1, 0, 1} // Floor
        };

    // Player Init
    Player player; // Create instance
    Vector2 position = {0.0f, 330.0f}; // Define position
    CreatePlayer(&player, 100, position,character.width, character.height, 34); // Player init

    // Camera init
    Camera2D camera = { 0 };
    camera.target = player.position;
    camera.offset = (Vector2){ screenWidth/2.0f, screenHeight/2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.00f;

    // Main game loop
    while (!WindowShouldClose())
    {
        // Updates
        float deltaTime = GetFrameTime();
        framesCounter++;
        UpdatePlayer(&player, deltaTime, envProps, framesCounter);
        UpdateProps(&player, envProps, deltaTime, framesCounter);
        UpdateClampedCameraPlayer(&camera, &player, envProps, deltaTime, screenWidth, screenHeight);

        // Draw cycle
        BeginDrawing();

            ClearBackground(GetColor(0x052c46ff));

            BeginMode2D(camera);
           
                // Draw background image
                DrawTextureEx(background, (Vector2){ 0, 20 }, 0.0f, 2.0f, WHITE);
                DrawTextureEx(background, (Vector2){ background.width, 20 }, 0.0f, 2.0f, WHITE);
                // Draw midground image
                DrawTextureEx(midground, (Vector2){ 0, 20 }, 0.0f, 2.0f, WHITE);
                DrawTextureEx(midground, (Vector2){ midground.width, 20 }, 0.0f, 2.0f, WHITE);
                // Draw foreground image
                DrawTextureEx(foreground, (Vector2){ 0, 70 }, 0.0f, 2.0f, WHITE);
                DrawTextureEx(foreground, (Vector2){ foreground.width, 70 }, 0.0f, 2.0f, WHITE);
                // Draw player
                DrawTextureRec(character, player.currentAnimationFrameRect, player.position, WHITE);
                //float charWidth = player.animationFrameWidth*player.characterWidthScale;
                //float charHeight= player.animationFrameHeight*player.characterHeightScale;
                //Rectangle dstRect = {player.position.x, player.position.y, charWidth, charHeight};
                //DrawTexturePro(character, player.currentAnimationFrameRect, dstRect, (Vector2) {0, 0}, 0, WHITE);
                // Draw props
                for (int i = 0; i < 1; i++) { // TODO 1 is "props[]"'s size
                    if (!envProps[i].isInvisible)
                        DrawRectangleRec(envProps[i].rect, WHITE);
                }


            EndMode2D();
            

            // TODO o que está nessa região fica "parado" em relação à câmera
            // Debbug
            char debbugMsg[20];
            DrawText("Hit / Floor:", 0, 0, 20, WHITE);
            sprintf(debbugMsg, "%i", player.isGrounded);
            DrawText(debbugMsg, 0, 25, 20, WHITE);

        EndDrawing();
    }

    // Unload
    UnloadTexture(background);
    UnloadTexture(midground);
    UnloadTexture(foreground);
    UnloadTexture(character);

    CloseWindow();
    return 0;
}

void CreatePlayer (Player *player, int maxHP, Vector2 position, float imageWidth, float imageHeight, int imageFramesCount) {

    player->maxHP = maxHP;
    player->currentHP = maxHP;
    player->position.x = position.x;
    player->position.y = position.y;
    player->velocity.x = 0;
    player->velocity.y = 0;
    player->currentWeaponID = -1;
    player->currentAmmo = 0;
    player->walkSpeed = 400;
    player->sprintSpeed = 800;
    player->jumpSpeed = 250;
    player->isGrounded = true;

    player->animationFrameSpeed = 0.08f;
    player->animationFrameWidth = 122;//(float)imageWidth/imageFramesCount;
    player->animationFrameHeight = 122;//imageHeight;
    player->currentAnimationFrame = 0;
    player->currentAnimationState = IDLE;
    player->isFacingRight = 1;
    player->timeSinceLastFrame = 0;
    player->currentAnimationFrameRect.x = 0.0f;
    player->currentAnimationFrameRect.y = 0.0f;
    player->currentAnimationFrameRect.width = player->animationFrameWidth;
    player->currentAnimationFrameRect.height = player->animationFrameHeight;
    player->characterWidthScale = 1.00f;
    player->characterHeightScale = 1.00f;
}

void UpdatePlayer(Player *player, float delta, Props *props, int framesCounter)
{
    enum CHARACTER_STATE currentState = player->currentAnimationState;
    player->timeSinceLastFrame += delta;

    if (player->currentAnimationState != DYING && player->currentAnimationState != HURT) {
        if (IsKeyDown(KEY_LEFT)) {
            player->position.x -= player->walkSpeed*delta;
            player->isFacingRight = -1;
            player->currentAnimationState = WALKING;
            
        } else if (IsKeyDown(KEY_RIGHT)) {
            player->position.x += player->walkSpeed*delta;
            player->isFacingRight = 1;
            player->currentAnimationState = WALKING;
        } else {
            player->currentAnimationState = IDLE;
        }

        if (IsKeyDown(KEY_SPACE) && player->isGrounded) 
        {
            player->velocity.y = -player->jumpSpeed;
            player->isGrounded = false;
        }

        if (IsKeyPressed(KEY_H))
            player->currentAnimationState = HURT;

        if (IsKeyPressed(KEY_P))
            player->currentHP = 0;
    }

    // Collision check
    int hitObstacle = 0;
    int hasFloorBelow = 0;
    Rectangle prect = {player->position.x, player->position.y, player->animationFrameWidth, player->animationFrameHeight};
    Rectangle prectGrav = {player->position.x, player->position.y+1, player->animationFrameWidth, player->animationFrameHeight};
    for (int i = 0; i < 1; i++)  // TODO 1 is "props[]"'s size
    {
        Props *eprop = props + i;
        Vector2 *p = &(player->position);
        if (eprop->canBeStepped) {
            if (CheckCollisionRecs(eprop->rect, prect)) {
                hitObstacle = 1;
                player->velocity.y = 0.0f;
                p->y = eprop->rect.y - player->animationFrameHeight;
            }
            if (CheckCollisionRecs(eprop->rect, prectGrav)) {
                hasFloorBelow = 1;
            }
        }
    }
    
    if (!hitObstacle) 
    {
        player->position.y += player->velocity.y * delta;
        player->velocity.y += GRAVITY * delta;
        player->isGrounded = false;
    } 
    else player->isGrounded = true;

    if (hasFloorBelow) {
        player->velocity.y = 0;
        player->isGrounded = true;
    }

    // Animation handlers
    if ((player->currentAnimationState != DYING) && (player->currentAnimationState != HURT)) {
        if (!player->isGrounded) {
            if (player->velocity.y < 0) {
                player->currentAnimationState = JUMPING;
            } else if(player->velocity.y > 0) {
                player->currentAnimationState = FALLING;
            }
        }
    }

    // Atualização de estado quando o player morre
    if (player->currentHP <= 0 && !(player->currentAnimationState == DYING)) {
        player->currentAnimationState = DYING;
    }

    int animRow = 0;
    if (currentState != player->currentAnimationState) {
        player->timeSinceLastFrame = 0.0f;
        player->currentAnimationFrame = 0;
    }
    switch (player->currentAnimationState)
    {
    case IDLE:
        animRow = 0;
        if (player->timeSinceLastFrame >= player->animationFrameSpeed) {
            player->timeSinceLastFrame = 0.0f;
            player->currentAnimationFrame++;
            if (player->currentAnimationFrame > 5) player->currentAnimationFrame = 0; // 5 porque são 6 frames para essa animação 
        }
        break;
    case WALKING:
        animRow = 1;
        if (player->timeSinceLastFrame >= player->animationFrameSpeed) {
            player->timeSinceLastFrame = 0.0f;
            player->currentAnimationFrame++;
            if (player->currentAnimationFrame > 7) player->currentAnimationFrame = 0; // 7 porque são 8 frames para essa animação 
        }
        break;
    case HURT:
        animRow = 2;
        if (player->timeSinceLastFrame >= player->animationFrameSpeed) {
            player->timeSinceLastFrame = 0.0f;
            player->currentAnimationFrame++;
            if (player->currentAnimationFrame > 5) { // 5 porque são 6 frames para essa animação, depois muda o estado
                player->currentAnimationFrame = 0; 
                player->currentAnimationState = IDLE;
            } else if (player->currentAnimationFrame < 2) {
                player->position.x -= player->isFacingRight*150*delta;
            }
        }
        break;
    case JUMPING:
        animRow = 3;
        if (player->timeSinceLastFrame >= player->animationFrameSpeed) {
            player->timeSinceLastFrame = 0.0f;
            player->currentAnimationFrame++;
            if (player->currentAnimationFrame > 4) player->currentAnimationFrame = 4; // 4 porque são 5 frames para essa animação, além disso, mantém o frame em "4"
        }
        break;
    case FALLING:
        animRow = 3;
        if (player->timeSinceLastFrame >= player->animationFrameSpeed) {
            player->timeSinceLastFrame = 0.0f;
            player->currentAnimationFrame++;
            player->currentAnimationFrame = 5; // Mantém o frame em "4", pois a queda é apenas 1 frame
        }
        break;
    case DYING:
        animRow = 4;
        if (player->timeSinceLastFrame >= player->animationFrameSpeed) {
            player->timeSinceLastFrame = 0.0f;
            player->currentAnimationFrame++;
            if (player->currentAnimationFrame > 6) { // 6 porque são 7 frames para essa animação, além disso, mantém o frame em "6"
                player->currentAnimationFrame = 6; 
            } else {
                player->position.x -= player->isFacingRight*100*delta;
            }
        }
        break;
    default:
        break;
    }

    // Update player animation frame Rect
    player->currentAnimationFrameRect.x = (float)player->currentAnimationFrame * player->animationFrameWidth;
    player->currentAnimationFrameRect.y = animRow * player->animationFrameHeight;
    player->currentAnimationFrameRect.width = player->isFacingRight * player->animationFrameWidth;

    // Limitar posição do player
    if (player->position.x < 0) {
        player->position.x = 0;
    } 
}

void UpdateProps(Player *player, Props *props, float delta, int framesCounter) {

}

void UpdateClampedCameraPlayer(Camera2D *camera, Player *player, Props *props, float delta, int width, int height)
{
    camera->target = player->position;
    camera->offset = (Vector2){ width/2.0f, height/2.0f };
    float minX = 0.00f, minY = 0.00f, maxX = 1000, maxY = height; // Camera clamp controls. TODO, maxX
    
    Vector2 max = GetWorldToScreen2D((Vector2){ maxX, maxY }, *camera);
    Vector2 min = GetWorldToScreen2D((Vector2){ minX, minY }, *camera);
    
    if (max.x < width) camera->offset.x = width - (max.x - width/2);
    if (max.y < height) camera->offset.y = height - (max.y - height/2);
    if (min.x > 0) camera->offset.x = width/2 - min.x;
    if (min.y > 0) camera->offset.y = height/2 - min.y;
}
