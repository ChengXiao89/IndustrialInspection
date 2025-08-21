#pragma once
#include <QtPlugin>
#include <QString>
#include <QObject>
#include <QWidget>

#include "signal_bridge.h"

/**
 * @brief 插件接口基类
 *
 * 所有插件都需要实现此接口，提供统一的插件管理能力
 */
class PLUGININTERFACE_EXPORT interface_plugin
{
public:
    interface_plugin() = default;
    virtual ~interface_plugin() = default;
    /***************插件元信息**************/
    virtual QString name() const = 0;                        //获取插件名称
    virtual QString version() const = 0;                     //获取插件版本
    virtual QString description() const = 0;                 //获取描述信息
	virtual QString author() const = 0;                      //获取插件作者信息

    /***************生命周期管理**************/
	virtual bool initialize() = 0;                              //初始化插件,在插件加载后调用,用于初始化插件资源和状态
	virtual void uninitialize() = 0;                            //反初始化插件,在插件卸载前调用,用于清理插件资源和状态,插件应该在此函数中释放所有占用的资源
    virtual bool is_initialized() const = 0;                     //检查插件是否已初始化

    /***************启用/禁用**************/
	virtual bool enable() = 0;                                  //启用插件功能，只有在插件已初始化的情况下才能启用
    virtual void disable() = 0;                                 //禁用插件功能，但不会反初始化插件
    virtual bool is_enabled() const = 0;                         //检查插件是否已启用

    /***************配置和错误处理**************/
    virtual QString get_configuration() const = 0;               //获取插件配置信息
    virtual bool set_configuration(const QString& config) = 0;   //设置插件配置信息
    virtual QString get_last_error() const = 0;                   //获取插件的最后错误信息

    /***************主功能入口**************/
    virtual bool execute() = 0;                                 //执行插件主要功能,只有在插件已启用的情况下才应该调用.true 执行成功，false 执行失败

    /*****************注册桥接对象****************/
    virtual void set_signal_bridge(signal_bridge* bridge) = 0;
};

// 定义插件接口的IID，用于Qt插件系统
#define interface_plugin_iid "com.mycompany.interface_plugin"
Q_DECLARE_INTERFACE(interface_plugin, interface_plugin_iid)

//void registerPluginInterfaces();