#include "trajectory.hpp"
#include "subsys.hpp"

void History::StoreValue (const XnPoint3D &pt_world, const XnPoint3D &pt_screen)
{
    if (--m_curr_pos < 0) m_curr_pos = m_max_size - 1;
    m_records [m_curr_pos].value_world = pt_world;
    m_records [m_curr_pos].value_screen = pt_screen;
    m_records [m_curr_pos].time = window::getTime();
    
    if (++m_size > m_max_size) m_size = m_max_size;
}


