// menu_config.h
#ifndef MENUCONFIG_H
#define MENUCONFIG_H

#include <QString>
#include <QVector>
#include <QIcon>
#include <QKeySequence>

// 菜单项配置
struct MenuItemConfig {
    QString id;                    // 唯一标识符
    QString text;                  // 显示文本
    QString iconPath;              // 图标路径
    QString shortcut;              // 快捷键字符串
    QString standardKey;           // 标准快捷键名称
    QString statusTip;             // 状态栏提示
    QString slot;                  // 槽函数名称
    bool enabled = true;           // 是否启用
    bool checkable = false;        // 是否可勾选
    bool separator = false;        // 是否为分隔符
};

// 菜单配置
struct MenuConfig {
    QString title;                 // 菜单标题
    QString name;                  // 菜单名称
    QVector<MenuItemConfig> items; // 菜单项列表
};

// 菜单配置管理器
class MenuConfigManager {
public:
    static QVector<MenuConfig> loadFromJson(const QString& filePath);
    static void saveToJson(const QString& filePath, const QVector<MenuConfig>& configs);
    static QKeySequence::StandardKey stringToStandardKey(const QString& keyStr);
    static QString standardKeyToString(QKeySequence::StandardKey key);
};

#endif // MENUCONFIG_H
