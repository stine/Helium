//----------------------------------------------------------------------------------------------------------------------
// ArchiveObjectLoader.cpp
//
// Copyright (C) 2010 WhiteMoon Dreams, Inc.
// All Rights Reserved
//----------------------------------------------------------------------------------------------------------------------

#include "PcSupportPch.h"

#include "PcSupport/ArchiveObjectLoader.h"

#include "Foundation/File/File.h"
#include "Foundation/File/Path.h"
#include "Engine/Config.h"
#include "Engine/Resource.h"
#include "PcSupport/ObjectPreprocessor.h"
#include "PcSupport/ArchivePackageLoader.h"
#include "Framework/Mesh.h"
#include "Foundation/Log.h"

using namespace Helium;

/// Constructor.
ArchiveObjectLoader::ArchiveObjectLoader()
{
    Reflect::RegisterClassType<ObjectDescriptor>(TXT("ObjectDescriptor"));
}

/// Destructor.
ArchiveObjectLoader::~ArchiveObjectLoader()
{
    Reflect::UnregisterClassType<ObjectDescriptor>();
}

/// Initialize the static object loader instance as an ArchiveObjectLoader.
///
/// @return  True if the loader was initialized successfully, false if not or another object loader instance already
///          exists.
bool ArchiveObjectLoader::InitializeStaticInstance()
{
    if( sm_pInstance )
    {
        return false;
    }

    sm_pInstance = new ArchiveObjectLoader;
    HELIUM_ASSERT( sm_pInstance );

    return true;
}

/// @copydoc GameObjectLoader::GetPackageLoader()
PackageLoader* ArchiveObjectLoader::GetPackageLoader( GameObjectPath path )
{
    ArchivePackageLoader* pLoader = m_packageLoaderMap.GetPackageLoader( path );

    return pLoader;
}

/// @copydoc GameObjectLoader::TickPackageLoaders()
void ArchiveObjectLoader::TickPackageLoaders()
{
    m_packageLoaderMap.TickPackageLoaders();
}

/// @copydoc GameObjectLoader::OnLoadComplete()
void ArchiveObjectLoader::OnLoadComplete( GameObjectPath /*path*/, GameObject* pObject, PackageLoader* /*pPackageLoader*/ )
{
    if( pObject )
    {
        CacheObject( pObject, true );
    }
}

/// @copydoc GameObjectLoader::OnPrecacheReady()
 void ArchiveObjectLoader::OnPrecacheReady( GameObject* pObject, PackageLoader* pPackageLoader )
 {
     HELIUM_ASSERT( pObject );
     HELIUM_ASSERT( pPackageLoader );
 
     // The default template object for a given type never has its resource data preprocessed, so there's no need to
     // precache default template objects.
     if( pObject->IsDefaultTemplate() )
     {
         return;
     }
 
     // Retrieve the object preprocessor if it exists.
     ObjectPreprocessor* pObjectPreprocessor = ObjectPreprocessor::GetStaticInstance();
     if( !pObjectPreprocessor )
     {
         HELIUM_TRACE(
             TRACE_WARNING,
             ( TXT( "ArchiveObjectLoader::OnPrecacheReady(): Missing ObjectPreprocessor to use for resource " )
             TXT( "preprocessing.\n" ) ) );
 
         return;
     }
 
     // We only need to do precache handling for resources, so skip non-resource types.
     Resource* pResource = Reflect::SafeCast< Resource >( pObject );
     if( !pResource )
     {
         return;
     }
 
     // Grab the package timestamp.
     HELIUM_ASSERT( pPackageLoader->IsSourcePackageFile() );
     int64_t objectTimestamp = pPackageLoader->GetFileTimestamp();
 
     // Attempt to load the resource data.
     pObjectPreprocessor->LoadResourceData( pResource, objectTimestamp );
 }

/// @copydoc GameObjectLoader::CacheObject()
bool ArchiveObjectLoader::CacheObject( GameObject* pObject, bool bEvictPlatformPreprocessedResourceData )
{
    HELIUM_ASSERT( pObject );

    // Don't cache broken objects or packages.
    if( pObject->GetAnyFlagSet( GameObject::FLAG_BROKEN ) || pObject->IsPackage() )
    {
        return false;
    }

    // Make sure we have an object preprocessor instance with which to cache the object.
    ObjectPreprocessor* pObjectPreprocessor = ObjectPreprocessor::GetStaticInstance();
    if( !pObjectPreprocessor )
    {
        HELIUM_TRACE(
            TRACE_WARNING,
            TXT( "ArchiveObjectLoader::CacheObject(): Missing ObjectPreprocessor to use for caching.\n" ) );

        return false;
    }

    // Configuration objects should not be cached.
    GameObjectPath objectPath = pObject->GetPath();

    Config& rConfig = Config::GetStaticInstance();
    GameObjectPath configPackagePath = rConfig.GetConfigContainerPackagePath();
    HELIUM_ASSERT( !configPackagePath.IsEmpty() );

    for( GameObjectPath testPath = objectPath; !testPath.IsEmpty(); testPath = testPath.GetParent() )
    {
        if( testPath == configPackagePath )
        {
            return false;
        }
    }

    // Get the timestamp for the object based on the timestamp of its source package file and, if it's a resource,
    // the timestamp of the source resource file.
    GameObject* pPackageObject;
    for( pPackageObject = pObject;
        pPackageObject && !pPackageObject->IsPackage();
        pPackageObject = pPackageObject->GetOwner() )
    {
    }

    HELIUM_ASSERT( pPackageObject );

    PackageLoader* pPackageLoader = Reflect::AssertCast< Package >( pPackageObject )->GetLoader();
    HELIUM_ASSERT( pPackageLoader );
    HELIUM_ASSERT( pPackageLoader->IsSourcePackageFile() );

    int64_t objectTimestamp = pPackageLoader->GetFileTimestamp();

    if( !pObject->IsDefaultTemplate() )
    {
        Resource* pResource = Reflect::SafeCast< Resource >( pObject );
        if( pResource )
        {
            GameObjectPath baseResourcePath = pResource->GetPath();
            HELIUM_ASSERT( !baseResourcePath.IsPackage() );
            for( ; ; )
            {
                GameObjectPath parentPath = baseResourcePath.GetParent();
                if( parentPath.IsEmpty() || parentPath.IsPackage() )
                {
                    break;
                }

                baseResourcePath = parentPath;
            }

            Path sourceFilePath;
            if ( !File::GetDataDirectory( sourceFilePath ) )
            {
                HELIUM_TRACE(
                    TRACE_WARNING,
                    TXT( "ArchiveObjectLoader::CacheObject(): Could not obtain data directory.\n" ) );

                return false;
            }

            sourceFilePath += baseResourcePath.ToFilePathString().GetData();

            int64_t sourceFileTimestamp = sourceFilePath.ModifiedTime();
            if( sourceFileTimestamp > objectTimestamp )
            {
                objectTimestamp = sourceFileTimestamp;
            }
        }
    }

    // Cache the object.
    bool bSuccess = pObjectPreprocessor->CacheObject(
        pObject,
        objectTimestamp,
        bEvictPlatformPreprocessedResourceData );
    if( !bSuccess )
    {
        HELIUM_TRACE(
            TRACE_ERROR,
            TXT( "ArchiveObjectLoader: Failed to cache object \"%s\".\n" ),
            *objectPath.ToString() );
    }

    return bSuccess;
}

void Helium::ArchiveObjectLoader::HACK_PostLink( GameObject *_game_object )
{
    if (_game_object && 
        _game_object->GetClass() && 
        _game_object->GetClass()->IsType(Helium::Mesh::GetStaticType()))
    {
        Helium::Mesh *mesh = Reflect::AssertCast<Helium::Mesh>(_game_object);

        if (mesh && mesh->GetMaterialCount() > 0)
        {
            tstringstream ss;
            ss << TXT("Post link mesh material: ") << mesh->GetMaterial(0) << TXT("\n");
            tstring str = ss.str();
            Log::PrintString(str.c_str(), OBJECT_CREATION_STREAM);

        }
    }

    if (_game_object && 
        _game_object->GetClass() && 
        _game_object->GetClass()->IsType(Helium::Material::GetStaticType()))
    {
        Helium::Material *material = Reflect::AssertCast<Helium::Material>(_game_object);

        if (material)
        {
            tstringstream ss;
            if (material->m_spShader.HasLinkIndex())
            {
                ss << TXT("Post link material shader: [LINK INDEX]\n");
            }
            else if (material->m_spShader.ReferencesObject())
            {
                ss << TXT("Post link material shader: ") << (material->m_spShader.ReferencesObject() ? material->m_spShader.Get() : 0) << TXT("\n");
            }
            tstring str = ss.str();
            Log::PrintString(str.c_str(), OBJECT_CREATION_STREAM);

        }

    }
}
