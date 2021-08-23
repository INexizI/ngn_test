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

struct tile_chunk_position
{
    uint32 TileChunkX;
    uint32 TileChunkY;

    uint32 RelTileX;
    uint32 RelTileY;
};

struct world_position
{
    uint32 AbsTileX;
    uint32 AbsTileY;

    real32 TileRelX;
    real32 TileRelY;
};

struct tile_chunk
{
    uint32 *Tiles;
};

struct world
{
    uint32 ChunkShift;
    uint32 ChunkMask;
    uint32 ChunkDim;

    real32 TileSideInMeters;
    int32 TileSideInPixels;
    real32 MetersToPixels;

    int32 TileChunkCountX;
    int32 TileChunkCountY;

    tile_chunk *TileChunks;
};

struct game_state
{
    world_position PlayerP;
};

#define ENGINE_H
#endif