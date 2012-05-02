//----------------------------------------------------------------------------------------------------------------------
// Engine.h
//
// Copyright (C) 2010 WhiteMoon Dreams, Inc.
// All Rights Reserved
//----------------------------------------------------------------------------------------------------------------------

#pragma once
#ifndef HELIUM_ENGINE_ENGINE_H
#define HELIUM_ENGINE_ENGINE_H

#include "Platform/Platform.h"  // Always make sure Platform.h gets included first.

#if HELIUM_SHARED
    #ifdef HELIUM_ENGINE_EXPORTS
        #define HELIUM_ENGINE_API HELIUM_API_EXPORT
    #else
        #define HELIUM_ENGINE_API HELIUM_API_IMPORT
    #endif
#else
    #define HELIUM_ENGINE_API
#endif

#endif  // HELIUM_ENGINE_ENGINE_H

#define OBJECT_CREATION_STREAM (1<<5)
