#include "ApplicationPch.h"
#include "Preferences.h"

#include "Platform/Process.h"

bool Helium::GetPreferencesDirectory( Helium::FilePath& preferencesDirectory )
{
    std::string prefDirectory = Helium::GetPreferencesDirectory();
	if ( !prefDirectory.empty() )
    {
		prefDirectory += TXT( "/.Helium/" );
		preferencesDirectory.Set( prefDirectory );
    }

    return false;
}