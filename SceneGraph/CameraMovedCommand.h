#pragma once

#include "Application/UndoQueue.h"

#include "Math/Matrix4.h"

namespace Helium
{
    namespace SceneGraph
    {
        class Camera;
        class Viewport;

        /// @class CameraMovedCommand
        /// @brief Used to implement a Undo/Redo queue for camera movement in the scene editor
        class CameraMovedCommand : public UndoCommand
        {
        private:
            SceneGraph::Camera*         m_Camera;
            SceneGraph::Viewport*           m_View;

            Matrix4    m_PreviousTransform;
        public:
            CameraMovedCommand(SceneGraph::Viewport* view, SceneGraph::Camera* cam );
            virtual ~CameraMovedCommand();

        public:
            void Undo();
            void Redo();
        };

    } // namespace SceneGraph
}