#if !defined(ENGINE_H)

/*
    NOTE(D):

    MADE_INTERNAL:
        0 - Build for public release
        1 - Build for developer only

    MADE_SLOW:
        0 - Not slow code allowed!
        1 - Slow code welcome.
*/

#include "engine_platform.h"

#define internal static
#define local_presist static
#define global_variable static

#define Pi32 3.14159265359f

#if MADE_SLOW
#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#else
#define Assert(Expression)
#endif

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))
// TODO(D): swap, min, max ... macros???

inline uint32
SafeTruncateUInt64(uint64 Value)
{
    Assert(Value <= 0xFFFFFFFF);
    uint32 Result = (uint32)Value;
    return (Result);
}

inline game_controller_input *GetController(game_input *Input, int unsigned ControllerIndex)
{
    Assert(ControllerIndex < ArrayCount(Input->Controllers));

    game_controller_input *Result = &Input->Controllers[ControllerIndex];
    return(Result);
}

//
//
//

struct canonical_position
{
    int32 TileMapX;
    int32 TileMapY;

    int32 TileX;
    int32 TileY;

    // NOTE(D): This is tile-relative X and Y
    real32 TileRelX;
    real32 TileRelY;
};

struct raw_position
{
    int32 TileMapX;
    int32 TileMapY;

    // NOTE(D): Tile-map relative X and Y
    real32 X;
    real32 Y;
};

struct tile_map
{
    uint32 *Tiles;
};

struct world
{
    real32 TileSideInMeters;
    int32 TileSideInPixels;

    int32 CountX;
    int32 CountY;

    real32 UpperLeftX;
    real32 UpperLeftY;

    int32 TileMapCountX;
    int32 TileMapCountY;

    tile_map *TileMaps;
};

struct game_state
{
    int32 PlayerTileMapX;
    int32 PlayerTileMapY;

    real32 PlayerX;
    real32 PlayerY;
};

#define ENGINE_H
#endif