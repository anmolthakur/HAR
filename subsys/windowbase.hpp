#ifndef __WINDOW_HPP__
#define __WINDOW_HPP__

#include <type_traits>
#include "imgui/imgui.h"

namespace window
{
    enum class Layer
    {
        BG,
        GUI
    };
    
    using DrawCallback = std::function<void(Layer)>; //std::add_pointer<void (Layer)>::type;
}

#endif //__WINDOW_HPP__

