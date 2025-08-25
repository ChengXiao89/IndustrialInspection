#pragma once

#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#include "boost/asio.hpp"
#include "boost/asio/serial_port.hpp"

class motion_control 
{
public:
    motion_control(const std::string& port_name, unsigned int baud_rate);

    ~motion_control();

    /*************************************************
    * 设置光源亮度
    * frequency     -- 光源频率，单位时间内的周期数（例如：赫兹，Hz）。频率越高，光源的闪烁周期越短。(可以)固定为 1000000
    * duty_cycle    -- 占空比。光源在一个周期内的开启时间比例，通常用百分比表示。比如，80% 的占空比表示光源在周期内的 80% 时间是开启的，剩余 20% 时间是关闭的
    * timeout_s     -- 判断超时。如果发送命令之后 timeout 秒内仍然没有返回消息，表示超时
    *************************************************/
    void set_light_source_param(int frequency, int duty_cycle, int timeout = 1);

    /*************************************************
    * 沿指定轴移动一定距离. 
    * axis      --  指定移动的轴. 0 : X轴  1 : Y轴
    * distance  --  移动的距离大小，脉冲数量
    * speed     --  移动速度
    *************************************************/
    void move_distance(int axis, int distance, int speed);

    /*************************************************
    * 沿指定轴移动到指定位置.
    * axis      --  指定移动的轴. 0 : X轴  1 : Y轴
    * position  --  目的地在轴上的位置
    * speed     --  移动速度
    *************************************************/
    void move_position(int axis, int position, int speed);

    /*************************************************
    * 重置位置，只在 X 轴上进行
    * 首先往前移动一小段距离，然后往后移动到限制位，再将限制位设置为零点
    * 重置之后,X 轴上的坐标范围为[0,N],单位为脉冲。 
    * 之所以需要重置，是因为 X 轴上的零点可以是任意位置，如果将某个位置设为零点，则X轴的范围则成为[-M,N]
    * 重置是为了将X轴上所有点坐标设为正值，便于处理
    *************************************************/
    void reset();

    /*************************************************
    * 向串口发送命令,这里会阻塞当前线程，直到接收到设备返回的完整消息(设备执行完毕)或者超时
    * 如果用户没有设置超时时间，或者设置了无效的超时时间(<=0),超时时间默认为10秒
    * 正常时间内返回 true, 表示设备执行完毕;超时后返回 false, 表示设备可能出现异常
    **************************************************/
    bool send_command(const std::string& cmd, int timeout = 10);

    /*************************************************
    * 读取设备返回的消息，如果消息结尾字符为 finifh_ch 表示读取完毕，返回true
    * 否则表示没有读取完毕，返回 false
    ***********************************************/
    bool read_reply(std::string& reply,char finifh_ch);

    /*************************************************
    * 串口如果由本线程打开，可以关闭
    ***********************************************/
    void close();

    bool is_opened() const { return m_is_opened; }

private:
    boost::asio::io_context m_io;
    boost::asio::serial_port m_serial;
    bool m_is_opened{ false };


};

