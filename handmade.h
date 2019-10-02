// Write to the null pointer (???) and crash
#if HANDMADE_SLOW
#define assert(expression) if (!(expression)) { *(int *)0 = 0; }
#else
#define assert(expression)
#endif


#define kilobytes(val) ((val)*1024)
#define megabytes(val) (kilobytes(val)*1024)
#define gigabytes(val) (megabytes(val)*1024)

struct game_offscreen_buffer
{
    void *Memory;
    int Width;
    int Height;
    int Pitch;
    int BytePerPixel;

};

struct game_sound_output_buffer
{
    int samplesPerSecond;
    int sampleCount;
    int16_t *samples;
};

struct game_state
{
    int hz;
    int greenOffset;
    int blueOffset;
};

struct game_memory
{
    bool isInitialized;

    uint64_t permanentStorageSize;
    void *permanentStorage;

    uint64_t transientStorageSize;
    void *transientStorage;
};

// void gameUpdateAndRender(game_offscreen_buffer *Buffer, int XOffset, int YOffset);