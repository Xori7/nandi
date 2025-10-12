#define SCP "Strategy communication protocol"
#include <stdint.h>

typedef struct {
    uint32_t x, y;
    uint32_t tileCacheIndex;
} A_Entity;

typedef struct {
    A_Entity* entities[16];

    float height;

} A_MapTile;

typedef struct {
    A_MapTile tiles[256][256];
} A_Map;

#define NULL 0

uint32_t map_tile_get_first_empty_entity_index(A_Map *map, uint32_t x, uint32_t y) {
    return 0;
}

void entity_position_set(A_Map *map, A_Entity *entity, uint32_t x, uint32_t y) {
    map->tiles[entity->x][entity->y].entities[entity->tileCacheIndex] = 0;

    uint32_t freeIndex = map_tile_get_first_empty_entity_index(map, x, y);
    map->tiles[x][y].entities[freeIndex] = entity;
    entity->x = x;
    entity->y = y;
    entity->tileCacheIndex = freeIndex;
}
