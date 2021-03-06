#pragma once

#include "Math/Color3.h"

#include "SceneGraph/API.h"
#include "SceneGraph/SettingsManager.h"

#include "Reflect/TranslatorDeduction.h"

namespace Helium
{
    namespace SceneGraph
    {
        class GridUnit
        {
        public:
            enum Enum
            {
                Meters,
                Centimeters,
            };

            HELIUM_DECLARE_ENUM( GridUnit );

            static void PopulateMetaType( Reflect::MetaEnum& info )
            {
                info.AddElement(Meters,         TXT( "Meters" ) );
                info.AddElement(Centimeters,    TXT( "Centimeters" ) );
            }
        };

        class HELIUM_SCENE_GRAPH_API GridSettings : public Settings
        {
        public:
            HELIUM_DECLARE_CLASS( GridSettings, Settings );

            GridSettings( const std::string& version = TXT( "" ),
                GridUnit units = GridUnit::Meters,
                uint32_t width = 12,
                uint32_t length = 12,
                float32_t majorStep = 5.0f,
                float32_t minorStep = 1.0f,
                Color3 axisColor = Color3( Vector3( 0.0f, 0.0f, 0.0f ) ),
                Color3 majorColor = Color3( Vector3( 0.5f, 0.5f, 0.5f ) ),
                Color3 minorColor = Color3( Vector3( 0.5f, 0.5f, 0.5f ) ) );
            ~GridSettings();

            virtual void PostDeserialize();

            uint32_t GetWidth();
            uint32_t GetLength();
            float32_t GetMajorStep();
            float32_t GetMinorStep();
            const Color3& GetAxisColor();
            const Color3& GetMajorColor();
            const Color3& GetMinorColor();

        private:
            void OnChanged( const Reflect::ObjectChangeArgs& args );

            static float32_t GetConversionFactor( GridUnit units );
            static float32_t ConvertUnits( float32_t sourceValue, GridUnit sourceUnits, GridUnit destinationUnits );

        private:
            GridUnit m_Units;
            GridUnit m_PreviousUnits;
            uint32_t m_Width;
            uint32_t m_Length;
            float32_t m_MajorStep;
            float32_t m_MinorStep;
            Color3 m_AxisColor;
            Color3 m_MajorColor;
            Color3 m_MinorColor;

        public:
            static void PopulateMetaType( Reflect::MetaStruct& comp )
            {
                comp.AddField( &GridSettings::m_Units, TXT( "Units" ) );
                comp.AddField( &GridSettings::m_Width, TXT( "Width" ) );
                comp.AddField( &GridSettings::m_Length, TXT( "Length" ) );
                comp.AddField( &GridSettings::m_MajorStep, TXT( "Major Step" ) );
                comp.AddField( &GridSettings::m_MinorStep, TXT( "Minor Step" ) );
                comp.AddField( &GridSettings::m_AxisColor, TXT( "Axis Color" ) );
                comp.AddField( &GridSettings::m_MajorColor, TXT( "Major Color" ) );
                comp.AddField( &GridSettings::m_MinorColor, TXT( "Minor Color" ) );
            }
        };

        typedef Helium::SmartPtr<GridSettings> GridSettingsPtr;
    }
}