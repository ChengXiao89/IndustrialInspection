/***************************************************
 * 公共接口类，包含一些常用的功能，例如共享内存、数据转换等
 ***************************************************/

#pragma once

#include "common_global.h"

struct COMMON_EXPORT st_position
{
	int m_x{ 0 }, m_y{ 0 };
	st_position(int x = 0, int y = 0) :m_x(x), m_y(y) {}

    //支持按照 Y 升序排序
    bool operator<(const st_position& other) const
    {
        return m_y < other.m_y;
    }
};


class COMMON_EXPORT common
{
public:
    common();
};
