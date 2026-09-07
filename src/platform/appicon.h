#ifndef APPICON_H
#define APPICON_H

#include <QIcon>

// 应用图标：从 Qt 资源 :/icons/resources/app.ico 读取（见根 CMakeLists 的
// qt_add_resources "appicon_res"）；资源缺失时直接为空，不做字体字形回退。
class AppIcon
{
public:
    static QIcon make();
};

#endif // APPICON_H
