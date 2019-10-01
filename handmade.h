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
// void gameUpdateAndRender(game_offscreen_buffer *Buffer, int XOffset, int YOffset);