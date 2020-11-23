#pragma once

class INativeWindow
{
  public:
    virtual ~INativeWindow()               = default;
    INativeWindow()                        = default;
    INativeWindow( const INativeWindow & ) = delete;
    INativeWindow &operator=( const INativeWindow & ) = delete;
    INativeWindow( INativeWindow && )                 = delete;
    INativeWindow &operator=( INativeWindow && ) = delete;
};