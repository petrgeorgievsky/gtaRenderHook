#pragma once
class IdleHook
{
public:
    static void Patch();

private:
    static void Idle( void *data );
    /**
     * @brief Updates time counters and deltas
     */
    static void TimeUpdate();
    static void InitPerFrame2D();
    /*!
        Prepares camera params
    */
    static void PrepareRwCamera();
    /*!
        Updates game and audio
    */
    static void GameUpdate();
    /*!
        Renders fronted part of game
    */
    static void RenderHUD();
};
