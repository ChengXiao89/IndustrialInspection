#include "plugin_manager.h"
#include <QDir>

plugin_manager::plugin_manager(QObject* parent)
    : QObject(parent)
{
    log_message("PluginManager initialized");
}

plugin_manager::~plugin_manager()
{
    unload_plugins();
    log_message("PluginManager destroyed");
}

void plugin_manager::load_plugins(const QString& pluginDir)
{
    m_plugin_directory = pluginDir;
    QDir dir(pluginDir);
    if (!dir.exists()) {
        log_message(QString("Plugin directory does not exist: %1").arg(pluginDir));
        return;
    }
    log_message(QString("Loading plugins from directory: %1").arg(pluginDir));

    // 设置过滤器，只加载动态库文件
    QStringList filters;
#ifdef Q_OS_WIN
    filters << "*.dll";
#elif defined(Q_OS_MAC)
    filters << "*.dylib";
#else
    filters << "*.so";
#endif
    dir.setNameFilters(filters);
    QFileInfoList fileList = dir.entryInfoList(QDir::Files);
    int loadedCount = 0;
    for (const QFileInfo& fileInfo : fileList) 
    {
        if (is_valid_plugin_file(fileInfo.absoluteFilePath())) 
        {
            load_plugin(fileInfo.absoluteFilePath());
            loadedCount++;
        }
    }
    log_message(QString("Loaded %1 plugins from %2 files").arg(m_plugins.size()).arg(loadedCount));
    emit post_all_plugins_loaded();
}

void plugin_manager::load_plugin(const QString& filePath)
{
    if (!is_valid_plugin_file(filePath))
    {
        log_message(QString("Invalid plugin file: %1").arg(filePath));
        return;
    }
    // 检查是否已经加载
    for (const PluginInfo& info : m_plugins) 
    {
        if (info.filePath == filePath) {
            log_message(QString("Plugin already loaded: %1").arg(filePath));
            return;
        }
    }
    PluginInfo pluginInfo;
    pluginInfo.loader = new QPluginLoader(filePath, this);
    pluginInfo.filePath = filePath;
    if (pluginInfo.loader->load()) 
    {
        pluginInfo.plugin_object = pluginInfo.loader->instance();
        if (pluginInfo.plugin_object)
        {
            pluginInfo.interface = qobject_cast<interface_plugin*>(pluginInfo.plugin_object);
            if (pluginInfo.interface) 
            {
                pluginInfo.name = pluginInfo.interface->name();
                // 检查名称冲突
                if (find_plugin(pluginInfo.name))
                {
                    log_message(QString("Plugin name conflict: %1").arg(pluginInfo.name));
                    delete pluginInfo.loader;
                    return;
                }
                // 连接信号
                connect(pluginInfo.plugin_object, &QObject::destroyed,this, &plugin_manager::on_plugin_destroyed);
                m_plugins.append(pluginInfo);
                log_message(QString("Successfully loaded plugin: %1 (v%2)")
                    .arg(pluginInfo.name).arg(pluginInfo.interface->version()));
                emit post_plugin_loaded(pluginInfo.name);
            }
            else 
            {
                log_message(QString("Plugin does not implement IPluginInterface: %1").arg(filePath));
                pluginInfo.loader->unload();
                delete pluginInfo.loader;
                emit post_plugin_load_failed(filePath, "Plugin does not implement IPluginInterface");
            }
        }
        else 
        {
            log_message(QString("Failed to get plugin instance: %1").arg(filePath));
            pluginInfo.loader->unload();
            delete pluginInfo.loader;
            emit post_plugin_load_failed(filePath, "Failed to get plugin instance");
        }
    }
    else 
    {
        QString error = pluginInfo.loader->errorString();
        log_message(QString("Failed to load plugin: %1 - %2").arg(filePath).arg(error));
        delete pluginInfo.loader;
        emit post_plugin_load_failed(filePath, error);
    }
}

void plugin_manager::unload_plugins()
{
    log_message("Unloading all plugins");
    // 先反初始化所有插件
    uninitialize_plugins();

    // 卸载所有插件
    for (PluginInfo& info : m_plugins) 
    {
        cleanup_plugin(info);
    }

    m_plugins.clear();
    log_message("All plugins unloaded");
}

void plugin_manager::unload_plugin(const QString& name)
{
    PluginInfo* info = find_plugin(name);
    if (info) 
    {
        log_message(QString("Unloading plugin: %1").arg(name));

        // 反初始化插件
        if (info->initialized && info->interface) 
        {
            info->interface->uninitialize();
            info->initialized = false;
        }

        cleanup_plugin(*info);

        // 从列表中移除
        for (int i = 0; i < m_plugins.size(); ++i) 
        {
            if (m_plugins[i].name == name) {
                m_plugins.removeAt(i);
                break;
            }
        }

        emit post_plugin_unloaded(name);
    }
    else 
    {
        log_message(QString("Plugin not found for unloading: %1").arg(name));
    }
}

QList<interface_plugin*> plugin_manager::get_plugins() const
{
    QList<interface_plugin*> plugins;
    for (const PluginInfo& info : m_plugins) 
    {
        if (info.interface && info.enabled) 
        {
            plugins.append(info.interface);
        }
    }
    return plugins;
}

interface_plugin* plugin_manager::get_plugin(const QString& name) const
{
    const PluginInfo* info = find_plugin(name);
    return (info && info->enabled) ? info->interface : nullptr;
}

QStringList plugin_manager::get_plugin_names() const
{
    QStringList names;
    for (const PluginInfo& info : m_plugins) 
    {
        names.append(info.name);
    }
    return names;
}

int plugin_manager::get_plugin_count() const
{
    return m_plugins.size();
}

bool plugin_manager::is_plugin_loaded(const QString& name) const
{
    return find_plugin(name) != nullptr;
}

QString plugin_manager::get_plugin_version(const QString& name) const
{
    const PluginInfo* info = find_plugin(name);
    return info ? info->interface->version() : QString();
}

void plugin_manager::enable_plugin(const QString& name, bool enabled)
{
    PluginInfo* info = find_plugin(name);
    if (info) 
    {
        if (info->enabled != enabled) 
        {
            info->enabled = enabled;

            if (enabled) 
            {
                if (info->interface && !info->initialized) 
                {
                    info->initialized = info->interface->initialize();
                }
                log_message(QString("Plugin enabled: %1").arg(name));
            }
            else 
            {
                if (info->interface && info->initialized) 
                {
                    info->interface->uninitialize();
                    info->initialized = false;
                }
                log_message(QString("Plugin disabled: %1").arg(name));
            }
        }
    }
}

void plugin_manager::disable_plugin(const QString& name)
{
    enable_plugin(name, false);
}

void plugin_manager::initialize_plugins()
{
    log_message("Initializing all plugins");

    for (PluginInfo& info : m_plugins) 
    {
        if (info.interface && info.enabled && !info.initialized) 
        {
            try 
            {
                info.initialized = info.interface->initialize();
                log_message(QString("Plugin initialized: %1").arg(info.name));
            }
            catch (const std::exception& e) 
            {
                log_message(QString("Failed to initialize plugin %1: %2")
                    .arg(info.name).arg(e.what()));
            }
            catch (...) 
            {
                log_message(QString("Failed to initialize plugin %1: Unknown error").arg(info.name));
            }
        }
    }
}

void plugin_manager::uninitialize_plugins()
{
    log_message("Uninitializing all plugins");

    for (PluginInfo& info : m_plugins) 
    {
        if (info.interface && info.initialized) 
        {
            try 
            {
                info.interface->uninitialize();
                info.initialized = false;
                log_message(QString("Plugin uninitialized: %1").arg(info.name));
            }
            catch (const std::exception& e) 
            {
                log_message(QString("Failed to uninitialize plugin %1: %2").arg(info.name).arg(e.what()));
            }
            catch (...) 
            {
                log_message(QString("Failed to uninitialize plugin %1: Unknown error").arg(info.name));
            }
        }
    }
}

void plugin_manager::on_plugin_destroyed(QObject* obj)
{
    // 当插件对象被销毁时，清理相关信息
    for (int i = 0; i < m_plugins.size(); ++i) 
    {
        if (m_plugins[i].loader && m_plugins[i].plugin_object == obj)
        {
            log_message(QString("Plugin object destroyed: %1").arg(m_plugins[i].name));
            m_plugins[i].plugin_object = nullptr;
        	m_plugins[i].interface = nullptr;
            break;
        }
    }
}

bool plugin_manager::is_valid_plugin_file(const QString& filePath)
{
    QFileInfo fileInfo(filePath);

    // 检查文件是否存在
    if (!fileInfo.exists()) 
    {
        return false;
    }
    // 检查文件扩展名
    QString suffix = fileInfo.suffix().toLower();
#ifdef Q_OS_WIN
    return suffix == "dll";
#elif defined(Q_OS_MAC)
    return suffix == "dylib";
#else
    return suffix == "so";
#endif
}

PluginInfo* plugin_manager::find_plugin(const QString& name)
{
    for (PluginInfo& info : m_plugins) 
    {
        if (info.name == name) 
        {
            return &info;
        }
    }
    return nullptr;
}

const PluginInfo* plugin_manager::find_plugin(const QString& name) const
{
    for (const PluginInfo& info : m_plugins) 
    {
        if (info.name == name) 
        {
            return &info;
        }
    }
    return nullptr;
}

void plugin_manager::log_message(const QString& message)
{
    qDebug() << "[PluginManager]" << message;
}

void plugin_manager::cleanup_plugin(PluginInfo& info)
{
    if (info.loader) 
    {
        if (info.loader->isLoaded()) 
        {
            info.loader->unload();
        }
        delete info.loader;
        info.loader = nullptr;
    }
    info.interface = nullptr;
    info.initialized = false;
}