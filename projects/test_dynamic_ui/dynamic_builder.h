#pragma once
#include <QWidget>
#include <QDomDocument>
#include <QMap>
#include <QVariant>
#include <QVBoxLayout>

class DynamicFormBuilder : public QWidget
{
    Q_OBJECT
public:
    explicit DynamicFormBuilder(QWidget* parent = nullptr);
    bool loadFromXmlFile(const QString& filePath);
    QVariantMap getValues() const;

private:
    struct ControlInfo {
        QString type;
        QWidget* widget;
        QString key;
    };

    QMap<QString, ControlInfo> controlsMap;
    QVBoxLayout* mainLayout = nullptr;

    QWidget* createParameterWidget(const QDomElement& paramElem);
    void applyBinding(const QDomElement& bindingElem, const QString& key);
    void enableWidgets(const QStringList& keys, bool enable);
};