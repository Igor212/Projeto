////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Player frames ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int static PLAYER_GRID[2] = {122, 122};
int static PLAYER_LEGS_IDLE_ROW = 0;
int static PLAYER_LEGS_IDLE_NUM_FRAMES = 6;
int static PLAYER_LEGS_JUMPING_ROW = 1;
int static PLAYER_LEGS_JUMPING_NUM_FRAMES = 6;
int static PLAYER_LEGS_WALKING_ROW = 2;
int static PLAYER_LEGS_WALKING_NUM_FRAMES = 6;
int static PLAYER_BODY_IDLE_ROW = 3;
int static PLAYER_BODY_IDLE_NUM_FRAMES = 6;
int static PLAYER_BODY_JUMPING_ROW = 4;
int static PLAYER_BODY_JUMPING_NUM_FRAMES = 6;
int static PLAYER_BODY_SHOOTING_45U_ROW = 5;
int static PLAYER_BODY_SHOOTING_45U_NUM_FRAMES = 4;
int static PLAYER_BODY_SHOOTING_45D_ROW = 6;
int static PLAYER_BODY_SHOOTING_45D_NUM_FRAMES = 4;
int static PLAYER_BODY_SHOOTING_ROW = 7;
int static PLAYER_BODY_SHOOTING_NUM_FRAMES = 4;
int static PLAYER_BODY_WALKING_ROW = 8;
int static PLAYER_BODY_WALKING_NUM_FRAMES = 8;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Objects frames //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum OBJECTS_TYPES {METAL_CRATE, AMMO_CRATE, HP_CRATE, CARD_CRATE1, CARD_CRATE2, CARD_CRATE3, TRASH_BIN, EXPLOSIVE_BARREL, METAL_BARREL,
                            GARBAGE_BAG1, GARBAGE_BAG2, TRASH_CONTAINER, ROAD_BLOCK};

int static OBJECTS_GRID[2] = {200, 200};
int static OBJECTS_METAL_CRATE[2] = {0, 0};
int static OBJECTS_AMMO_CRATE[2] = {0, 1};
int static OBJECTS_HP_CRATE[2] = {0, 2};
int static OBJECTS_CARD_CRATE1[2] = {0, 3};
int static OBJECTS_CARD_CRATE2[2] = {0, 4};
int static OBJECTS_CARD_CRATE3[2] = {0, 5};
int static OBJECTS_TRASH_BIN[2] = {1, 0};
int static OBJECTS_EXPLOSIVE_BARREL[2] = {1, 1};
int static OBJECTS_METAL_BARREL[2] = {1, 2};
int static OBJECTS_GARBAGE_BAG1[2] = {1, 3};
int static OBJECTS_GARBAGE_BAG1[2] = {1, 4};
int static OBJECTS_TRASH_CONTAINER_RECT[4] = {2, 0, 3, 2};
int static OBJECTS_ROAD_BLOCK_RECT[4] = {2, 2, 3, 2};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Background frames ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


int static BACKGROUND_GRID[2] = {120, 450};
int static BACKGROUND_SKYSCRAPER_ROW = 0;
int static BACKGROUND_SKYSCRAPER_NUM_TYPES = 4;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Midground frames ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


int static MIDGROUND_GRID[2] = {178, 36};
int static MIDGROUND_SKYSCRAPER_COL = 0;
int static MIDGROUND_SKYSCRAPER_NUM_TYPES = 6;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Foreground frames ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


int static FOREGROUND_GRID[2] = {200, 200};
int static FOREGROUND_NUM_TYPES = 2; // Número de colunas
int static FOREGROUND_EDGE_ROW = 0;
int static FOREGROUND_DOOR_ROW = 1;
int static FOREGROUND_WALL1_ROW = 2;
int static FOREGROUND_WALL2_ROW = 3;
int static FOREGROUND_ROOF_ROW = 4;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// EnvProps ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


