#include "handmade.h"

void gameOutputSound(game_sound_output_buffer *soundBuffer)
{
    // it breaks without static (sound is corrupted)
    static float tSine;
    int16_t volume = 3000;
    int hz = 256;
    int wavePeriod = soundBuffer->samplesPerSecond / hz;
    int16_t *sampleOut = soundBuffer->samples;

    for(int sampleIndex = 0; sampleIndex < soundBuffer->sampleCount; ++sampleIndex)
    {
        float sineValue = sinf(tSine);
        int16_t sampleValue = (int16_t)(sineValue * volume);
        *sampleOut++ = sampleValue;
        *sampleOut++ = sampleValue;

        tSine += 2.0f * Pi32 * 1.0f / (float)wavePeriod;
    }
}

void DrawGradient(game_offscreen_buffer *Buffer, int XOffset, int YOffset)
{
    uint8_t *Row = (uint8_t *) Buffer->Memory;

    for(int Y = 0; Y < Buffer->Height; ++Y)
    {
        uint32_t *Pixel = (uint32_t *) Row;
        for(int X = 0; X < Buffer->Width; ++X)
        {
            uint8_t Blue = (X + XOffset);
            uint8_t Green = (Y + YOffset);
            *Pixel++ = ((Green << 8) | Blue);
        }
        Row += Buffer->Pitch;
    }
};

void gameUpdateAndRender(game_memory *memory,
                         game_offscreen_buffer *Buffer,
                         game_sound_output_buffer *soundBuffer
                        )
{
    assert(sizeof(game_state) <= memory->permanentStorageSize);
    game_state *gameState = (game_state *)memory->permanentStorage;
    if(!memory->isInitialized)
    {
        gameState->hz = 256;
        gameState->greenOffset = 0;
        gameState->blueOffset = 0;
        memory->isInitialized = true;
    }

    gameOutputSound(soundBuffer);
    DrawGradient(Buffer, gameState->blueOffset, gameState->greenOffset);
};