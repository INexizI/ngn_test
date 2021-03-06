#include "engine.h"
#include "engine_tile.cpp"
#include "engine_random.h"

internal void
GameOutputSound(game_state *GameState, game_sound_output_buffer *SoundBuffer, int ToneHz)
{
    int16 ToneVolume = 3000;
    int WavePeriod = SoundBuffer->SamplesPerSecond/ToneHz;
    
    int16 *SampleOut = SoundBuffer->Samples;
    for (int SampleIndex = 0;
         SampleIndex < SoundBuffer->SampleCount;
         ++SampleIndex)
    {
#if 0
        real32 SineValue = sinf(GameState->tSine);
        int16 SampleValue = (int16)(SineValue * ToneVolume);
#else
        int16 SampleValue = 0;
#endif
        *SampleOut++ = SampleValue;
        *SampleOut++ = SampleValue;
#if 0
        GameState->tSine += 2.0f*Pi32*1.0f/(real32)WavePeriod;
        if(GameState->tSine > 2.0f*Pi32)
        {
            GameState->tSine -= 2.0f*Pi32;
        }
#endif
    }
}

internal void
DrawRectangle(game_offscreen_buffer *Buffer,
              real32 RealMinX, real32 RealMinY, real32 RealMaxX, real32 RealMaxY,
              real32 R, real32 G, real32 B)
{
    int32 MinX = RoundReal32ToInt32(RealMinX);
    int32 MinY = RoundReal32ToInt32(RealMinY);
    int32 MaxX = RoundReal32ToInt32(RealMaxX);
    int32 MaxY = RoundReal32ToInt32(RealMaxY);

    if(MinX < 0)
    {
        MinX = 0;
    }

    if(MinY < 0)
    {
        MinY = 0;
    }

    if(MaxX > Buffer->Width)
    {
        MaxX = Buffer->Width;
    }

    if(MaxY > Buffer->Height)
    {
        MaxY = Buffer->Height;
    }

    uint32 Color = ((RoundReal32ToInt32(R * 255.0f) << 16) |
                    (RoundReal32ToInt32(G * 255.0f) << 8) |
                    (RoundReal32ToInt32(B * 255.0f) << 0));

    uint8 *Row = ((uint8 *)Buffer->Memory +
                  MinX*Buffer->BytesPerPixel +
                MinY*Buffer->Pitch);
    for (int Y = MinY;
         Y < MaxY;
         ++Y)
    {
        uint32 *Pixel = (uint32 *)Row;
        for(int X = MinX;
            X < MaxX;
            ++X)
        {
            *Pixel++ = Color;
        }

        Row += Buffer->Pitch;
    }
}

internal void
InitilaizeArena(memory_arena *Arena, memory_index Size, uint8 *Base)
{
    Arena->Size = Size;
    Arena->Base = Base;
    Arena->Used = 0;
}

#pragma pack(push, 1)
struct bitmap_header
{
    uint16 FileType;
    uint32 FileSize;
    uint16 Reserved1;
    uint16 Reserved2;
    uint32 BitmapOffset;
    uint32 Size;
    int32 Width;
    int32 Height;
    uint16 Planes;
    uint16 BitsPerPixel;
};
#pragma pack(pop)

internal uint32 *
DEBUGLoadBMP(thread_context *Thread, debug_platform_read_entire_file *ReadEntireFile, char *FileName)
{
    uint32 *Result = 0;

    debug_read_file_result ReadResult = ReadEntireFile(Thread, FileName);
    if(ReadResult.ContentSize != 0)
    {
        bitmap_header *Header = (bitmap_header *)ReadResult.Contents;
        uint32 *Pixels = (uint32 *)((uint8 *)ReadResult.Contents + Header->BitmapOffset);
        Result = Pixels;
    }

    return(Result);
}

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    Assert((&Input->Controllers[0].Terminator - &Input->Controllers[0].Buttons[0]) ==
            (ArrayCount(Input->Controllers[0].Buttons)));
    Assert(sizeof(game_state) <= Memory->PermanentStorageSize);

    real32 PlayerHeight = 1.4f;
    real32 PlayerWidth = 0.75f * PlayerHeight;

    game_state *GameState = (game_state *)Memory->PermanentStorage;
    if(!Memory->IsInitialized)
    {
        GameState->PixelPointer = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/test_bg.bmp");

        GameState->PlayerP.AbsTileX = 1;
        GameState->PlayerP.AbsTileY = 3;
        GameState->PlayerP.OffsetX = 5.0f;
        GameState->PlayerP.OffsetY = 5.0f;
        InitilaizeArena(&GameState->WorldArena, Memory->PermanentStorageSize - sizeof(game_state),
                        (uint8 *)Memory->PermanentStorage + sizeof(game_state));

        GameState->World = PushStruct(&GameState->WorldArena, world);
        world *World = GameState->World;
        World->TileMap = PushStruct(&GameState->WorldArena, tile_map);

        tile_map *TileMap = World->TileMap;

        TileMap->ChunkShift = 4;
        TileMap->ChunkMask = (1 << TileMap->ChunkShift) - 1;
        TileMap->ChunkDim = (1 << TileMap->ChunkShift);

        TileMap->TileChunkCountX = 128;
        TileMap->TileChunkCountY = 128;
        TileMap->TileChunkCountZ = 2;
        TileMap->TileChunks = PushArray(&GameState->WorldArena,
                                TileMap->TileChunkCountX*
                                TileMap->TileChunkCountY*
                                TileMap->TileChunkCountZ,
                                tile_chunk);

        TileMap->TileSideInMeters = 1.4f;

        uint32 RandomNumberIndex = 0;
        uint32 TilesPerWidth = 17;
        uint32 TilesPerHeight = 9;
        uint32 ScreenX = 0;
        uint32 ScreenY = 0;
        uint32 AbsTileZ = 0;

        bool32 DoorLeft = false;
        bool32 DoorRight = false;
        bool32 DoorTop = false;
        bool32 DoorBottom = false;
        bool32 DoorUp = false;
        bool32 DoorDown = false;
        for(uint32 ScreenIndex = 0;
            ScreenIndex < 100;
            ++ScreenIndex)
        {
            Assert(RandomNumberIndex < ArrayCount(RandomNumberTable));

            uint32 RandomChoice;
            if(DoorUp || DoorDown)
            {
                RandomChoice = RandomNumberTable[RandomNumberIndex++] % 2;
            }
            else
            {
                RandomChoice = RandomNumberTable[RandomNumberIndex++] % 3;
            }

            bool32 CreatedZDoor = false;
            if(RandomChoice == 2)
            {
                CreatedZDoor = true;
                if(AbsTileZ == 0)
                {
                    DoorUp = true;
                }
                else
                {
                    DoorDown = true;
                }
            }
            else if(RandomChoice == 1)
            {
                DoorRight = true;
            }
            else
            {
                DoorTop = true;
            }

            for(uint32 TileY = 0;
                TileY < TilesPerHeight;
                ++TileY)
            {
                for(uint32 TileX = 0;
                    TileX < TilesPerWidth;
                    ++TileX)
                {
                    uint32 AbsTileX = ScreenX*TilesPerWidth + TileX;
                    uint32 AbsTileY = ScreenY*TilesPerHeight + TileY;

                    uint32 TileValue = 1;
                    if((TileX == 0) && (!DoorLeft || (TileY != (TilesPerHeight/2))))
                    {
                        TileValue = 2;
                    }

                    if((TileX == (TilesPerWidth - 1)) && (!DoorRight || (TileY != (TilesPerHeight/2))))
                    {
                        TileValue = 2;
                    }

                    if((TileY == 0) && (!DoorBottom || (TileX != (TilesPerWidth/2))))
                    {
                        TileValue = 2;
                    }

                    if((TileY == (TilesPerHeight - 1)) && (!DoorTop || (TileX != (TilesPerWidth/2))))
                    {
                        TileValue = 2;
                    }

                    if((TileX == 10) && (TileY == 6))
                    {
                        if(DoorUp)
                        {
                            TileValue = 3;
                        }

                        if(DoorDown)
                        {
                            TileValue = 4;
                        }
                    }
                    

                    SetTileValue(&GameState->WorldArena, World->TileMap, AbsTileX, AbsTileY, AbsTileZ,
                                    TileValue);
                }
            }

            DoorLeft = DoorRight;
            DoorBottom = DoorTop;


            if(CreatedZDoor)
            {
                DoorDown = !DoorDown;
                DoorUp = !DoorUp;
            }
            else
            {
                DoorUp = false;
                DoorDown = false;
            }

            DoorRight = false;
            DoorTop = false;

            if(RandomChoice == 2)
            {
                if(AbsTileZ == 0)
                {
                    AbsTileZ = 1;
                }
                else
                {
                    AbsTileZ = 0;
                }
            }
            else if(RandomChoice == 1)
            {
                ScreenX += 1;
            }
            else
            {
                ScreenY += 1;
            }
        }

        Memory->IsInitialized = true;
    }

    world *World = GameState->World;
    tile_map *TileMap = World->TileMap;

    int32 TileSideInPixels = 60;
    real32 MetersToPixels = (real32)TileSideInPixels / (real32)TileMap->TileSideInMeters;

    real32 LowerLeftX = -(real32)TileSideInPixels / 2;
    real32 LowerLeftY = (real32)Buffer->Height;


    for(int ControllerIndex = 0;
        ControllerIndex < ArrayCount(Input->Controllers);
        ++ControllerIndex)
    {
        game_controller_input *Controller = GetController(Input, ControllerIndex);
        if(Controller->IsAnalog)
        {
            // NOTE(d): Use analog movement tuning
        }
        else
        {
            // NOTE(d): Use digital movement tuning
            real32 dPlayerX = 0.0f; // pixels/seconds
            real32 dPlayerY = 0.0f; // pixels/seconds

            if(Controller->MoveUp.EndedDown)
            {
                dPlayerY = 1.0f;
            }
            if(Controller->MoveDown.EndedDown)
            {
                dPlayerY = -1.0f;
            }
            if(Controller->MoveLeft.EndedDown)
            {
                dPlayerX = -1.0f;
            }
            if(Controller->MoveRight.EndedDown)
            {
                dPlayerX = 1.0f;
            }
            real32 PlayerSpeed = 2.0f;
            if(Controller->ActionUp.EndedDown)
            {
                PlayerSpeed = 10.0f;
            }
            dPlayerX *= PlayerSpeed;
            dPlayerY *= PlayerSpeed;

            // TODO(D): Diagonal will be faster! Fix once we have vectors
            tile_map_position NewPlayerP = GameState->PlayerP;
            NewPlayerP.OffsetX += Input->dtForFrame*dPlayerX;
            NewPlayerP.OffsetY += Input->dtForFrame*dPlayerY;
            NewPlayerP = RecanonicalizePosition(TileMap, NewPlayerP);

            tile_map_position PlayerLeft = NewPlayerP;
            PlayerLeft.OffsetX -= 0.5f*PlayerWidth;
            PlayerLeft = RecanonicalizePosition(TileMap, PlayerLeft);

            tile_map_position PlayerRight = NewPlayerP;
            PlayerRight.OffsetX += 0.5f*PlayerWidth;
            PlayerRight = RecanonicalizePosition(TileMap, PlayerRight);

            if(IsTileMapPointEmpty(TileMap, NewPlayerP) &&
               IsTileMapPointEmpty(TileMap, PlayerLeft) &&
               IsTileMapPointEmpty(TileMap, PlayerRight))
            {
                if(!AreOnSameTile(&GameState->PlayerP, &NewPlayerP))
                {
                    uint32 NewTileValue = GetTileValue(TileMap, NewPlayerP);

                    if(NewTileValue == 3)
                    {
                        ++NewPlayerP.AbsTileZ;
                    }
                    else if(NewTileValue == 4)
                    {
                        --NewPlayerP.AbsTileZ;
                    }
                }

                GameState->PlayerP = NewPlayerP;
            }
        }
    }

    DrawRectangle(Buffer, 0.0f, 0.0f, (real32)Buffer->Width, (real32)Buffer->Height,
                  1.0f, 0.0f, 0.0f);

    real32 ScreenCenterX = 0.5f*(real32)Buffer->Width;
    real32 ScreenCenterY = 0.5f*(real32)Buffer->Height;

    for(int32 RelRow = -10;
        RelRow < 10;
        ++RelRow)
    {
        for(int32 RelColumn = -20;
            RelColumn < 20;
            ++RelColumn)
        {
            uint32 Column = GameState->PlayerP.AbsTileX + RelColumn;
            uint32 Row = GameState->PlayerP.AbsTileY + RelRow;
            uint32 TileID = GetTileValue(TileMap, Column, Row, GameState->PlayerP.AbsTileZ);

            if(TileID > 0)
            {
                real32 Gray = 0.5f;
                if(TileID == 2)
                {
                    Gray = 1.0f;
                }

                if(TileID > 2)
                {
                    Gray = 0.25f;
                }

                if((Column == GameState->PlayerP.AbsTileX) &&
                (Row == GameState->PlayerP.AbsTileY))
                {
                    Gray = 0.0f;
                }

                real32 CenX = ScreenCenterX - MetersToPixels*GameState->PlayerP.OffsetX + ((real32)RelColumn)*TileSideInPixels;
                real32 CenY = ScreenCenterY + MetersToPixels*GameState->PlayerP.OffsetY - ((real32)RelRow)*TileSideInPixels;
                real32 MinX = CenX - 0.5f*TileSideInPixels;
                real32 MinY = CenY - 0.5f*TileSideInPixels;
                real32 MaxX = CenX + 0.5f*TileSideInPixels;
                real32 MaxY = CenY + 0.5f*TileSideInPixels;
                DrawRectangle(Buffer, MinX, MinY, MaxX, MaxY, Gray, Gray, Gray);
            }
        }
    }

    real32 PlayerR = 1.0f;
    real32 PlayerG = 1.0f;
    real32 PlayerB = 0.0f;
    real32 PlayerLeft = ScreenCenterX - 0.5f*MetersToPixels*PlayerWidth;
    real32 PlayerTop = ScreenCenterY - MetersToPixels*PlayerHeight;
    DrawRectangle(Buffer,
                  PlayerLeft, PlayerTop,
                  PlayerLeft + MetersToPixels*PlayerWidth,
                  PlayerTop + MetersToPixels*PlayerHeight,
                  PlayerR, PlayerG, PlayerB);

#if 0
    uint32 *Source = GameState->PixelPointer;
    uint32 *Dest = (uint32 *)Buffer->Memory;
    for (int32 Y = 0;
         Y < Buffer->Height;
         ++Y)
    {
        for(int32 X = 0;
            X < Buffer->Height;
            ++X)
        {
            *Dest++ = *Source++;
        }
    }
#endif
}

extern "C" GAME_GET_SOUND_SAMPLES(GameGetSoundSamples)
{
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    GameOutputSound(GameState, SoundBuffer, 400);
}

/*
internal void
RenderWeirdGradient(game_offscreen_buffer *Buffer, int BlueOffset, int GreenOffset)
{
    uint8 *Row = (uint8 *)Buffer->Memory;
    for (int Y = 0; Y < Buffer->Height; ++Y)
    {
        uint32 *Pixel = (uint32 *)Row;
        for (int X = 0; X < Buffer->Width; ++X)
        {
            uint8 Blue = (uint8)(X + BlueOffset);
            uint8 Green = (uint8)(Y + GreenOffset);
            *Pixel++ = ((Green << 8) | Blue);
        }

        Row += Buffer->Pitch;
    }
}
*/