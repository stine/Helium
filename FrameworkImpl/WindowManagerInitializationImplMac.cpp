#include "FrameworkImplPch.h"
#include "FrameworkImpl/WindowManagerInitializationImpl.h"

#include "Windowing/WindowManager.h"

using namespace Helium;

/// Constructor.
///
/// @param[in] hInstance  Handle to the application instance.
/// @param[in] nCmdShow   Flags specifying how the application window should be shown (passed in from WinMain()).
WindowManagerInitializationImpl::WindowManagerInitializationImpl()
{
}

/// @copydoc WindowManager::Initialize()
bool WindowManagerInitializationImpl::Initialize()
{
    WindowManager* pWindowManager = WindowManager::CreateStaticInstance();
    HELIUM_ASSERT( pWindowManager );
    if( !pWindowManager )
    {
        return false;
    }

    if( !pWindowManager->Initialize() )
    {
        WindowManager::DestroyStaticInstance();

        return false;
    }

    return true;
}
