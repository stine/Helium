#pragma once

#include "Application/Undo/BatchCommand.h"

namespace Helium
{
    namespace Core
    {
        class HierarchyNode;
        typedef Helium::SmartPtr< Core::HierarchyNode > HierarchyNodePtr;

        class ParentCommand : public Undo::BatchCommand
        {
        private:
            HierarchyNodePtr m_Node;
            HierarchyNodePtr m_NextParent;
            HierarchyNodePtr m_PreviousParent;

        public:
            ParentCommand(const HierarchyNodePtr& child, const HierarchyNodePtr& parent);

            virtual void Undo() HELIUM_OVERRIDE;

            virtual void Redo() HELIUM_OVERRIDE;

            void Swap();
        };
    }
}