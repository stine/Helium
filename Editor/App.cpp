#include "EditorPch.h"
#include "App.h"

#include "Platform/Debug.h"
#include "Platform/Process.h"
#include "Platform/Exception.h"
#include "Platform/Trace.h"
#include "Platform/Timer.h"
#include "Platform/Console.h"
#include "Platform/Timer.h"

#include "Foundation/Math/Common.h"

#include "Foundation/Log.h"
#include "Foundation/Startup.h"
#include "Foundation/Exception.h"
#include "Foundation/SettingsManager.h"
#include "Foundation/InitializerStack.h"
#include "Foundation/AsyncLoader.h"
#include "Foundation/CommandLine/Option.h"
#include "Foundation/CommandLine/Command.h"
#include "Foundation/CommandLine/Commands/Help.h"
#include "Foundation/CommandLine/Processor.h"
#include "Foundation/Document/Document.h"
#include "Foundation/File/File.h"
#include "Foundation/Inspect/Inspect.h"
#include "Foundation/Inspect/Interpreters/Reflect/InspectReflectInit.h"
#include "Foundation/Name.h"
#include "Foundation/Reflect/Registry.h"
#include "Foundation/Worker/Process.h"

#include "Engine/CacheManager.h"
#include "Engine/Config.h"
#include "Engine/GameObjectType.h"
#include "Engine/Package.h"
#include "Engine/JobManager.h"

#include "EngineJobs/EngineJobs.h"

#include "GraphicsJobs/GraphicsJobs.h"

#include "Graphics/GraphicsCustomTypeRegistration.h"
#include "Framework/FrameworkCustomTypeRegistration.h"

#include "PcSupport/ConfigPc.h"
#include "PcSupport/ObjectPreprocessor.h"
#include "PcSupport/PlatformPreprocessor.h"

#include "PreprocessingPc/PcPreprocessor.h"

#include "EditorSupport/EditorObjectLoader.h"
#include "EditorSupport/FontResourceHandler.h"

#include "SceneGraph/SceneGraphInit.h"

#include "Editor/ArtProvider.h"
#include "Editor/Input.h"
#include "Editor/EditorGenerated.h"
#include "Editor/Perforce/Perforce.h"
#include "Editor/ProjectViewModel.h"
#include "Editor/Settings/EditorSettings.h"
#include "Editor/Settings/WindowSettings.h"
#include "Editor/Tracker/Tracker.h"
#include "Editor/Task/TaskInit.h"
#include "Editor/Perforce/Perforce.h"
#include "Editor/Dialogs/PerforceWaitDialog.h"
#include "Editor/Vault/VaultSettings.h"

#include "Editor/Commands/ProfileDumpCommand.h"
#include "Editor/Commands/RebuildCommand.h"

#include "Editor/Clipboard/ClipboardDataWrapper.h"
#include "Editor/Clipboard/ClipboardFileList.h"

#include "Editor/Inspect/Widgets/DrawerWidget.h"
#include "Editor/Inspect/Widgets/LabelWidget.h"
#include "Editor/Inspect/Widgets/ValueWidget.h"
#include "Editor/Inspect/Widgets/SliderWidget.h"
#include "Editor/Inspect/Widgets/ChoiceWidget.h"
#include "Editor/Inspect/Widgets/CheckBoxWidget.h"
#include "Editor/Inspect/Widgets/ColorPickerWidget.h"
#include "Editor/Inspect/Widgets/ListWidget.h"
#include "Editor/Inspect/Widgets/ButtonWidget.h"
#include "Editor/Inspect/Widgets/FileDialogButtonWidget.h"
#include "Editor/Inspect/TreeCanvas.h"
#include "Editor/Inspect/TreeCanvasWidget.h"
#include "Editor/Inspect/StripCanvas.h"
#include "Editor/Inspect/StripCanvasWidget.h"

#include <set>
#include <tchar.h>
#include <wx/wx.h>
#include <wx/choicdlg.h>
#include <wx/msw/private.h>
#include <wx/cmdline.h>
#include <wx/splash.h>
#include <wx/cshelp.h>

using namespace Helium;
using namespace Helium::Editor;
using namespace Helium::CommandLine;
using namespace Helium;

extern void RegisterEngineTypes();
extern void RegisterGraphicsTypes();
extern void RegisterFrameworkTypes();
extern void RegisterPcSupportTypes();
extern void RegisterEditorSupportTypes();

extern void UnregisterEngineTypes();
extern void UnregisterGraphicsTypes();
extern void UnregisterFrameworkTypes();
extern void UnregisterPcSupportTypes();
extern void UnregisterEditorSupportTypes();

static void ShowBreakpointDialog(const Debug::BreakpointArgs& args )
{
    static std::set<uintptr_t> disabled;
    static bool skipAll = false;
    bool skip = skipAll;

    // are we NOT skipping everything?
    if (!skipAll)
    {
        // have we disabled this break point?
        if (disabled.find(args.m_Info->ContextRecord->IPREG) != disabled.end())
        {
            skip = true;
        }
        // we have NOT disabled this break point yet
        else
        {
            Debug::ExceptionArgs exArgs ( Debug::ExceptionTypes::SEH, args.m_Fatal ); 
            Debug::GetExceptionDetails( args.m_Info, exArgs ); 

            // dump args.m_Info to console
            Helium::Print(Helium::ConsoleColors::Red, stderr, TXT( "%s" ), Debug::GetExceptionInfo(args.m_Info).c_str());

            // display result
            tstring message( TXT( "A break point was triggered in the application:\n\n" ) );
            message += Debug::GetSymbolInfo( args.m_Info->ContextRecord->IPREG );
            message += TXT("\n\nWhat do you wish to do?");

            const tchar_t* nothing = TXT( "Let the OS handle this as an exception" );
            const tchar_t* thisOnce = TXT( "Skip this break point once" );
            const tchar_t* thisDisable = TXT( "Skip this break point and disable it" );
            const tchar_t* allDisable = TXT( "Skip all break points" );

            wxArrayString choices;
            choices.Add(nothing);
            choices.Add(thisOnce);
            choices.Add(thisDisable);
            choices.Add(allDisable);
            wxString choice = ::wxGetSingleChoice( message.c_str(), TXT( "Break Point Triggered" ), choices );

            if (choice == nothing)
            {
                // we are not continuable, so unhook the top level filter
                SetUnhandledExceptionFilter( NULL );

                // this should let the OS prompt for the debugger
                args.m_Result = EXCEPTION_CONTINUE_SEARCH;
                return;
            }
            else if (choice == thisOnce)
            {
                skip = true;
            }
            else if (choice == thisDisable)
            {
                skip = true;
                disabled.insert(args.m_Info->ContextRecord->IPREG);
            }
            else if (choice == allDisable)
            {
                skip = true;
                skipAll = true;
            }
        }
    }

    if (skipAll || skip)
    {
        // skip break instruction (move the ip ahead one byte)
        args.m_Info->ContextRecord->IPREG += 1;

        // continue execution past the break instruction
        args.m_Result = EXCEPTION_CONTINUE_EXECUTION;
    }
    else
    {
        // fall through and let window's crash API run
        args.m_Result = EXCEPTION_CONTINUE_SEARCH;
    }
}

namespace Helium
{
    namespace Editor
    {
        IMPLEMENT_APP( App );
    }
}

App::App()
#pragma TODO("This needs fixing otherwise dialogs will not be modal -Geoff")
: m_AppVersion( HELIUM_APP_VERSION )
, m_AppName( HELIUM_APP_NAME )
, m_AppVerName( HELIUM_APP_VER_NAME )
, m_SettingsManager( new SettingsManager() )
, m_Frame( NULL )
{
}

App::~App()
{
}

///////////////////////////////////////////////////////////////////////////////
// Called after OnInitCmdLine.  The base class handles the /help command line
// switch and exits.  If we get this far, we need to parse the command line
// and determine what mode to launch the app in.
// 
bool App::OnInit()
{
    SetVendorName( HELIUM_APP_NAME );

    Timer::StaticInitialize();

    // don't spend a lot of time updating idle events for windows that don't need it
    wxUpdateUIEvent::SetMode( wxUPDATE_UI_PROCESS_SPECIFIED );
    wxIdleEvent::SetMode( wxIDLE_PROCESS_SPECIFIED );

    tchar_t module[MAX_PATH];
    GetModuleFileName( 0, module, MAX_PATH );

    Helium::Path exePath( module );
    Helium::Path iconFolder( exePath.Directory() + TXT( "Icons/" ) );

    wxInitAllImageHandlers();
    wxImageHandler* curHandler = wxImage::FindHandler( wxBITMAP_TYPE_CUR );
    if ( curHandler )
    {
        // Force the cursor handler to the end of the list so that it doesn't try to
        // open TGA files.
        wxImage::RemoveHandler( curHandler->GetName() );
        curHandler = NULL;
        wxImage::AddHandler( new wxCURHandler );
    }

    ArtProvider* artProvider = new ArtProvider();
    wxArtProvider::Push( artProvider );

    wxSimpleHelpProvider* helpProvider = new wxSimpleHelpProvider();
    wxHelpProvider::Set( helpProvider );

    // Make sure various module-specific heaps are initialized from the main thread before use.
    InitEngineJobsDefaultHeap();
    InitGraphicsJobsDefaultHeap();

    // Register shutdown for general systems.
    m_InitializerStack.Push( File::Shutdown );
    m_InitializerStack.Push( CharName::Shutdown );
    m_InitializerStack.Push( WideName::Shutdown );
    m_InitializerStack.Push( GameObjectPath::Shutdown );

    // Async I/O.
    AsyncLoader& asyncLoader = AsyncLoader::GetStaticInstance();
    HELIUM_VERIFY( asyncLoader.Initialize() );
    m_InitializerStack.Push( AsyncLoader::DestroyStaticInstance );

    // GameObject cache management.
    Path baseDirectory;
    if ( !File::GetBaseDirectory( baseDirectory ) )
    {
        HELIUM_TRACE( TRACE_ERROR, TXT( "Could not get base directory." ) );
        return false;
    }

    HELIUM_VERIFY( CacheManager::InitializeStaticInstance( baseDirectory ) );
    m_InitializerStack.Push( CacheManager::DestroyStaticInstance );

    // FreeType support.
    HELIUM_VERIFY( FontResourceHandler::InitializeStaticLibrary() );
    m_InitializerStack.Push( FontResourceHandler::DestroyStaticLibrary );

    // libs
    Editor::PerforceWaitDialog::Enable( true );
    m_InitializerStack.Push( Perforce::Initialize, Perforce::Cleanup );
    m_InitializerStack.Push( Reflect::ObjectRefCountSupport::Shutdown );
    m_InitializerStack.Push( Reflect::Initialize, Reflect::Cleanup );
    m_InitializerStack.Push( Inspect::Initialize, Inspect::Cleanup );
    m_InitializerStack.Push( InspectReflect::Initialize, InspectReflect::Cleanup );
    m_InitializerStack.Push( SceneGraph::Initialize,  SceneGraph::Cleanup );
    m_InitializerStack.Push( TaskInitialize, TaskCleanup );

    // inspect
    m_InitializerStack.Push( Reflect::RegisterClassType< Editor::Widget >( TXT("Editor::Widget") ) );
    m_InitializerStack.Push( Reflect::RegisterClassType< Editor::DrawerWidget >( TXT("Editor::DrawerWidget") ) );
    m_InitializerStack.Push( Reflect::RegisterClassType< Editor::LabelWidget >( TXT("Editor::LabelWidget") ) );
    m_InitializerStack.Push( Reflect::RegisterClassType< Editor::ValueWidget >( TXT("Editor::ValueWidget") ) );
    m_InitializerStack.Push( Reflect::RegisterClassType< Editor::SliderWidget >( TXT("Editor::SliderWidget") ) );
    m_InitializerStack.Push( Reflect::RegisterClassType< Editor::ChoiceWidget >( TXT("Editor::ChoiceWidget") ) );
    m_InitializerStack.Push( Reflect::RegisterClassType< Editor::CheckBoxWidget >( TXT("Editor::CheckBoxWidget") ) );
    m_InitializerStack.Push( Reflect::RegisterClassType< Editor::ColorPickerWidget >( TXT("Editor::ColorPickerWidget") ) );
    m_InitializerStack.Push( Reflect::RegisterClassType< Editor::ListWidget >( TXT("Editor::ListWidget") ) );
    m_InitializerStack.Push( Reflect::RegisterClassType< Editor::ButtonWidget >(TXT("Editor::ButtonWidget") ) );
    m_InitializerStack.Push( Reflect::RegisterClassType< Editor::FileDialogButtonWidget >( TXT("Editor::FileDialogButtonWidget") ) );
    m_InitializerStack.Push( Reflect::RegisterClassType< Editor::Canvas >( TXT("Editor::Canvas") ) );
    m_InitializerStack.Push( Reflect::RegisterClassType< Editor::TreeCanvas >( TXT("Editor::TreeCanvas") ) );
    m_InitializerStack.Push( Reflect::RegisterClassType< Editor::TreeCanvasWidget >( TXT("Editor::TreeCanvasWidget") ) );
    m_InitializerStack.Push( Reflect::RegisterClassType< Editor::StripCanvas >( TXT("Editor::StripCanvas") ) );
    m_InitializerStack.Push( Reflect::RegisterClassType< Editor::StripCanvasWidget >( TXT("Editor::StripCanvasWidget") ) );

    // clipboard
    m_InitializerStack.Push( Reflect::RegisterClassType< ReflectClipboardData >( TXT("Editor::ReflectClipboardData") ) );
    m_InitializerStack.Push( Reflect::RegisterClassType< ClipboardDataWrapper >( TXT("Editor::ClipboardDataWrapper") ) );
    m_InitializerStack.Push( Reflect::RegisterClassType< ClipboardFileList >( TXT("Editor::ClipboardFileList") ) );

    // vault
    m_InitializerStack.Push( Reflect::RegisterClassType< VaultSearchQuery >( TXT( "Editor::VaultSearchQuery" ) ) );   
    m_InitializerStack.Push( Reflect::RegisterEnumType< Editor::VaultViewMode >( TXT( "Editor::VaultViewMode" ) ) );

    // settings
    m_InitializerStack.Push( Reflect::RegisterEnumType< IconSize >( TXT( "Editor::IconSize" ) ) );
    m_InitializerStack.Push( Reflect::RegisterClassType< EditorSettings >( TXT( "Editor::EditorSettings" ) ) );
    Reflect::GetClass< EditorSettings >()->SetProperty( TXT( "UIName" ), TXT( "Editor Settings" ) );

    m_InitializerStack.Push( Reflect::RegisterClassType< VaultSettings >( TXT( "Editor::VaultSettings" ) ) );
    Reflect::GetClass< VaultSettings >()->SetProperty( TXT( "UIName" ), TXT( "Vault Settings" ) );

    m_InitializerStack.Push( Reflect::RegisterClassType< WindowSettings >( TXT( "Editor::WindowSettings" ) ) );
    Reflect::GetClass< WindowSettings >()->SetProperty( TXT( "UIName" ), TXT( "Window Settings" ) );

    m_InitializerStack.Push( Reflect::RegisterEnumType< Editor::ProjectMenuID >( TXT( "Editor::ProjectMenuID" ) ) );

    // Engine type registration.
    m_InitializerStack.Push( GameObject::Shutdown );
    m_InitializerStack.Push( GameObjectType::Shutdown );
    m_InitializerStack.Push( RegisterEngineTypes, UnregisterEngineTypes );
    m_InitializerStack.Push( PreRegisterGraphicsTypes, PostRegisterGraphicsTypes );
    m_InitializerStack.Push( RegisterGraphicsTypes, UnregisterGraphicsTypes );
    m_InitializerStack.Push( PreRegisterFrameworkTypes, PostRegisterFrameworkTypes);
    m_InitializerStack.Push( RegisterFrameworkTypes, UnregisterFrameworkTypes );
    m_InitializerStack.Push( RegisterPcSupportTypes, UnregisterPcSupportTypes );
    m_InitializerStack.Push( RegisterEditorSupportTypes, UnregisterEditorSupportTypes );

    // GameObject loader and preprocessor.
    HELIUM_VERIFY( EditorObjectLoader::InitializeStaticInstance() );
    m_InitializerStack.Push( EditorObjectLoader::DestroyStaticInstance );

    GameObjectLoader* pObjectLoader = GameObjectLoader::GetStaticInstance();
    HELIUM_ASSERT( pObjectLoader );

    ObjectPreprocessor* pObjectPreprocessor = ObjectPreprocessor::CreateStaticInstance();
    HELIUM_ASSERT( pObjectPreprocessor );
    PlatformPreprocessor* pPlatformPreprocessor = new PcPreprocessor;
    HELIUM_ASSERT( pPlatformPreprocessor );
    pObjectPreprocessor->SetPlatformPreprocessor( Cache::PLATFORM_PC, pPlatformPreprocessor );

    m_InitializerStack.Push( ObjectPreprocessor::DestroyStaticInstance );

    // Engine configuration.
    Config& rConfig = Config::GetStaticInstance();
    rConfig.BeginLoad();
    while( !rConfig.TryFinishLoad() )
    {
        pObjectLoader->Tick();
    }

    m_InitializerStack.Push( Config::DestroyStaticInstance );

    ConfigPc::SaveUserConfig();

    // Job manager.
    JobManager& rJobManager = JobManager::GetStaticInstance();
    HELIUM_VERIFY( rJobManager.Initialize() );
    m_InitializerStack.Push( JobManager::DestroyStaticInstance );

    LoadSettings();

    if ( Log::GetErrorCount() )
    {
        wxMessageBox( TXT( "There were errors during startup, use Editor with caution." ), TXT( "Error" ), wxCENTER | wxICON_ERROR | wxOK );
    }

    Connect( wxEVT_CHAR, wxKeyEventHandler( App::OnChar ), NULL, this );

    m_Frame = new MainFrame( m_SettingsManager );
    m_Frame->Show();

    if ( GetSettingsManager()->GetSettings< EditorSettings >()->GetReopenLastProjectOnStartup() )
    {
        const std::vector< tstring >& mruPaths = wxGetApp().GetSettingsManager()->GetSettings<EditorSettings>()->GetMRUProjects();
        if ( !mruPaths.empty() )
        {
            Path projectPath( *mruPaths.rbegin() );
            if ( projectPath.Exists() )
            {
                m_Frame->OpenProject( *mruPaths.rbegin() );
            }
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// Called when the application is being exited.  Cleans up resources.
// 
int App::OnExit()
{
    Disconnect( wxEVT_CHAR, wxKeyEventHandler( App::OnChar ), NULL, this );

    SaveSettings();

    m_SettingsManager.Release();

    m_InitializerStack.Cleanup();

    wxImage::CleanUpHandlers();

    int result = wxApp::OnExit();

    // Always clear out memory heaps last.
    ThreadLocalStackAllocator::ReleaseMemoryHeap();

    return result;
}

void App::OnChar( wxKeyEvent& event )
{
    if ( !m_Frame )
    {
        return;
    }

    Helium::KeyboardInput input;
    Helium::ConvertEvent( event, input );
    tstring error;

    if ( input.IsCtrlDown() )
    {
        switch( input.GetKeyCode() )
        {
        case KeyCodes::a: // ctrl-a
            m_Frame->GetEventHandler()->ProcessEvent( wxCommandEvent( wxEVT_COMMAND_MENU_SELECTED, wxID_SELECTALL ) );
            event.Skip( false );
            return;
            break;

        case KeyCodes::i: // ctrl-i
            m_Frame->InvertSelection();
            event.Skip( false );
            return;
            break;

        case KeyCodes::o: // ctrl-o
            m_Frame->OpenProjectDialog();
            event.Skip( false );
            return;
            break;

        case KeyCodes::s: // ctrl-s
            if ( !m_Frame->SaveAll( error ) )
            {
                wxMessageBox( error.c_str(), wxT( "Error" ), wxCENTER | wxICON_ERROR | wxOK, m_Frame );
            }
            event.Skip( false );
            return;
            break;

        case KeyCodes::v: // ctrl-v
            m_Frame->GetEventHandler()->ProcessEvent( wxCommandEvent( wxEVT_COMMAND_MENU_SELECTED, wxID_PASTE ) );
            event.Skip( false );
            return;
            break;

        case KeyCodes::w: // ctrl-w
            m_Frame->CloseProject();
            event.Skip( false );
            return;
            break;

        case KeyCodes::x: // ctrl-x
            m_Frame->GetEventHandler()->ProcessEvent( wxCommandEvent( wxEVT_COMMAND_MENU_SELECTED, wxID_CUT ) );
            event.Skip( false );
            return;
            break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// Called when an assert failure occurs
// 
void App::OnAssertFailure(const wxChar *file, int line, const wxChar *func, const wxChar *cond, const wxChar *msg)
{
    HELIUM_BREAK();
}

///////////////////////////////////////////////////////////////////////////////
// Called when an exception occurs in the process of dispatching events
//  It is Helium's policy to not throw C++ exceptions into wxWidgets
//  If this is a Win32/SEH exception then set your debugger to break
//   on throw instead of break on user-unhandled
// 
void App::OnUnhandledException()
{
    HELIUM_BREAK();
}

///////////////////////////////////////////////////////////////////////////////
// See above
//
bool App::OnExceptionInMainLoop()
{
    HELIUM_BREAK();
    throw;
}

void App::SaveSettings()
{
    Helium::Path path;
    Helium::GetPreferencesDirectory( path );
    path += TXT("EditorSettings.xml");

    tstring error;

    if ( !path.MakePath() )
    {
        error = tstring( TXT( "Could not save '" ) ) + path.c_str() + TXT( "': We could not create the directory to store the settings file." );
        wxMessageBox( error.c_str(), wxT( "Error" ), wxOK | wxCENTER | wxICON_ERROR );
        return;
    }

    if ( Helium::IsDebuggerPresent() )
    {
        Reflect::ToArchive( path, m_SettingsManager, Reflect::ArchiveTypes::XML );
    }
    else
    {
        if ( !Reflect::ToArchive( path, m_SettingsManager, Reflect::ArchiveTypes::XML ) )
        {
            error = tstring( TXT( "Could not save '" ) ) + path.c_str() + TXT( "'." );
            wxMessageBox( error.c_str(), wxT( "Error" ), wxOK | wxCENTER | wxICON_ERROR );
        }
    }
}

void App::LoadSettings()
{
    Helium::Path path;
    Helium::GetPreferencesDirectory( path );
    path += TXT("EditorSettings.xml");

    if ( !path.Exists() )
    {
        return;
    }

    SettingsManagerPtr settingsManager = Reflect::FromArchive< SettingsManager >( path, Reflect::ArchiveTypes::XML );
    if ( settingsManager.ReferencesObject() )
    {
        settingsManager->Clean();
        m_SettingsManager = settingsManager;
    }
    else
    {
        wxMessageBox( TXT( "Unfortunately, we could not parse your existing settings.  Your settings have been reset to defaults.  We apologize for the inconvenience." ), wxT( "Error" ), wxOK | wxCENTER | wxICON_ERROR );
    }
}

#pragma TODO("Apparently wxWidgets doesn't support unicode command lines, please to fix in wxWidgets 2.9.x")
static int wxEntryWrapper(HINSTANCE hInstance, HINSTANCE hPrevInstance, tchar_t* pCmdLine, int nCmdShow)
{
    std::string cmdLine;
    Helium::ConvertString( pCmdLine, cmdLine );
    return wxEntry( hInstance, hPrevInstance, const_cast<char*>(cmdLine.c_str()), nCmdShow );
}

/////////////////////////////////////////////////////////////////////////////////
int Main( int argc, const tchar_t** argv )
{
    // print physical memory
    MEMORYSTATUSEX status;
    memset(&status, 0, sizeof(status));
    status.dwLength = sizeof(status);
    ::GlobalMemoryStatusEx(&status);
    Log::Print( TXT( "Physical Memory: %I64u M bytes total, %I64u M bytes available\n" ), status.ullTotalPhys >> 20, status.ullAvailPhys >> 20);

    // fill out the options vector
    std::vector< tstring > options;
    for ( int i = 1; i < argc; ++i )
    {
        options.push_back( argv[ i ] );
    }
    std::vector< tstring >::const_iterator& argsBegin = options.begin(), argsEnd = options.end();

    bool success = true;
    tstring error; 

    Processor processor( TXT( "luna" ), TXT( "[COMMAND <ARGS>]" ), TXT( "Editor (c) 2010 - Helium" ) );

    ProfileDumpCommand profileDumpCommand;
    success &= profileDumpCommand.Initialize( error );
    success &= processor.RegisterCommand( &profileDumpCommand, error );

    RebuildCommand rebuildCommand;
    success &= rebuildCommand.Initialize( error );
    success &= processor.RegisterCommand( &rebuildCommand, error );

    Helium::CommandLine::Help helpCommand;
    helpCommand.SetOwner( &processor );
    success &= helpCommand.Initialize( error );
    success &= processor.RegisterCommand( &helpCommand, error );

    //success &= processor.AddOption( new FlagOption(  , "pipe", "use pipe for console connection" ), error ); 

    bool disableTracker = false;
    success &= processor.AddOption( new FlagOption( &disableTracker, TXT( "disable_tracker" ), TXT( "disable Asset Tracker" ) ), error );
    if ( disableTracker )
    {
        wxGetApp().GetSettingsManager()->GetSettings< EditorSettings >()->SetEnableAssetTracker( false );
    }

    //success &= processor.AddOption( new FlagOption(  , WindowSettings::s_Reset, "reset all window positions" ), error );
    //success &= processor.AddOption( new FlagOption(  , Settings::s_ResetSettings, "resets all preferences for all of Editor" ), error );

    //success &= processor.AddOption( new FlagOption(  , Worker::Args::Debug, "debug use of background processes" ), error );
    //success &= processor.AddOption( new FlagOption(  , Worker::Args::Wait, "wait forever for background processes" ), error );

    bool scriptFlag = false;
    success &= processor.AddOption( new FlagOption( &scriptFlag, StartupArgs::Script, TXT( "omit prefix and suffix in console output" ) ), error );

    bool attachFlag = false;
    success &= processor.AddOption( new FlagOption( &attachFlag, StartupArgs::Attach, TXT( "wait for a debugger to attach to the process on startup" ) ), error );

    bool profileFlag = false;
    success &= processor.AddOption( new FlagOption( &profileFlag, StartupArgs::Profile, TXT( "enable profile output to the console windows" ) ), error );

    bool memoryFlag = false;
    success &= processor.AddOption( new FlagOption( &memoryFlag, StartupArgs::Memory, TXT( "profile and report memory usage to the console" ) ), error );

    bool vreboseFlag = false;
    success &= processor.AddOption( new FlagOption( &vreboseFlag, StartupArgs::Verbose, TXT( "output a verbose level of console output" ) ), error );

    bool extremeFlag = false;
    success &= processor.AddOption( new FlagOption( &extremeFlag, StartupArgs::Extreme, TXT( "output an extremely verbose level of console output" ) ), error );

    bool debugFlag = false;
    success &= processor.AddOption( new FlagOption( &debugFlag, StartupArgs::Debug, TXT( "output debug console output" ) ), error );

    int nice = 0;
    success &= processor.AddOption( new SimpleOption<int>( &nice , TXT( "nice" ), TXT( "<NUM>" ), TXT( "number of processors to nice (for other processes)" ) ), error );

    bool helpFlag;
    success &= processor.AddOption( new FlagOption( &helpFlag, TXT( "h|help" ), TXT( "print program usage" ) ), error );

    success &= processor.ParseOptions( argsBegin, argsEnd, error );

    if ( success )
    {
        if ( helpFlag )
        {
            Log::Print( TXT( "\nPrinting help for Editor...\n" ) );
            Log::Print( processor.Help().c_str() );
            Log::Print( TXT( "\n" ) );
            success = true;
        }
        else if ( argsBegin != argsEnd )
        {
            while ( success && ( argsBegin != argsEnd ) )
            {
                const tstring& arg = (*argsBegin);
                ++argsBegin;

                if ( arg.length() < 1 )
                {
                    continue;
                }

                if ( arg[ 0 ] == '-' )
                {
                    error = TXT( "Unknown option, or option passed out of order: " ) + arg;
                    success = false;
                }
                else
                {
                    Command* command = processor.GetCommand( arg );
                    if ( command )
                    {
                        success = command->Process( argsBegin, argsEnd, error );
                    }
                    else
                    {
                        error = TXT( "Unknown commandline parameter: " ) + arg + TXT( "\n\n" );
                        success = false;
                    }
                }
            }
        }
        else
        {
            rebuildCommand.Cleanup();

#ifndef _DEBUG
            ::FreeConsole();
#endif

            return Helium::StandardWinMain( &wxEntryWrapper );
        }
    }

    rebuildCommand.Cleanup();

    if ( !success && !error.empty() )
    {
        Log::Error( TXT( "%s\n" ), error.c_str() );
    }

    return success ? 0 : 1;
}


///////////////////////////////////////////////////////////////////////////////
// Main entry point for the application.
//
int _tmain( int argc, const tchar_t** argv )
{
    Helium::InitializerStack initializerStack( true );

    Debug::g_BreakpointOccurred.Set( &ShowBreakpointDialog );

    int result = Helium::StandardMain( &Main, argc, argv );

    Debug::g_BreakpointOccurred.Clear();

    return result;
}
