#pragma once

#include <vector>
#include <memory>
#include <string>
#include <algorithm>

#include "Core/Layer.h"


class LayerStack
{
public:
    LayerStack() : m_LayerInsertIndex(0) {}

    ~LayerStack()
    {
        for (Layer* layer : m_Layers)
        {
            if (layer)
            {
                layer->OnDetach();
                delete layer;
            }
        }
        m_Layers.clear();
    }

    void PushLayer(Layer* layer)
    {
        m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, layer);
        m_LayerInsertIndex++;
        layer->OnAttach();
    }

    void PushOverlay(Layer* overlay)
    {
        m_Layers.emplace_back(overlay);
        overlay->OnAttach();
    }

    // Removes a regular layer from the stack and deletes the pointer.
    void PopLayer(Layer* layer)
    {
        // Search only in the regular layer section (from begin up to m_LayerInsertIndex)
        auto it = std::find(m_Layers.begin(), m_Layers.begin() + m_LayerInsertIndex, layer);
        if (it != m_Layers.begin() + m_LayerInsertIndex)
        {
            layer->OnDetach();
            m_Layers.erase(it);
            m_LayerInsertIndex--; // Adjust the insertion point down
            delete layer;
        }
    }

    // Removes an overlay from the stack and deletes the pointer.
    void PopOverlay(Layer* overlay)
    {
        // Search in the overlay section (from m_LayerInsertIndex to end)
        auto it = std::find(m_Layers.begin() + m_LayerInsertIndex, m_Layers.end(), overlay);
        if (it != m_Layers.end())
        {
            (*it)->OnDetach(); // FIX: Using the parameter 'overlay' to call OnDetach
            m_Layers.erase(it);
            // NOTE: The m_LayerInsertIndex remains unchanged as overlays are popped
            delete (*it);
        }
    }

    // --- Standard Iterator Access ---

    std::vector<Layer*>::iterator begin() { return m_Layers.begin(); }
    std::vector<Layer*>::iterator end() { return m_Layers.end(); }

    std::vector<Layer*>::const_iterator begin() const { return m_Layers.begin(); }
    std::vector<Layer*>::const_iterator end() const { return m_Layers.end(); }

    // Reverse iterators are often used for event propagation (top layer handles first)
    std::vector<Layer*>::reverse_iterator rbegin() { return m_Layers.rbegin(); }
    std::vector<Layer*>::reverse_iterator rend() { return m_Layers.rend(); }

private:
    std::vector<Layer*> m_Layers;
    unsigned int m_LayerInsertIndex; // Index separating Layers (0 to index-1) from Overlays (index to end)
};