#include "menuconfig.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

QVector<MenuConfig> MenuConfigManager::loadFromJson(const QString& filePath) {
    QVector<MenuConfig> menus;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "无法打开菜单配置文件:" << filePath;
        return menus;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        qWarning() << "菜单配置文件格式错误:" << filePath;
        return menus;
    }

    QJsonObject root = doc.object();
    QJsonArray menusArray = root["menus"].toArray();

    for (long long i=0;i<menusArray.size();++i) {
        const QJsonValue& menuValue = menusArray[i];
        QJsonObject menuObj = menuValue.toObject();
        MenuConfig menu;

        menu.title = menuObj["title"].toString();
        menu.name = menuObj["name"].toString();

        QJsonArray itemsArray = menuObj["items"].toArray();
        for (long long j=0;j<itemsArray.size();++j) {
            const QJsonValue& itemValue = itemsArray[j];
            QJsonObject itemObj = itemValue.toObject();

            // 如果是分隔符
            if (itemObj.contains("separator") && itemObj["separator"].toBool()) {
                MenuItemConfig separatorItem;
                separatorItem.separator = true;
                menu.items.append(separatorItem);
                continue;
            }

            MenuItemConfig item;
            item.id = itemObj["id"].toString();
            item.text = itemObj["text"].toString();
            item.iconPath = itemObj["icon"].toString();
            item.shortcut = itemObj["shortcut"].toString();
            item.standardKey = itemObj["standardKey"].toString();
            item.statusTip = itemObj["statusTip"].toString();
            item.slot = itemObj["slot"].toString();

            if (itemObj.contains("enabled"))
                item.enabled = itemObj["enabled"].toBool();
            if (itemObj.contains("checkable"))
                item.checkable = itemObj["checkable"].toBool();

            menu.items.append(item);
        }

        menus.append(menu);
    }

    return menus;
}

QKeySequence::StandardKey MenuConfigManager::stringToStandardKey(const QString& keyStr) {
    static QHash<QString, QKeySequence::StandardKey> mapping = {
        {"New", QKeySequence::New},
        {"Open", QKeySequence::Open},
        {"Save", QKeySequence::Save},
        {"SaveAs", QKeySequence::SaveAs},
        {"Close", QKeySequence::Close},
        {"Quit", QKeySequence::Quit},
        {"Undo", QKeySequence::Undo},
        {"Redo", QKeySequence::Redo},
        {"ZoomIn", QKeySequence::ZoomIn},
        {"ZoomOut", QKeySequence::ZoomOut},
        {"Cut", QKeySequence::Cut},
        {"Copy", QKeySequence::Copy},
        {"Paste", QKeySequence::Paste},
        {"Delete", QKeySequence::Delete},
        {"Find", QKeySequence::Find},
        {"Replace", QKeySequence::Replace},
        {"HelpContents", QKeySequence::HelpContents},
        {"WhatsThis", QKeySequence::WhatsThis}
    };

    return mapping.value(keyStr, QKeySequence::UnknownKey);
}
