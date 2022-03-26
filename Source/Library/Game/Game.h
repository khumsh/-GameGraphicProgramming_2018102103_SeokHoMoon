/*+===================================================================
  File:      GAME.H
  Summary:   Game header file that contains declarations of functinos
             used for the lab samples of Game Graphics Programming
             course.
  Functions: InitWindow, InitDevice, CleanupDevice, Render
  © 2022 Kyung Hee University
===================================================================+*/
#pragma once

#include "Common.h"

#include "Renderer/Renderer.h"
#include "Window/MainWindow.h"

namespace library
{
    /*--------------------------------------------------------------------
      Forward declarations
    --------------------------------------------------------------------*/

    /*F+F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F
      Function: InitWindow
      Summary:  Registers the window class and creates a window
      Args:     HINSTANCE hInstance
                  Handle to the instance
                INT nCmdShow
                  Is a flag that says whether the main application window
                  will be minimized, maximized, or shown normally
        Returns:  HRESULT
                    Status code
    F---F---F---F---F---F---F---F---F---F---F---F---F---F---F---F---F-F*/
    HRESULT InitWindow(_In_ HINSTANCE hInstance, _In_ INT nCmdShow);

    /*F+F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F
      Function: InitDevice
      Summary:  Create Direct3D device and swap chain
      Returns:  HRESULT
                  Status code
    F---F---F---F---F---F---F---F---F---F---F---F---F---F---F---F---F-F*/
    HRESULT InitDevice();

    /*F+F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F
      Function: CleanupDevice
      Summary:  Clean up the objects we've created
    F---F---F---F---F---F---F---F---F---F---F---F---F---F---F---F---F-F*/
    void CleanupDevice();

    /*F+F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F
      Function: Render
      Summary:  Render the frame
    F---F---F---F---F---F---F---F---F---F---F---F---F---F---F---F---F-F*/
    void Render();

    /*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
      Class:    Game
      Summary:  Main game engine class
      Methods:  Initialize
                  Initializes the components of the game
                Run
                  Runs the game loop
                GetGameName
                  Returns the name of the game
                Game
                  Constructor.
                ~Game
                  Destructor.
    C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
    class Game final
    {
    public:
        Game(_In_ PCWSTR pszGameName);
        Game(const Game& other) = delete;
        Game(Game&& other) = delete;
        Game& operator=(const Game& other) = delete;
        Game& operator=(Game&& other) = delete;
        ~Game() = default;

        HRESULT Initialize(_In_ HINSTANCE hInstance, _In_ INT nCmdShow);

        INT Run();

        PCWSTR GetGameName() const;
    private:
        PCWSTR m_pszGameName;
        std::unique_ptr<MainWindow> m_mainWindow;
        std::unique_ptr<Renderer> m_renderer;
    };
}