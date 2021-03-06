require "Dependencies/Helium"
require "Dependencies/tbb"
require "Dependencies/fbx"

require "Helium"

project( prefix .. "Platform" )

	Helium.DoModuleProjectSettings( ".", "HELIUM", "Platform", "PLATFORM" )

	files
	{
		"Platform/*.cpp",
		"Platform/*.h",
		"Platform/*.inl",
	}

	configuration "windows"
		excludes
		{
			"Platform/*Posix.*",
			"Platform/*Mac.*",
			"Platform/*Lin.*",
		}

	configuration "macosx"
		excludes
		{
			"Platform/*Win.*",
			"Platform/*Lin.*",
		}

	configuration "linux"
		excludes
		{
			"Platform/*Win.*",
			"Platform/*Mac.*",
		}

project( prefix .. "Foundation" )

	Helium.DoModuleProjectSettings( ".", "HELIUM", "Foundation", "FOUNDATION" )

	files
	{
		"Foundation/**",
	}

	configuration "SharedLib"
		links
		{
			prefix .. "Platform",
		}

project( prefix .. "Application" )

	Helium.DoModuleProjectSettings( ".", "HELIUM", "Application", "APPLICATION" )

	files
	{
		"Application/**",
	}

	configuration "SharedLib"
		links
		{
			prefix .. "Platform",
			prefix .. "Foundation",
		}

project( prefix .. "Reflect" )

	Helium.DoModuleProjectSettings( ".", "HELIUM", "Reflect", "REFLECT" )

	files
	{
		"Reflect/**",
	}

	configuration "SharedLib"
		links
		{
			prefix .. "Platform",
			prefix .. "Foundation",
		}

project( prefix .. "Persist" )

	Helium.DoModuleProjectSettings( ".", "HELIUM", "Persist", "PERSIST" )

	files
	{
		"Persist/**",
	}

	configuration "SharedLib"
		links
		{
			prefix .. "Platform",
			prefix .. "Foundation",
			prefix .. "Reflect",
			"mongo-c",
		}

project( prefix .. "Mongo" )

	Helium.DoModuleProjectSettings( ".", "HELIUM", "Mongo", "MONGO" )

	files
	{
		"Mongo/**",
	}

	configuration "SharedLib"
		links
		{
			prefix .. "Platform",
			prefix .. "Foundation",
			prefix .. "Reflect",
			prefix .. "Persist",
			"mongo-c",
		}

project( prefix .. "Inspect" )

	Helium.DoModuleProjectSettings( ".", "HELIUM", "Inspect", "INSPECT" )

	files
	{
		"Inspect/**",
	}

	includedirs
	{
		"Inspect",
	}

	configuration "SharedLib"
		links
		{
			prefix .. "Platform",
			prefix .. "Foundation",
			prefix .. "Application",
			prefix .. "Reflect",
			prefix .. "Persist",
			prefix .. "Math",
		}

project( prefix .. "Math" )

	Helium.DoModuleProjectSettings( ".", "HELIUM", "Math", "MATH" )

	files
	{
		"Math/**",
	}

	configuration "SharedLib"
		links
		{
			prefix .. "Platform",
			prefix .. "Foundation",
			prefix .. "Reflect",
			prefix .. "Persist",
		}

project( prefix .. "MathSimd" )

	Helium.DoModuleProjectSettings( ".", "HELIUM", "MathSimd", "MATH_SIMD" )

	files
	{
		"MathSimd/**",
	}

	configuration "SharedLib"
		links
		{
			prefix .. "Platform",
			prefix .. "Foundation",
			prefix .. "Reflect",
			prefix .. "Persist",
		}
