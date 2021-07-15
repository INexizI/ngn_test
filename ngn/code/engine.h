#if !defined(ENGINE_H)

// TODO(D): Services that the platform layer provides to the game
// NOTE(D): Services that the game to the platform layer

// FOUR THINGS - timing, controller/keyboards input, bitmap buffer to use, sound buffer to use
struct game_offscreen_buffer
{
    // NOTE(D): Pixels are always 32-bits wide, Memory Order BB GG RR XX
    void *Memory;
    int Width;
    int Height;
    int Pitch;
};
internal void GameUpdateAndRender(game_offscreen_buffer *Buffer, int BlueOffset, int GreenOffset);

#define ENGINE_H
#endif