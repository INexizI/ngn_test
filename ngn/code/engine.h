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

struct game_sound_output_buffer
{
    int SamplesPerSecond;
    int SampleCount;
    int16 *Samples;
};

internal void GameUpdateAndRender(game_offscreen_buffer *Buffer, int BlueOffset, int GreenOffset,
                                  game_sound_output_buffer *SoundBuffer);

#define ENGINE_H
#endif