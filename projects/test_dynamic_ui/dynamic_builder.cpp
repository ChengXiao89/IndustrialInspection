#include "dynamic_builder.h"
#include <QDomElement>
#include <QFile>
#include <QGroupBox>
#include <QFormLayout>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QLineEdit>
#include <QDebug>

DynamicFormBuilder::DynamicFormBuilder(QWidget* parent)
    : QWidget(parent)
{
    mainLayout = new QVBoxLayout(this);
}

bool DynamicFormBuilder::loadFromXmlFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return false;

    QDomDocument doc;
    doc.setContent(&file);
    file.close();

    QDomElement root = doc.documentElement();
    QDomNodeList groups = root.elementsByTagName("GroupBox");

    for (int i = 0; i < groups.count(); ++i) {
        QDomElement groupElem = groups.at(i).toElement();
        QString title = groupElem.attribute("title");
        QGroupBox* groupBox = new QGroupBox(title);
        QFormLayout* formLayout = new QFormLayout(groupBox);

        QDomNodeList params = groupElem.elementsByTagName("Parameter");
        for (int j = 0; j < params.count(); ++j) {
            QDomElement paramElem = params.at(j).toElement();
            QWidget* w = createParameterWidget(paramElem);
            QString label = paramElem.firstChildElement("Info").attribute("label");
            formLayout->addRow(label, w);
        }

        mainLayout->addWidget(groupBox);
    }

    return true;
}

QWidget* DynamicFormBuilder::createParameterWidget(const QDomElement& paramElem)
{
    QString type = paramElem.attribute("type");
    QDomElement info = paramElem.firstChildElement("Info");
    QString key = info.attribute("key");

    QWidget* widget = nullptr;

    if (type == "ComboBox") {
        QComboBox* box = new QComboBox();
        QDomElement options = paramElem.firstChildElement("Options");
        QString def = options.attribute("default");

        QDomNodeList items = options.elementsByTagName("Item");
        int defaultIndex = 0;
        for (int i = 0; i < items.count(); ++i) {
            QDomElement item = items.at(i).toElement();
            QString value = item.attribute("value");
            QString label = item.attribute("label");
            box->addItem(label, value);
            if (value == def) defaultIndex = i;
        }
        box->setCurrentIndex(defaultIndex);
        widget = box;
        controlsMap[key] = { "ComboBox", box, key };

        // 绑定响应
        QDomElement bindElem = paramElem.firstChildElement("Binding");
        if (!bindElem.isNull()) {
            applyBinding(bindElem, key);
        }
    }
    else if (type == "SpinBox") {
        QSpinBox* spin = new QSpinBox();
        spin->setMinimum(info.attribute("min").toInt());
        spin->setMaximum(info.attribute("max").toInt());
        spin->setSingleStep(info.attribute("step").toInt());
        spin->setValue(info.attribute("value").toInt());
        widget = spin;
        controlsMap[key] = { "SpinBox", spin, key };
    }
    else if (type == "DoubleSpinBox") {
        QDoubleSpinBox* spin = new QDoubleSpinBox();
        spin->setMinimum(info.attribute("min").toDouble());
        spin->setMaximum(info.attribute("max").toDouble());
        spin->setSingleStep(info.attribute("step").toDouble());
        spin->setValue(info.attribute("value").toDouble());
        widget = spin;
        controlsMap[key] = { "DoubleSpinBox", spin, key };
    }

    return widget;
}

void DynamicFormBuilder::applyBinding(const QDomElement& bindingElem, const QString& key)
{
    QComboBox* combo = qobject_cast<QComboBox*>(controlsMap[key].widget);
    if (!combo) return;

    QDomNodeList onElems = bindingElem.elementsByTagName("On");
    for (int i = 0; i < onElems.count(); ++i) {
        QDomElement onElem = onElems.at(i).toElement();
        QString value = onElem.attribute("value");
        QStringList enableKeys, disableKeys;

        QDomElement enableElem = onElem.firstChildElement("Enable");
        if (!enableElem.isNull()) enableKeys = enableElem.attribute("keys").split(',');

        QDomElement disableElem = onElem.firstChildElement("Disable");
        if (!disableElem.isNull()) disableKeys = disableElem.attribute("keys").split(',');

        QObject::connect(combo, &QComboBox::currentTextChanged, this,
            [=](const QString& val) {
                if (val == value) {
                    enableWidgets(enableKeys, true);
                    enableWidgets(disableKeys, false);
                }
            });
    }
}

void DynamicFormBuilder::enableWidgets(const QStringList& keys, bool enable)
{
    for (const QString& key : keys) {
        if (controlsMap.contains(key)) {
            controlsMap[key].widget->setEnabled(enable);
        }
    }
}

QVariantMap DynamicFormBuilder::getValues() const
{
    QVariantMap map;
    for (const auto& [key, info] : controlsMap.toStdMap()) {
        if (info.type == "SpinBox") {
            map[key] = qobject_cast<QSpinBox*>(info.widget)->value();
        }
        else if (info.type == "DoubleSpinBox") {
            map[key] = qobject_cast<QDoubleSpinBox*>(info.widget)->value();
        }
        else if (info.type == "ComboBox") {
            map[key] = qobject_cast<QComboBox*>(info.widget)->currentData().toString();
        }
    }
    return map;
}