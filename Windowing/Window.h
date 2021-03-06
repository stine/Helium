#pragma once

#include "Windowing/Windowing.h"

#include "Foundation/Event.h"

namespace Helium
{
    /// Interface to a window.
    class HELIUM_WINDOWING_API Window : NonCopyable
    {
    public:
#if HELIUM_OS_WIN
		typedef HWND Handle;
#else
        typedef void* Handle;
#endif

        /// Creation parameters.
        struct HELIUM_WINDOWING_API Parameters
        {
            /// Window title.
            const char* pTitle;
            /// Window width, in pixels.
            uint32_t width;
            /// Window height, in pixels.
            uint32_t height;
            /// True to create the window for presentation as a full-screen display, false to initialize for a windowed
            /// display.
            bool bFullscreen;

            /// @name Construction/Destruction
            //@{
            inline Parameters();
            //@}
        };

        /// @name Construction/Destruction
        //@{
        Window();
        Window( Handle pHandle, const char* pTitle, uint32_t width, uint32_t height, bool bFullscreen );
        ~Window();
        //@}

		/// @name Window Management
        //@{
        void Destroy();
        //@}

        /// @name Data Access
        //@{
        void Set( Handle pHandle, const char* pTitle, uint32_t width, uint32_t height, bool bFullscreen );
        inline void* GetHandle() const;
        inline const String& GetTitle() const;
        inline uint32_t GetWidth() const;
        inline uint32_t GetHeight() const;
        inline bool GetFullscreen() const;
        //@}

        /// @name Callback Registration
        //@{
        void SetOnDestroyed( const Delegate<Window*>& rOnDestroyed );
        inline const Delegate<Window*>& GetOnDestroyed() const;
        //@}

    protected:
        /// Callback to execute when this window is actually destroyed.
        Delegate<Window*> m_onDestroyed;

        /// Platform-dependent window handle.
        Handle m_pHandle;
        /// Window title.
        String m_title;
        /// Window width, in pixels.
        uint32_t m_width;
        /// Window height, in pixels.
        uint32_t m_height;
        /// True if the window is configured for display as a full-screen window, false if it is set up for windowed
        /// display.
        bool m_bFullscreen;
    };
}

#include "Windowing/Window.inl"
