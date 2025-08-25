#include "motion_control.h"

motion_control::motion_control(const std::string& port_name, unsigned int baud_rate)
    : m_io(), m_serial(m_io, port_name)
{
    try
    {
        m_serial.close();           // 先关闭才能正常打开...
        m_serial.open(port_name);   // 打开串口
        m_serial.set_option(boost::asio::serial_port_base::baud_rate(baud_rate));
        m_serial.set_option(boost::asio::serial_port_base::character_size(8));
        m_serial.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
        m_serial.set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));
        m_serial.set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none));
        // 开机亮光
        set_light_source_param(1000000, 80);
        m_is_opened = true;
    }
    catch (const boost::system::system_error& e)
    {
        std::cerr << "串口打开失败: " << e.what() << std::endl;
    }
}

motion_control::~motion_control()
{
    //close();
}

void motion_control::set_light_source_param(int frequency, int duty_cycle, int timeout) 
{
    std::string cmd = "SetLight " + std::to_string(frequency) + " " + std::to_string(duty_cycle);
    send_command(cmd, timeout);
}

void motion_control::move_distance(int axis, int step, int speed) 
{
    std::string cmd = "MoveDistance " + std::to_string(axis) + " " + std::to_string(step) + " " + std::to_string(speed);
    send_command(cmd);
}

void motion_control::move_position(int axis, int step, int speed)
{
    std::string cmd = "MovePosition " + std::to_string(axis) + " " + std::to_string(step) + " " + std::to_string(speed);
    send_command(cmd);
}

void motion_control::reset()
{
    move_distance(0, 1000, 5000);       //先向前移动一小段距离
    move_distance(0, -30000, 5000);     //再向后移动到限制位
    send_command("SetZero");            //将限制位设置为零点
}

void motion_control::close() 
{
    try
    {
        if (m_serial.is_open())
        {
            m_serial.close();
        }
        // 检查是否成功关闭串口
        if (!m_serial.is_open())
        {
            m_is_opened = false;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "关闭串口时发生异常: " << e.what() << std::endl;
    }
}

bool motion_control::send_command(const std::string& cmd, int timeout)
{
    if (timeout <= 0)
    {
        timeout = 10;
    }
    // 发送命令
    boost::asio::write(m_serial, boost::asio::buffer(cmd));
    auto start = std::chrono::steady_clock::now();      //当前时间
    std::string reply("");                              //设备返回的消息
    bool finish(false);                                 //通过设备返回的消息判断是否执行完毕
    while (true)
    {
        // 尝试读取设备返回的消息
        finish = read_reply(reply,'\n');
        if (finish)
        {
            return true;
        }
        // 检查是否超时
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - start).count() > timeout)
        {
            return false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

bool motion_control::read_reply(std::string& reply, char finish_ch)
{
    using namespace boost::asio;
    char sz_buf[256] = { 0 };
    boost::system::error_code error_code;
    size_t n = m_serial.read_some(buffer(sz_buf), error_code);
    if (error_code)     //发生了错误
    {
        reply = "error";
        return true;
    }
    if (n > 0)
    {
        reply.append(sz_buf, n);
        if (reply.back() == finish_ch)
        {
            return true;
        }
    } 
    return false;
}