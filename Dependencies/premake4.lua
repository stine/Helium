local thisFileLocation = ...
if thisFileLocation == nil then
    thisFileLocation = '.'
end
thisFileLocation = path.getdirectory( thisFileLocation )

require( thisFileLocation .. '/Helium' )
require( thisFileLocation .. '/fbx' )
require( thisFileLocation .. '/tbb' )
require( thisFileLocation .. '/wxWidgets' )

function CheckEnvironment()

    print("\nChecking Environment...\n")
    
    if _PREMAKE_VERSION < Helium.RequiredPremakeVersion then
        print( "You must be running at least Premake " .. Helium.RequiredPremakeVersion .. "." )
        os.exit( 1 )
    end

    if os.get() == "windows" then
    
        local failed = 0
        
        if os.getenv( "VCINSTALLDIR" ) == nil then
            print( " -> You must be running in a Visual Studio Command Prompt.")
            failed = 1
        end

        if not failed then
            if os.pathsearch( 'cl.exe', os.getenv( 'PATH' ) ) == nil then
                print( " -> cl.exe was not found in your path.  Make sure you are using a Visual Studio 2008 SP1 Command Prompt." )
                failed = 1
            else
                compilerPath = "cl.exe"
            end

            local compilerVersion = ''
            local compilerVersionOutput = os.capture( "\"cl.exe\" 2>&1" )
            for major, minor, build in string.gmatch( compilerVersionOutput, "Version (%d+)\.(%d+)\.(%d+)" ) do
                compilerVersion = major .. minor .. build
            end
            
            if tonumber( compilerVersion ) < Helium.RequiredCLVersion then
                print( " -> You must have Visual Studio 2008 with SP1 applied to compile Helium.  Please update your compiler and tools." )
                failed = 1
            end
        end

        if os.getenv( "DXSDK_DIR" ) == nil then
            print( " -> You must have the DirectX SDK installed (DXSDK_DIR is not defined in your environment)." )
            failed = 1
        end

        local fbxDir = Helium.GetFbxSdkLocation()
        if not fbxDir or not os.isdir( fbxDir ) then
            print( " -> You must have the FBX SDK installed and the FBX_SDK environment variable set." )
            print( " -> Make sure to point the FBX_SDK environment variable at the FBX install location, eg: C:\\Program Files\\Autodesk\\FBX\\FbxSdk\\" .. Helium.RequiredFBXVersion )
            failed = 1
        end

        if failed == 1 then
            print( "\nCannot proceed until your environment is valid." )
            os.exit( 1 )
        end

    elseif os.get() == "macosx" then

        local major = 10
        local minor = 8
        local revision = 5
        local check = major * 10000 + minor * 100 + revision

        local ver = os.getversion()
        local number = ver.majorversion * 10000 + ver.minorversion * 100 + ver.revision
        if number < check then
            local str = string.format("%d.%d.%d", major, minor, revision)
            print( " -> Please update to OS X " .. str )
            failed = 1
        end

        if failed == 1 then
            print( "\nCannot proceed until your environment is valid." )
            os.exit( 1 )
        end

    end
end

newoption {
   trigger     = "no-wxwidgets",
   description = "Skip building wxWidgets, use system installed version"
}

newoption {
   trigger     = "no-tbb",
   description = "Skip building tbb, use system installed version"
}

newoption {
   trigger     = "no-fbx",
   description = "Skip fbx"
}

-- Do nothing if there is no action (--help, etc...)
if _ACTION then

	-- Check prereqs
	CheckEnvironment()

	if _ACTION == "xcode3" then
        print("XCode 3 is not supported")
        os.exit(1)
	end

    if _ACTION == "vs2002" then
        print("Visual Studio 2002 is not supported")
        os.exit(1)
    end

    if _ACTION == "vs2003" then
        print("Visual Studio 2003 is not supported")
        os.exit(1)
    end

	if _ACTION == "vs2005" then
        print("Visual Studio 2005 is not supported")
        os.exit(1)
	end

	if _ACTION ~= "clean" then
	
		local bin = "../Bin"

        if not _OPTIONS["no-wxwidgets"] then
    		Helium.BuildWxWidgets()
    		Helium.PublishWxWidgets( bin )
        end

        if not _OPTIONS["no-tbb"] then
    		Helium.BuildTbb()
    		Helium.PublishTbb( bin )
        end

        if not _OPTIONS["no-fbx"] then
    		Helium.PublishFbx( bin )
        end
		
	else
	
        if not _OPTIONS["no-wxwidgets"] then
    		Helium.CleanWxWidgets()
        end

        if not _OPTIONS["no-tbb"] then
    		Helium.CleanTbb()
        end
	
	end

	solution "Dependencies"
	Helium.DoBasicSolutionSettings()
	dofile "Dependencies.lua"

end
--]]