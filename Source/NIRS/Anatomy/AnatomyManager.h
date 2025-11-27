#pragma once
#include "NIRS/Anatomy/Head.h"
#include "NIRS/Anatomy/Cortex.h"

namespace NIRS {
    class AnatomyManager {
    public:
        static AnatomyManager& Instance() {
            static AnatomyManager instance;
            return instance;
        }

        void LoadHead(std::string& path);
        void LoadCortex(std::string& path);

        Head* GetHead() { return m_Head.get(); }
        Cortex* GetCortex() { return m_Cortex.get(); }

        bool HasHead() const { return m_Head != nullptr; }
        bool HasCortex() const { return m_Cortex != nullptr; }

    private:
        AnatomyManager() = default;
        Scope<Head> m_Head;
        Scope<Cortex> m_Cortex;
    };
}