#include <time.h>
#include <math.h>
#include "raylib.h"
#include "frameMapping.c"


// Enums
enum GAME_STATE{MENU, ACTIVE, PAUSE, GAMEOVER, SCORE};
enum CHARACTER_STATE{IDLE, WALKING, JUMPING, FALLING, ATTACKING, THROWING, DYING, DEAD};
enum CHARACTER_AIMING{FORWARD, UP45, DOWN45};
enum ENTITY_TYPES{PLAYER, ENEMY};
enum ENEMY_BEHAVIOR{NONE, ATTACK, MOVE};
enum BACKGROUND_TYPES{BACKGROUND, MIDDLEGROUND, FOREGROUND};
enum BACKGROUND_STYLE{SKYSCRAPER};
enum MIDDLEGROUND_STYLE{COMPLEX, RED_BUILDING};
enum FOREGROUND_STYLE{RESIDENTIAL, URBAN_FOREST};
enum OBJECT_CLUSTER {PILE_OF_GARBAGE_S, PILE_OF_GARBAGE_L, PILE_OF_BARREL_S, PILE_OF_BARREL_L, PILE_OF_CRATE, COLLECTIBLE};
enum BULLET_TYPE{MAGNUM, SNIPER, LASER};
enum ENEMY_CLASSES{SWORDSMAN, ASSASSIN, GUNNER, SNIPERSHOOTER, DRONE, TURRET, BOSS};
enum OBJECTS_TYPES {METAL_CRATE, AMMO_CRATE, HP_CRATE, CARD_CRATE1, CARD_CRATE2, CARD_CRATE3, TRASH_BIN, EXPLOSIVE_BARREL, METAL_BARREL, GARBAGE_BAG1, GARBAGE_BAG2, TRASH_CONTAINER};
enum PARTICLE_TYPES {EXPLOSION, SMOKE, BLOOD_SPILL, MAGNUM_SHOOT};
enum SOUNDS {FX_MAGNUM, FX_SWORD, FX_CHANGE_SELECTION, FX_SELECTED, FX_ENTITY_LANDING, FX_GRENADE_LAUNCH, FX_GRENADE_BOUNCING, FX_GRENADE_EXPLOSION, FX_HURT, FX_DYING};

// Consts
const float GRAVITY = 600; // px / f²
const float bulletLifeTime = 0.65; // s
const float grenadeExplosionTime = 2.5f; // s
const float msgTime = 3; // s
const float corpseTime = 2; // s
const static int numBackgroundRendered = 7;
const static int maxNumBullets = 100;
const static int maxNumParticles = 500;
const static int maxNumGrenade = 50;
const static int maxNumEnemies = 60;
const static int maxNumGrounds = 300;
const static int maxNumEnvProps = 50;
const static int maxNumMSGs = 50;
const static int numEnemyClasses = 6;
const int screenWidth = 1920;
const int screenHeight = 1080;
const char gameName[30] = "Project N30-N";
bool isFullscreen = true;

// Structs
typedef struct circle {
    Vector2 center;
    float radius;
} Circle;

typedef struct animation {
    enum CHARACTER_STATE currentAnimationState;
    Rectangle currentAnimationFrameRect;
    int isFacingRight;
    float animationFrameSpeed;
    int animationFrameWidth;
    int animationFrameHeight;
    int currentAnimationFrame;
    float timeSinceLastFrame;

} Animation;

typedef struct entity {
    enum ENTITY_TYPES type;
    int maxHP;
    int currentHP;
    float timeSinceDeath;
    float characterWidthScale;
    float characterHeightScale;
    Rectangle drawableRect;
    Rectangle collisionBox;
    Circle collisionHead;
    Vector2 position;
    Vector2 velocity;
    Vector2 momentum;
    float maxXSpeed;
    float sprintSpeed;
    float jumpSpeed;
    bool isGrounded;
    Vector2 eyesOffset;
    Animation upperAnimation;
    Animation lowerAnimation;
    int width;
    int height;
    int grenadeAmmo;
    int magnumAmmo;

    //////// Handlers para controlar tiros nas diagonais
    bool upPressed;
    bool downPressed;

    //////
    int GRID[2];
    int LEGS_IDLE_ROW;
    int LEGS_IDLE_NUM_FRAMES;
    int LEGS_WALKING_ROW;
    int LEGS_WALKING_NUM_FRAMES;
    int UPPER_IDLE_ROW;
    int UPPER_IDLE_NUM_FRAMES;
    int UPPER_WALKING_ROW;
    int UPPER_WALKING_NUM_FRAMES;
    int UPPER_ATTACKING_ROW;
    int UPPER_ATTACKING_NUM_FRAMES;
    int BODY_DYING_ROW;
    int BODY_DYING_NUM_FRAMES;

} Entity;

typedef struct player
{
    Entity entity;
    long points;

}  Player;

typedef struct enemy
{
    Entity entity;
    Vector2 target;
    int id;
    enum ENEMY_CLASSES class;
    enum ENEMY_BEHAVIOR behavior;
    int viewDistance;
    int attackRange;
    float timeSinceLastBehaviorChange;
    float behaviorChangeInterval;
    float noDetectionTime;
    float loseTargetInterval;
    Vector2 spawnLocation;
    float maxDistanceToSpawn;
    bool isAlive;
    float attackSpeed;
    float timeSinceLastAttack;
    int pointsWorth;

} Enemy;

typedef struct ground {
    Rectangle rect;
    bool canBeStepped;
    bool followCamera;
    bool blockPlayer;
    bool isInvisible;
    bool isActive;
    bool isFromObject;
    enum OBJECTS_TYPES objType;
} Ground;


typedef struct background {
    enum BACKGROUND_TYPES bgType;
    RenderTexture2D canvas;
    Texture2D atlas;
    Vector2 position;
    int id;
    int originalX;
    float scale;
    int width;
    float height;
    float relativePosition;

} Background;


typedef struct particle {
    int id;
    Vector2 position;
    Vector2 velocity;
    Rectangle drawableRect;
    Rectangle frameRect;
    int width;
    int height;
    enum PARTICLE_TYPES type;
    float animationFrameSpeed;
    int numFrames;
    int frameRow; // Só será usada se for um atlas para as partículas. Em caso de arquivo individual, pode deletar a variável
    int currentAnimationFrame;
    float timeSinceLastFrame;
    float angle;
    float angularVelocity;
    Vector2 scaleRange;
    float scale;
    bool scaleUp;
    int loopAllowed;
    int isFacingRight;
    float timeToDestroy;
    float lifeTime;
    bool isActive;

} Particle;

typedef struct msgsystem {
    int id;
    Vector2 position;
    Color color;
    int colorId;
    int msg;
    float lifeTime;
    bool isActive;
    
} MSGSystem;

typedef struct bullet {
    Vector2 position;
    int id;
    float angle;
    Animation animation;
    enum ENTITY_TYPES srcEntity;
    enum BULLET_TYPE bulletType;
    Vector2 direction;
    int width;
    int height;
    float power;
    float lifeTime;
    bool isActive;
    Rectangle drawableRect;
    Rectangle collisionBox;

} Bullet;

typedef struct grenade {
    Vector2 position;
    Vector2 velocity;
    int id;
    float angle;
    Animation animation;
    enum ENTITY_TYPES srcEntity;
    Vector2 direction;
    int width;
    int height;
    float power;
    float lifeTime;
    bool isActive;
    Rectangle drawableRect;
    Circle collisionCircle;

} Grenade;

typedef struct envProps {
    int id;
    int groundID;
    enum OBJECTS_TYPES type;
    Rectangle collisionRect;
    Rectangle frameRect;
    Rectangle drawableRect;
    bool isDestroyable;
    bool isCollectable;
    bool isActive;
    int pointsWorth;
} EnvProps;

// Headers
Texture2D CreateTexture(enum BACKGROUND_TYPES bgLayer, Image srcAtlas);
void CreateBullet(Entity *entity, Bullet *bulletsPool, enum BULLET_TYPE bulletType, enum ENTITY_TYPES srcEntity);
int CreateGround(Ground *groundPool, Vector2 position, int width, int height, bool canBeStepped, bool followCamera, bool blockPlayer, bool isInvisible, bool isFromObject, enum OBJECTS_TYPES objType);
void CreateEnvProp(EnvProps *envPropsPool, Ground *groundPool, enum OBJECTS_TYPES obType, Vector2 position, int width, int height);
Background CreateBackground(Player *player, Enemy *enemyPool, EnvProps *envPropsPool, Background *backgroundPool, Ground *groundPool, Texture2D srcAtlas, enum BACKGROUND_TYPES bgType, int *numBackground, int id, int difficulty);
Camera2D CreateCamera (Vector2 target, Vector2 offset, float rotation, float zoom);
Player CreatePlayer(int maxHP, Vector2 position, int width, int height);
void CreateEnemy(Enemy *enemyPool, enum ENEMY_CLASSES class, Vector2 position, int width, int height);
void CreateGrenade(Entity *entity, Grenade *grenadePool, enum ENTITY_TYPES srcEntity);
void CreateParticle(Vector2 srcPosition, Vector2 velocity, Particle *particlePool, enum PARTICLE_TYPES type, float animTime, float angularVelocity, Vector2 scaleRange, bool isLoopable, int facingRight);
void CreateMSG(Vector2 srcPosition, MSGSystem *msgPool, int value);

void DestroyEnvProp(Player *player, Enemy *enemyPool,EnvProps *envPropsPool, Ground *groundsPool, Particle *particlePool, Sound *soundPool, MSGSystem *msgSystem, int envPropID, int difficulty);

void UpdateBackground(Player *player, Background *backgroundPool, int i, Texture2D srcAtlas, Enemy *enemyPool, EnvProps *envPropsPool, Ground *groundPool, float delta, int *numBackground, float minX, float *maxX, int difficulty);
void UpdateClampedCameraPlayer(Camera2D *camera, Player *player, float delta, int width, int height, float *minX, float *maxX);
void UpdatePlayer(Player *player, Enemy *enemy, Bullet *bulletPool, Grenade *grenadePool, float delta, Ground *ground, EnvProps *envProps, Particle *particlePool, Sound *soundPool, MSGSystem *msgSystem, float minX, int difficulty);
void UpdateBullets(Bullet *bullet, Enemy *enemyPool, Player *player, MSGSystem *msgSystem, Ground *groundsPool, EnvProps *envPropsPool, Sound *soundPool, Particle *particlePool, float delta, int maxX, int difficulty);
void UpdateEnemy(Enemy *enemy, Player *player, Bullet *bulletPool, float delta, Ground *ground, EnvProps *envProps, Sound *soundPool, Particle *particlePool, MSGSystem *msgSystem, int minX, int difficulty);
void UpdateGrounds(Player *player, Ground *ground, float delta, float minX);
void UpdateEnvProps(Player *player, Enemy *enemyPool, EnvProps *envPropsPool, Ground *groundsPool, Particle *particlePool, Sound *soundPool, MSGSystem *msgSystem, float delta, float minX);
void UpdateGrenades(Grenade *grenade, Enemy *enemy, Player *player, MSGSystem *msgSystem, Ground *ground, EnvProps *envProp, Particle *particlePool, Sound *soundPool, float delta, int difficulty);
void UpdateParticles(Particle *particlePool, float delta, float minX);
void UpdateMSGs(MSGSystem *curMsg, float delta);
void UpdateDifficulty(int *difficulty, float minX, float time);

void DrawEnemy(Enemy *enemy, Texture2D *texture, bool drawDetectionCollision, bool drawLife, bool drawCollisionBox);
void DrawBullet(Bullet *bullet, Texture2D texture, bool drawCollisionBox);
void DrawPlayer(Player *player, Texture2D texture, bool drawCollisionBox);
void DrawGrenade(Grenade *grenade, Texture2D texture, bool drawCollisionCircle);
void DrawParticle(Particle *particle, Texture2D texture);
void DrawMSG(MSGSystem *msg);

RenderTexture2D PaintCanvas(Texture2D atlas, enum BACKGROUND_TYPES bgLayer, Ground *groundPool, int relativeXPos);

void GenerateBackground(RenderTexture2D canvas, Texture atlas, enum BACKGROUND_STYLE bgStyle);
void GenerateMidground(RenderTexture2D canvas, Texture atlas, enum MIDDLEGROUND_STYLE mgStyle);
void GenerateForeground(RenderTexture2D canvas, Ground *groundPool, Texture atlas, enum FOREGROUND_STYLE fgStyle, int relativeXPos);

void TurnAround(Entity *ent) {
    ent->lowerAnimation.isFacingRight *= -1;
}

void SetTarget(Vector2 target, Enemy *enemy) {
    enemy->target = target;
    enemy->entity.velocity.x = 0;
}

void LookAtTarget(Enemy *enemy) {
    if (enemy->entity.position.x >= enemy->target.x) { //
        if (enemy->entity.lowerAnimation.isFacingRight == 1) TurnAround(&(enemy->entity));
    } else {
        if (enemy->entity.lowerAnimation.isFacingRight == -1) TurnAround(&(enemy->entity));
    }
}

void MoveToTarget(Enemy *enemy) {
    LookAtTarget(enemy);
    enemy->entity.upperAnimation.currentAnimationState = MOVE;
    enemy->entity.momentum.x += 2000;
}

void KillEnemy(Player *player, Enemy *enemy, MSGSystem *msgSystem) {
    int value = 1000;
    Vector2 position = enemy->entity.position;
    position.y += 20;
    player->points += enemy->pointsWorth;
    CreateMSG(position, msgSystem, enemy->pointsWorth);
    enemy->entity.currentHP = 0;
}

void AttackTarget(Enemy *enemy, Entity *playerEntity, Bullet *bulletPool, enum ENEMY_CLASSES enemyClass, Sound *soundPool, Particle *particlePool) {
    // Atualizar estado
    LookAtTarget(enemy);
    switch (enemyClass)
    {
    case ASSASSIN:
        PlaySoundMulti(soundPool[FX_SWORD]);
        PlaySoundMulti(soundPool[FX_HURT]);
        CreateParticle(playerEntity->position, (Vector2) {0,0}, particlePool, BLOOD_SPILL, 2.5f, 0, (Vector2){1,1}, false, enemy->entity.lowerAnimation.isFacingRight);
        playerEntity->currentHP-=30;
        break;
    case GUNNER:
        PlaySoundMulti(soundPool[FX_MAGNUM]);
        CreateBullet(&(enemy->entity), bulletPool, MAGNUM, ENEMY);
        break;
    default:
        break;
    }
    enemy->entity.upperAnimation.currentAnimationState = ATTACKING;
    enemy->entity.momentum.x = 0;
    enemy->entity.velocity.x = 0;
}

void SteeringBehavior(Enemy *enemy, Player *player, Entity *playerEntity, Bullet *bulletPool, Sound *soundPool, Particle *particlePool, float delta, enum ENEMY_CLASSES enemyClass) {
    Entity *eEnt = &(enemy->entity); // Pointer direto para a Entity do inimigo
    Entity *pEnt = &(player->entity); // Pointer direto para a Entity do player
    
    float eyesX = eEnt->position.x + eEnt->eyesOffset.x;
    float eyesY = eEnt->position.y + eEnt->eyesOffset.y;

    Rectangle detectionBox;

    if (eEnt->lowerAnimation.isFacingRight == 1) {
        detectionBox = (Rectangle){eyesX, eyesY, enemy->viewDistance, 5};
    } else {
        detectionBox = (Rectangle){eyesX-enemy->viewDistance, eyesY, enemy->viewDistance, 5};
    }

    Rectangle playerBox = (Rectangle) {pEnt->position.x, pEnt->position.y, pEnt->lowerAnimation.animationFrameWidth * pEnt->characterWidthScale, pEnt->lowerAnimation.animationFrameHeight * pEnt->characterHeightScale};
    
    if ((enemy->entity.lowerAnimation.currentAnimationState != DYING)) {
        if (CheckCollisionRecs(detectionBox, playerBox)) { // Se houver detecção, setar target
            enemy->noDetectionTime = 0;
            if (enemy->behavior == NONE) { // Se não tiver target
                enemy->behavior = MOVE;
                SetTarget(pEnt->position, enemy);
            } else { // Se já tiver target
                SetTarget(pEnt->position, enemy); // Atualiza target
                // MOVER OU ATACAR
                float attackX;
                if (eEnt->lowerAnimation.isFacingRight == 1) { // Direita
                    attackX = eEnt->position.x + eEnt->lowerAnimation.animationFrameWidth * eEnt->characterWidthScale + enemy->attackRange;
                    if (attackX < enemy->target.x) { // Ainda não chegou
                        enemy->behavior = MOVE;
                        MoveToTarget(enemy);
                    } else {
                        if (pEnt->lowerAnimation.currentAnimationState != DYING) {
                            if (enemy->timeSinceLastAttack >= 1/enemy->attackSpeed && enemy->entity.upperAnimation.currentAnimationFrame == 0) {
                                enemy->timeSinceLastAttack = 0;
                                enemy->behavior = ATTACK;
                                AttackTarget(enemy, playerEntity, bulletPool, enemyClass, soundPool, particlePool);
                            }
                        }
                    }
                } else {
                    attackX = eEnt->position.x - enemy->attackRange;
                    if (attackX > enemy->target.x + pEnt->lowerAnimation.animationFrameWidth * pEnt->characterWidthScale) { // Ainda não chegou
                        enemy->behavior = MOVE;
                        MoveToTarget(enemy);
                    } else {
                        if (pEnt->lowerAnimation.currentAnimationState != DYING) {
                            if (enemy->timeSinceLastAttack >= 1/enemy->attackSpeed && enemy->entity.upperAnimation.currentAnimationFrame == 0) {
                                enemy->timeSinceLastAttack = 0;
                                enemy->behavior = ATTACK;
                                AttackTarget(enemy, playerEntity, bulletPool, enemyClass, soundPool, particlePool);
                            }
                        }
                    }
                }
            }
        } else { // Se não tem colisão
            if (enemy->behavior == NONE) { // Se não tiver target
                if (enemy->timeSinceLastBehaviorChange >= enemy->behaviorChangeInterval) { // Controle de tempo para alterar comportamento
                    enemy->timeSinceLastBehaviorChange = 0;
                    int random = GetRandomValue(1, 5); // 5 possibilidades. Precisar tunar para que o inimigo não se afaste tanto do spawn próprio
                    if (random <= 1) { // 10%
                        // Mudar direção
                        TurnAround(eEnt);
                        eEnt->momentum.x = 0; // Parar
                        eEnt->velocity.x = 0; // Parar
                    } else { // 80%
                        random = GetRandomValue(1,5);
                        // Alguma outra opção?
                        if (random <= 2) {// 20%
                            eEnt->momentum.x = 0; // Parar
                            eEnt->velocity.x = 0; // Parar
                        } else {
                            eEnt->momentum.x = 200; // Tem que tunar
                        }
                    }
                }
            } else { // Se tem target
                enemy->noDetectionTime += delta;
                SetTarget(pEnt->position, enemy); // Atualiza target
                if (enemy->noDetectionTime >= enemy->loseTargetInterval) { // Perder target
                    enemy->behavior = NONE;
                    SetTarget((Vector2){-1, -1}, enemy);
                } else { // Se ainda não deu tempo de perder target, ir atrás do player
                    enemy->behavior = MOVE;
                    MoveToTarget(enemy);
                }
            }
        }
    }
}

void PlayEntityAnimation(Entity *entity, float delta, Animation *animation, float numOfFrames, bool isLoopable, bool isFixed, int fixedFrame, bool transitToAnotherState, enum CHARACTER_STATE nextState) {
// ENTITY E DELTA ESTÃO SENDO USADOS TEMPORARIAMENTE PARA USO DAS ANIMACOES DYING E HURT. DEVERÃO SAIR EM ALGUM MOMENTO
    if (animation->timeSinceLastFrame >= animation->animationFrameSpeed) {
        animation->timeSinceLastFrame = 0.0f;
        animation->currentAnimationFrame++;
        if (animation->currentAnimationFrame > numOfFrames-1) { 
            if (isLoopable) {
                animation->currentAnimationFrame = 0; 
            } else {
                if (isFixed) {
                    animation->currentAnimationFrame = fixedFrame; 
                } else {
                    if (transitToAnotherState) {
                        animation->currentAnimationFrame = 0;
                        animation->currentAnimationState = nextState;
                    } else {
                        animation->currentAnimationFrame = numOfFrames-1; 
                    }
                }
            }
        } else {
            /////////////// A PARTIR DESTE PONTO, O CÓDIGO É TEMPORÁRIO
            if (animation->currentAnimationState == DYING) {
                entity->position.x -= animation->isFacingRight*1000*delta;
            }
        }
        if (animation->currentAnimationState == DYING)
            if (!entity->isGrounded) entity->position.x -= animation->isFacingRight*1000*delta;

            /////////////// FIM DO CÓDIGO TEMPORÁRIO
    }
}

void PhysicsAndGraphicsHandlers (Entity *entity, float delta, enum CHARACTER_STATE currentLowerState, enum CHARACTER_STATE currentUpperState, enum ENTITY_TYPES entityType, enum ENEMY_CLASSES enemyClass) {
    Animation *upperAnimation = &(entity->upperAnimation);
    Animation *lowerAnimation = &(entity->lowerAnimation);

    // Atualização da física
    entity->velocity.x += entity->lowerAnimation.isFacingRight*entity->momentum.x * delta;
    if (entity->velocity.x > entity->maxXSpeed) {
        entity->velocity.x = entity->maxXSpeed;
    } else if (entity->velocity.x < -entity->maxXSpeed) {
        entity->velocity.x = -entity->maxXSpeed;
    }
    if (entity->velocity.x > 0) {
        entity->lowerAnimation.isFacingRight = 1;
    } else if (entity->velocity.x < 0) {
        entity->lowerAnimation.isFacingRight = -1;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Atualização de estado para controle da animação da entity        /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Estados relacionados a movimentação
    if ((entity->lowerAnimation.currentAnimationState != DYING)){//} && (entity->animation.currentAnimationState != HURT)) {
        entity->position.x += entity->velocity.x * delta;
        if (entity->isGrounded) {
            if (entity->velocity.x != 0) {
                entity->lowerAnimation.currentAnimationState = WALKING;
                if (entity->upperAnimation.currentAnimationState != ATTACKING && entity->upperAnimation.currentAnimationState != THROWING)  
                    entity->upperAnimation.currentAnimationState = WALKING;
            } else {
                entity->lowerAnimation.currentAnimationState = IDLE;
                if (entity->upperAnimation.currentAnimationState != ATTACKING && entity->upperAnimation.currentAnimationState != THROWING)  
                    entity->upperAnimation.currentAnimationState = IDLE;
            }
        } else if (!entity->isGrounded) {
            if (entity->velocity.y < 0) {
                entity->lowerAnimation.currentAnimationState = JUMPING;
                if (entity->upperAnimation.currentAnimationState != ATTACKING && entity->upperAnimation.currentAnimationState != THROWING)  
                    entity->upperAnimation.currentAnimationState = JUMPING;
            } else if(entity->velocity.y > 0) {
                entity->lowerAnimation.currentAnimationState = FALLING;
                if (entity->upperAnimation.currentAnimationState != ATTACKING && entity->upperAnimation.currentAnimationState != THROWING)  
                    entity->upperAnimation.currentAnimationState = FALLING;
            }
        }
    }
    
    // Atualização de estado quando o inimigo morre
    if (entity->currentHP <= 0 && !(entity->lowerAnimation.currentAnimationState == DYING)) {
        entity->lowerAnimation.currentAnimationState = DYING;
        entity->upperAnimation.currentAnimationState = DYING;

    }

    if ((entity->lowerAnimation.currentAnimationState == DYING)) {
        entity->timeSinceDeath+=delta;
        if (entity->timeSinceDeath >= corpseTime) {
            entity->lowerAnimation.currentAnimationState = DEAD;
        }
    }

    int lAnimRow = 0;
    int uAnimRow = 0;


    if (currentLowerState != lowerAnimation->currentAnimationState) {
        lowerAnimation->timeSinceLastFrame = 0.0f;
        lowerAnimation->currentAnimationFrame = 0;
    }
    if (currentUpperState != upperAnimation->currentAnimationState) {
        upperAnimation->timeSinceLastFrame = 0.0f;
        upperAnimation->currentAnimationFrame = 0;
    }
    switch (lowerAnimation->currentAnimationState)
    {
    case IDLE:
        if (entityType == PLAYER) {
            lAnimRow = PLAYER_LEGS_IDLE_ROW;
            PlayEntityAnimation(entity, delta, lowerAnimation, PLAYER_LEGS_IDLE_NUM_FRAMES, true, false, -1, false, IDLE);
        } else {
            lAnimRow = entity->LEGS_IDLE_ROW;
            PlayEntityAnimation(entity, delta, lowerAnimation, entity->LEGS_IDLE_NUM_FRAMES, true, false, -1, false, IDLE);
        }
        break;
    case WALKING:
        if (entityType == PLAYER) {
            lAnimRow = PLAYER_LEGS_WALKING_ROW;
            PlayEntityAnimation(entity, delta, lowerAnimation, PLAYER_LEGS_WALKING_NUM_FRAMES, true, false, -1, false, WALKING);
        } else {
            lAnimRow = entity->LEGS_WALKING_ROW;
            PlayEntityAnimation(entity, delta, lowerAnimation, entity->LEGS_WALKING_NUM_FRAMES, true, false, -1, false, WALKING);
        }
        break;
    case JUMPING:
        if (entityType == PLAYER) {
            lAnimRow = PLAYER_LEGS_JUMPING_ROW;
            PlayEntityAnimation(entity, delta, lowerAnimation, PLAYER_LEGS_JUMPING_NUM_FRAMES, false, false, -1, false, JUMPING);
        } else {
            // Não tem animação para os inimigos... Usando idle
            lAnimRow = entity->LEGS_IDLE_ROW;
            PlayEntityAnimation(entity, delta, lowerAnimation, entity->LEGS_IDLE_NUM_FRAMES, true, false, -1, false, IDLE);
        }
        break;
    case FALLING:
        if (entityType == PLAYER) {
            lAnimRow = PLAYER_LEGS_JUMPING_ROW;
            PlayEntityAnimation(entity, delta, lowerAnimation, PLAYER_LEGS_JUMPING_NUM_FRAMES, false, true, PLAYER_LEGS_JUMPING_NUM_FRAMES, false, FALLING);
        } else {
            // Não tem animação para os inimigos... Usando idle
            lAnimRow = entity->LEGS_IDLE_ROW;
            PlayEntityAnimation(entity, delta, lowerAnimation, entity->LEGS_IDLE_NUM_FRAMES, true, false, -1, false, IDLE);
        }
        break;
    case DYING:
        if (entityType == PLAYER) {
            lAnimRow = PLAYER_BODY_DYING_ROW;
            PlayEntityAnimation(entity, delta, lowerAnimation, PLAYER_BODY_DYING_NUM_FRAMES, false, false, -1, false, DEAD);
        } else {
            lAnimRow = entity->BODY_DYING_ROW;
            PlayEntityAnimation(entity, delta, lowerAnimation, entity->BODY_DYING_NUM_FRAMES, false, false, -1, false, DEAD);
        }
        break;
    default:
        break;
    }
    
    int numOfFrames = 0;
    switch (upperAnimation->currentAnimationState) {
    case IDLE:
        if (entityType == PLAYER) {
            uAnimRow = PLAYER_UPPER_IDLE_ROW;
            PlayEntityAnimation(entity, delta, upperAnimation, PLAYER_UPPER_IDLE_NUM_FRAMES, true, false, -1, false, IDLE);
        } else {
            uAnimRow = entity->UPPER_IDLE_ROW;
            PlayEntityAnimation(entity, delta, upperAnimation, entity->UPPER_IDLE_NUM_FRAMES, true, false, -1, false, IDLE);
        }
        break;
    case WALKING:
        if (entityType == PLAYER) {
            uAnimRow = PLAYER_UPPER_WALKING_ROW;
            PlayEntityAnimation(entity, delta, upperAnimation, PLAYER_UPPER_WALKING_NUM_FRAMES, true, false, -1, false, WALKING);
        } else {
            uAnimRow = entity->UPPER_WALKING_ROW;
            PlayEntityAnimation(entity, delta, upperAnimation, entity->UPPER_WALKING_NUM_FRAMES, true, false, -1, false, WALKING);
        }
        break;
    case JUMPING:
        if (entityType == PLAYER) {
            uAnimRow = PLAYER_UPPER_JUMPING_ROW;
            PlayEntityAnimation(entity, delta, upperAnimation, PLAYER_UPPER_JUMPING_NUM_FRAMES, false, false, -1, false, JUMPING);
        } else {
            // Não tem animção para os inimigos... usando idle
            uAnimRow = entity->UPPER_IDLE_ROW;
            PlayEntityAnimation(entity, delta, upperAnimation, entity->UPPER_IDLE_NUM_FRAMES, true, false, -1, false, IDLE);
        }
        break;
    case FALLING:
        if (entityType == PLAYER) {
            uAnimRow = PLAYER_UPPER_JUMPING_ROW;
            PlayEntityAnimation(entity, delta, upperAnimation, PLAYER_UPPER_JUMPING_NUM_FRAMES, false, true, PLAYER_UPPER_JUMPING_NUM_FRAMES, false, FALLING);
        } else {
            // Não tem animação para os inimigos... usando idle
            uAnimRow = entity->UPPER_IDLE_ROW;
            PlayEntityAnimation(entity, delta, upperAnimation, entity->UPPER_IDLE_NUM_FRAMES, true, false, -1, false, IDLE);
        }
        break;
    case ATTACKING:
        if (entityType == PLAYER) {
            if (entity->upPressed) {
                uAnimRow = PLAYER_UPPER_ATTACKING_45U_ROW;
                numOfFrames = PLAYER_UPPER_ATTACKING_45U_NUM_FRAMES;
            } else if (entity->downPressed) {
                uAnimRow = PLAYER_UPPER_ATTACKING_45D_ROW;
                numOfFrames = PLAYER_UPPER_ATTACKING_45D_NUM_FRAMES;
            } else {
                uAnimRow = PLAYER_UPPER_ATTACKING_ROW;
                numOfFrames = PLAYER_UPPER_ATTACKING_NUM_FRAMES;
            }
            PlayEntityAnimation(entity, delta, upperAnimation, numOfFrames, false, false, -1, true, IDLE);
        } else {
            uAnimRow = entity->UPPER_ATTACKING_ROW;
            PlayEntityAnimation(entity, delta, upperAnimation, entity->UPPER_ATTACKING_NUM_FRAMES, false, false, -1, true, IDLE);
        }
        break;
    case DYING:
        uAnimRow = -1; // A animação é gerida apenas por "lowerAnimation"
        break;
    case THROWING:
        if (entityType == PLAYER) {
            uAnimRow = PLAYER_UPPER_THROWING_ROW;
            PlayEntityAnimation(entity, delta, upperAnimation, PLAYER_UPPER_THROWING_NUM_FRAMES, false, false, -1, true, IDLE);
        } else {
            // Não tem animação para o inimigo...
        }
        break;
    default:
        break;
    }

    entity->lowerAnimation.currentAnimationFrameRect.x = (float)entity->lowerAnimation.currentAnimationFrame * entity->lowerAnimation.animationFrameWidth;
    entity->lowerAnimation.currentAnimationFrameRect.y = lAnimRow * entity->lowerAnimation.animationFrameHeight;
    entity->lowerAnimation.currentAnimationFrameRect.width = entity->lowerAnimation.isFacingRight * entity->lowerAnimation.animationFrameWidth;

    entity->upperAnimation.currentAnimationFrameRect.x = (float)entity->upperAnimation.currentAnimationFrame * entity->upperAnimation.animationFrameWidth;
    entity->upperAnimation.currentAnimationFrameRect.y = uAnimRow * entity->upperAnimation.animationFrameHeight;
    entity->upperAnimation.currentAnimationFrameRect.width = entity->lowerAnimation.isFacingRight * entity->upperAnimation.animationFrameWidth;
}

void EntityCollisionHandler(Player *player, Entity *entity, Enemy *enemyPool, Ground *ground, EnvProps *envProp, Particle *particlePool, Sound *soundPool, MSGSystem *msgSystem, float delta, int difficulty) {
    // Colisão com grounds                                            ///////////////////////////////////////////////////////////////////////
    int hitObstacle = 0;
    bool initIsGrounded = entity->isGrounded; // usado para o som da entidade batendo no chão
    for (int i = 0; i < maxNumGrounds; i++)
    {
        Ground *curGround = ground + i;
        Vector2 *p = &(entity->position);
        if (curGround->isActive) {
            Rectangle *eCol = &(entity->collisionBox);
            Vector2 *eVel = &(entity->velocity);
            Rectangle futureBox = (Rectangle) {eCol->x + eVel->x * delta, eCol->y + eVel->y * delta, eCol->width, eCol->height};
            if (curGround->blockPlayer) {
                if (CheckCollisionRecs(curGround->rect, futureBox)) {
                    bool checkHorizontal = true;
                    if (futureBox.y + futureBox.height < curGround->rect.y + 5 && eVel->y >= 0) {
                        hitObstacle = 1;
                        eVel->y = 0;
                        p->y = curGround->rect.y - entity->height / 2+1;
                        checkHorizontal = false;
                    } else
                    if (futureBox.y > curGround->rect.y + curGround->rect.height - 5 && eVel->y < 0) {
                        checkHorizontal = false;
                        eVel->y = 0;
                    }
                    if (checkHorizontal) {
                        if (futureBox.x > curGround->rect.x) {
                            if (entity->lowerAnimation.isFacingRight < 0) {
                                p->x = curGround->rect.x + curGround->rect.width + 9;
                            } else {
                                p->x = curGround->rect.x + curGround->rect.width + 39;
                            }
                            eVel->x = 0;
                        } else
                        if (futureBox.x < curGround->rect.x) {
                            if (entity->lowerAnimation.isFacingRight < 0) {
                                p->x = curGround->rect.x - 39;
                            } else {
                                p->x = curGround->rect.x - 9;
                            }
                            eVel->x = 0;
                        }
                    }
                }
            } else if (curGround->canBeStepped) {
                if (CheckCollisionRecs(curGround->rect, futureBox)) {
                    if (entity->velocity.y >= 0) {
                        if (p->y <= curGround->rect.y - entity->height / 2+1) {
                            hitObstacle = 1;
                            entity->velocity.y = 0;
                            p->y = curGround->rect.y - entity->height / 2+1;
                        }
                    }
                }
            }
        }
    }
    
    // Verifica se tem props abaixo para controle da gravidade      ///////////////////////////////////////////////////////////////////////
    if (!hitObstacle) 
    {
        entity->position.y += entity->velocity.y*delta;
        entity->velocity.y += GRAVITY*delta;
        entity->isGrounded = false;
    } else {
        entity->isGrounded = true;
    }

    if (abs(player->entity.position.x - entity->position.x) < 1.1f*screenWidth) {
        if (initIsGrounded != entity->isGrounded && (entity->isGrounded)) {
            PlaySoundMulti(soundPool[FX_ENTITY_LANDING]);
        }
    }

    // Colisão com Props coletáveis                                  ///////////////////////////////////////////////////////////////////////
    if (entity->type == PLAYER) {
        for (int i = 0; i < maxNumEnvProps; i++)
        {
            EnvProps *curProp = envProp + i;
            if (curProp->isActive) {
                Rectangle *eCol = &(entity->collisionBox);
                Vector2 *eVel = &(entity->velocity);
                Rectangle futureBox = (Rectangle) {eCol->x + eVel->x * delta, eCol->y + eVel->y * delta, eCol->width, eCol->height};
                if (curProp->isCollectable) {
                    if (CheckCollisionRecs(curProp->collisionRect, futureBox)) {
                        // TODO dar o benefício
                        switch (curProp->type) {
                        case AMMO_CRATE:
                            entity->grenadeAmmo += 10;
                            entity->grenadeAmmo = fmin(entity->grenadeAmmo, 100);
                            entity->magnumAmmo += 200;
                            entity->magnumAmmo = fmin(entity->magnumAmmo, 999);
                            //player->points += curProp->pointsWorth;
                            CreateMSG((Vector2) {curProp->drawableRect.x+curProp->drawableRect.width/2, curProp->drawableRect.y}, msgSystem, curProp->pointsWorth);
                            break;
                        case HP_CRATE:
                            entity->currentHP += 50;
                            entity->currentHP = fmin(entity->currentHP, entity->maxHP);
                            //player->points += curProp->pointsWorth;
                            CreateMSG((Vector2) {curProp->drawableRect.x+curProp->drawableRect.width/2, curProp->drawableRect.y}, msgSystem, curProp->pointsWorth);
                            break;
                        default:
                            break;
                        }
                        DestroyEnvProp(player, enemyPool, envProp, ground, particlePool, soundPool, msgSystem, i, difficulty);
                        curProp->isActive = false;
                    }
                }
            }
        }
    }

}

void HurtEntity(Entity *dstEntity, Sound *soundPool, int damage) {
    PlaySoundMulti(soundPool[FX_HURT]);
    dstEntity->currentHP -= damage;
}

void ExplosionAOE(Player *player, MSGSystem *msgSystem, EnvProps *envPropPool, Enemy *enemyPool, Ground *groundPool, Particle *particlePool, Sound *soundPool, int explosionRadius, float energy, Vector2 centerOfExplosion, enum ENTITY_TYPES srcEntity, int difficulty) {
    int maxCount = 0;
    maxCount = fmax(maxNumGrounds, maxNumGrenade);
    maxCount = fmax(maxCount, maxNumEnvProps);
    maxCount = fmax(maxCount, maxNumEnemies);
    maxCount = fmax(maxCount, maxNumBullets);
    for (int i = 0; i < maxCount; i++) {
        if (i < maxNumEnvProps) {
            EnvProps *curEnvProp = envPropPool + i;
            // Props
            if (curEnvProp->isActive && curEnvProp->isDestroyable) {
                if (CheckCollisionCircleRec(centerOfExplosion, explosionRadius, curEnvProp->collisionRect)) {
                    DestroyEnvProp(player, enemyPool, envPropPool, groundPool, particlePool, soundPool, msgSystem, i, difficulty);
                }
            }
        }

        if (i < maxNumEnemies) {
            Enemy *curEnemy = enemyPool + i;
            // Enemy
            if (curEnemy->entity.lowerAnimation.currentAnimationState != DYING) {
                if (CheckCollisionCircleRec(centerOfExplosion, explosionRadius, curEnemy->entity.collisionBox)) {
                    KillEnemy(player, curEnemy, msgSystem);
                    curEnemy->entity.currentHP = 0;
                }
            }
        }

    }
}

void PopulateChunk(int chunkId, EnvProps *envPropsPool, Ground *groundPool, Enemy *enemyPool, int difficulty) {
    // chunkId -> posição do chunk para correto posicionamento
    int objAdditions = 0;
    int enemyAdditions = 0;
    // Popular com objetos
    int objProb = 60; // 5% de chance de ter um objeto
    int enemyProb = 70; // 15% de chance de ter um inimigo
    if (GetRandomValue(1,100) <= objProb) {
        objAdditions++;
        int obType;
        int clusterType = GetRandomValue(PILE_OF_GARBAGE_S, PILE_OF_CRATE);
        int rnd;
        if (GetRandomValue(1,100) <= 3) { // 3% de chance
            clusterType = COLLECTIBLE;
        }

        int nextObj;
        int numObjRow;
        int xOffset;
        int numRows = 2;
        enum OBJECTS_TYPES obj;
        int w, h;
        int xPos;
        int pileMax;
        int rowHei[3];
        int objLim1, objLim2;
        int hasAbove;
        rowHei[0] = screenHeight - 130;
        rowHei[1] = screenHeight - 115;
        rowHei[2] = screenHeight - 105;
        switch (clusterType)
        {
        case PILE_OF_GARBAGE_S:
            // amontoado pequeno de lixo (saco de lixo e caixa de papelão)
            numRows = 2;
            xOffset = GetRandomValue(100, 700);
            numObjRow = GetRandomValue(1,3);
            for (int j = 0; j < numRows; j++) {
                xPos = chunkId*screenWidth + xOffset + j*w/2;
                for (int i = 0; i < numObjRow; i++) {
                    nextObj = GetRandomValue(1, 100);
                    if (nextObj <= 60) { // 60% saco de lixo
                        obj = GetRandomValue(GARBAGE_BAG1, GARBAGE_BAG2);
                        w = (obj == GARBAGE_BAG1 ? 100 : 80);
                        h = (obj == GARBAGE_BAG1 ? 100 : 80);
                    } else {
                        obj = GetRandomValue(CARD_CRATE1, CARD_CRATE3);
                        w = 130;
                        h = 130;
                    }
                    CreateEnvProp(envPropsPool, groundPool, obj, (Vector2) {xPos, rowHei[j] - h}, w, h);
                    xPos+=w;
                }
            }
            break;
        case PILE_OF_GARBAGE_L:
            numRows = 2;
            objLim1 = 0;
            objLim2 = 0;
            xOffset = GetRandomValue(100, 500);
            numObjRow = GetRandomValue(2,4);
            for (int j = 0; j < numRows; j++) {
                xPos = chunkId*screenWidth + xOffset + j*w/2;
                for (int i = 0; i < numObjRow; i++) {
                    nextObj = GetRandomValue(1, 100);
                    if (j == 0) {
                        if (nextObj <= 35) { // 20% container
                            if (objLim1 < 1) {
                                obj = TRASH_CONTAINER;
                                w = 220;
                                h = 220;
                            } else {
                                if (GetRandomValue(1,2) == 1) {
                                    obj = GetRandomValue(GARBAGE_BAG1, GARBAGE_BAG2);
                                    w = (obj == GARBAGE_BAG1 ? 100 : 80);
                                    h = (obj == GARBAGE_BAG1 ? 100 : 80);
                                } else {
                                    obj = GetRandomValue(CARD_CRATE1, CARD_CRATE3);
                                    w = 130;
                                    h = 130;
                                }
                            }
                        } else if (nextObj <= 60) { // 40% lata de lixo
                            obj = TRASH_BIN;
                            w = 130;
                            h = 130;
                        } else {
                            if (GetRandomValue(1,2) == 1) {
                                obj = GetRandomValue(GARBAGE_BAG1, GARBAGE_BAG2);
                                w = (obj == GARBAGE_BAG1 ? 100 : 80);
                                h = (obj == GARBAGE_BAG1 ? 100 : 80);
                            } else {
                                obj = GetRandomValue(CARD_CRATE1, CARD_CRATE3);
                                w = 130;
                                h = 130;
                            }
                        }
                    } else {
                        if (nextObj <= 40) { // 40% lata de lixo
                            if (objLim2 < 2) {
                                obj = TRASH_BIN;
                                w = 130;
                                h = 130;
                            } else {
                                if (GetRandomValue(1,2) == 1) {
                                    obj = GetRandomValue(GARBAGE_BAG1, GARBAGE_BAG2);
                                    w = (obj == GARBAGE_BAG1 ? 100 : 80);
                                    h = (obj == GARBAGE_BAG1 ? 100 : 80);
                                } else {
                                    obj = GetRandomValue(CARD_CRATE1, CARD_CRATE3);
                                    w = 130;
                                    h = 130;
                                }
                            }
                        } else {
                            if (GetRandomValue(1,2) == 1) {
                                obj = GetRandomValue(GARBAGE_BAG1, GARBAGE_BAG2);
                                w = (obj == GARBAGE_BAG1 ? 100 : 80);
                                h = (obj == GARBAGE_BAG1 ? 100 : 80);
                            } else {
                                obj = GetRandomValue(CARD_CRATE1, CARD_CRATE3);
                                w = 130;
                                h = 130;
                            }
                        }
                    }
                    CreateEnvProp(envPropsPool, groundPool, obj, (Vector2) {xPos, rowHei[j] - h}, w, h);
                    if (obj == TRASH_CONTAINER) objLim1 = 1;
                    if (obj == TRASH_BIN) objLim2++;
                    xPos+=w;
                }
            }
            break;
        case PILE_OF_BARREL_S:
            numRows = 2;
            objLim1 = 0;
            objLim2 = 0;
            xOffset = GetRandomValue(100, 500);
            numObjRow = GetRandomValue(1,2);
            for (int j = 0; j < numRows; j++) {
                xPos = chunkId*screenWidth + xOffset + j*w/2;
                for (int i = 0; i < numObjRow+j; i++) {
                    nextObj = GetRandomValue(1, 100);
                    if (nextObj <= 10) { // 10% explosivo
                        if (objLim1 < 1) {
                            obj = EXPLOSIVE_BARREL;
                            w = 130;
                            h = 130;
                        } else {
                            if (GetRandomValue(1,2) == 1) {
                                obj = GetRandomValue(GARBAGE_BAG1, GARBAGE_BAG2);
                                w = (obj == GARBAGE_BAG1 ? 100 : 80);
                                h = (obj == GARBAGE_BAG1 ? 100 : 80);
                            } else {
                                obj = GetRandomValue(CARD_CRATE1, CARD_CRATE3);
                                w = 130;
                                h = 130;
                            }
                        }
                    } else if (nextObj <= 80) { // 70% metal
                        obj = METAL_BARREL;
                        w = 140;
                        h = 140;
                    } else if (nextObj <= 90) { // deixando 10% sem nada
                        if (GetRandomValue(1,2) == 1) {
                            obj = GetRandomValue(GARBAGE_BAG1, GARBAGE_BAG2);
                            w = (obj == GARBAGE_BAG1 ? 100 : 80);
                            h = (obj == GARBAGE_BAG1 ? 100 : 80);
                        } else {
                            obj = GetRandomValue(CARD_CRATE1, CARD_CRATE3);
                            w = 130;
                            h = 130;
                        }
                    } else {
                        obj = -1;
                    }
                    if (obj != -1) CreateEnvProp(envPropsPool, groundPool, obj, (Vector2) {xPos, rowHei[j] - h}, w, h);
                    if (obj == EXPLOSIVE_BARREL) objLim1 = 1;
                    xPos+=w;
                }
            }
            break;
        case PILE_OF_BARREL_L:
            numRows = 3;
            objLim1 = 0;
            objLim2 = 0;
            xOffset = GetRandomValue(100, 500);
            numObjRow = GetRandomValue(1,2);
            for (int j = 0; j < numRows; j++) {
                xPos = chunkId*screenWidth + xOffset + j*w/2*(GetRandomValue(1,2) == 1 ? -1 : 1);
                for (int i = 0; i < numObjRow; i++) {
                    nextObj = GetRandomValue(1, 100);
                    if (nextObj <= 20) { // 20% explosivo
                        if (objLim1 < 1) {
                            obj = EXPLOSIVE_BARREL;
                            w = 130;
                            h = 130;
                        } else {
                            if (GetRandomValue(1,2) == 1) {
                                obj = GetRandomValue(GARBAGE_BAG1, GARBAGE_BAG2);
                                w = (obj == GARBAGE_BAG1 ? 100 : 80);
                                h = (obj == GARBAGE_BAG1 ? 100 : 80);
                            } else {
                                obj = GetRandomValue(CARD_CRATE1, CARD_CRATE3);
                                w = 130;
                                h = 130;
                            }
                        }
                    } else if (nextObj <= 80) { // 60% metal
                        obj = METAL_BARREL;
                        w = 140;
                        h = 140;
                    } else if (nextObj <= 90) { // deixando 10% sem nada
                        if (GetRandomValue(1,2) == 1) {
                            obj = GetRandomValue(GARBAGE_BAG1, GARBAGE_BAG2);
                            w = (obj == GARBAGE_BAG1 ? 100 : 80);
                            h = (obj == GARBAGE_BAG1 ? 100 : 80);
                        } else {
                            obj = GetRandomValue(CARD_CRATE1, CARD_CRATE3);
                            w = 130;
                            h = 130;
                        }
                    } else {
                        obj = -1;
                    }
                    if (obj != -1) CreateEnvProp(envPropsPool, groundPool, obj, (Vector2) {xPos, rowHei[j] - h}, w, h);
                    if (obj == EXPLOSIVE_BARREL) objLim1 = 1;
                    xPos+=w;
                }
            }
            break;
        case PILE_OF_CRATE:
            numRows = 2;
            objLim1 = 0;
            objLim2 = 0;
            hasAbove = 0;
            pileMax = 1;
            xOffset = GetRandomValue(100, 500);
            numObjRow = GetRandomValue(1,2);
            if (GetRandomValue(1,4) < 2) {
                pileMax++;
            }
            for (int j = 0; j < numRows; j++) {
            xPos = chunkId*screenWidth + xOffset + j*w/2*(GetRandomValue(1,2) == 1 ? -1 : 1);
                for (int i = 0; i < numObjRow; i++) {
                    hasAbove = 0;
                    nextObj = GetRandomValue(1, 100);
                        if (nextObj <= 40) { // 40% de ter a caixa
                            obj = METAL_CRATE;
                            w = 130;
                            h = 130;
                            if (pileMax == 2) {
                                if (GetRandomValue(1,5) < 5) {
                                    hasAbove = 1;
                                }
                            }
                        } else if (nextObj <= 80) { // 40% de ter o barril
                            obj = METAL_BARREL;
                            w = 140;
                            h = 140;
                            if (pileMax == 2) {
                                if (GetRandomValue(1,5) < 5) {
                                    hasAbove = 1;
                                }
                            }
                        } else {
                            obj = -1;
                        }
                    if (obj != -1) {
                        CreateEnvProp(envPropsPool, groundPool, obj, (Vector2) {xPos, rowHei[j] - h}, w, h);
                        if (hasAbove == 1) {
                            if (j == 0) {
                                CreateEnvProp(envPropsPool, groundPool, obj, (Vector2) {xPos, rowHei[j] - 2*h-2}, w, h);
                            }
                        }
                    } 
                }
            }
            break;
        case COLLECTIBLE:
            numRows = 2;
            objLim1 = 0;
            objLim2 = 0;
            hasAbove = 0;
            pileMax = 1;
            xOffset = GetRandomValue(100, 500);
            numObjRow = GetRandomValue(0,2);
            obj = (GetRandomValue(1,2) == 1 ? AMMO_CRATE : HP_CRATE);
            w = 130;
            h = 130;
            xPos = chunkId*screenWidth + xOffset + numObjRow*w/2*(GetRandomValue(1,2) == 1 ? -1 : 1);
            CreateEnvProp(envPropsPool, groundPool, obj, (Vector2) {xPos, rowHei[numObjRow] - h}, w, h);
            break;
        default:
            break;
        }
    }

    if (chunkId != 0) {
        for(int i = 0; i < (difficulty+1); i++){
            if (GetRandomValue(1,100) <= enemyProb) {
                enemyAdditions++;
                int enClass = GetRandomValue(ASSASSIN, GUNNER);
                CreateEnemy(enemyPool, enClass, (Vector2) {chunkId*screenWidth + GetRandomValue(50, 500), screenHeight-GetRandomValue(160,screenHeight)}, 122, 122);
            }
        }
    }
}
