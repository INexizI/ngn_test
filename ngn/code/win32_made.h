#if !defined(WIN32_MADE_H)

struct win32_offscreen_buffer
{
    // NOTE(D): Pixels are always 32-bits wide, Memory Order BB GG RR XX
    BITMAPINFO Info;
    void *Memory;
    int Width;
    int Height;
    int Pitch;
    int BytesPerPixel;
};

struct win32_window_dimension
{
    int Width;
    int Height;
};

struct win32_sound_output
{
    int SamplesPerSecond;
    uint32 RunningSampleIndex;
    int BytesPerSample;
    DWORD SecondaryBufferSize;
    DWORD SafetyBytes;
    real32 tSine;
    int LatencySamppleCount;
};

struct win32_debug_time_marker
{
    DWORD OutputPlayCursor;
    DWORD OutputWriteCursor;
    DWORD OutputLocation;
    DWORD OutputByteCount;
    DWORD ExpectedFlipPlayCursor;

    DWORD FlipPlayCursor;
    DWORD FlipWriteCursor;
};

struct win32_game_code
{
    HMODULE GameCodeDLL;
    FILETIME DLLLastWriteTime;
    game_update_and_render *UpdateAndRender;
    game_get_sound_samples *GetSoundSamples;

    bool32 IsValid;
};

struct win32_state
{
    uint64 TotalSize;
    void *GameMemoryBlock;

    HANDLE RecordingHandle;
    int InputRecordingIndex;

    HANDLE PlaybackHandle;
    int InputPlayingIndex;
};

#define WIN32_MADE_H
#endif