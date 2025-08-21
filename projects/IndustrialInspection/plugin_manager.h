#pragma once

#include <QObject>
#include <QList>
#include <QPluginLoader>
#include "../PluginInterface/interface.h"

//插件信息结构体
struct PluginInfo {
	QPluginLoader* loader;          // 插件加载器
    QObject* plugin_object;         // 加载插件之后得到的QObject对象指针
	interface_plugin* interface;    // 插件接口指针，从 plugin_object 转换而来
	QString name;                   // 插件名称
	QString filePath;               // 插件文件路径
	bool enabled;                   // 插件是否启用
	bool initialized;               // 插件是否已初始化

    PluginInfo() : loader(nullptr), plugin_object(nullptr), interface(nullptr), enabled(true), initialized(false) {}
};

class plugin_manager : public QObject
{
    Q_OBJECT

public:
    explicit plugin_manager(QObject* parent = nullptr);
    ~plugin_manager();

    // 加载插件
    void load_plugins(const QString& pluginDir);
    void load_plugin(const QString& filePath);

    // 卸载插件
    void unload_plugins();
    void unload_plugin(const QString& name);

    // 插件管理
    QList<interface_plugin*> get_plugins() const;
    interface_plugin* get_plugin(const QString& name) const;
    QStringList get_plugin_names() const;

    // 插件信息
    int get_plugin_count() const;
    bool is_plugin_loaded(const QString& name) const;
    QString get_plugin_version(const QString& name) const;

    // 插件控制
    void enable_plugin(const QString& name, bool enabled = true);
    void disable_plugin(const QString& name);
	void initialize_plugins();              //初始化所有插件，需要在插件加载后调用       
    void uninitialize_plugins();

signals:
    void post_plugin_loaded(const QString& name);
    void post_plugin_unloaded(const QString& name);
    void post_plugin_load_failed(const QString& filePath, const QString& error);
    void post_all_plugins_loaded();

private slots:
    void on_plugin_destroyed(QObject* obj);

private:
	QList<PluginInfo> m_plugins;         //所有的插件信息
    QString m_plugin_directory;          //插件所在目录

    // 私有辅助方法
    PluginInfo* find_plugin(const QString& name);
    const PluginInfo* find_plugin(const QString& name) const;

    static bool is_valid_plugin_file(const QString& filePath);
    static void log_message(const QString& message);
    static void cleanup_plugin(PluginInfo& info);
};