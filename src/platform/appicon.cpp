#include "appicon.h"

#include <QIcon>

namespace {

// 图标已编入 exe 的 Qt 资源（见根 CMakeLists 的 qt_add_resources "appicon_res"），
// 运行期直接从资源读取，无需 exe 旁额外文件。
// 路径规则与 anim_res 一致：PREFIX "/icons" + 相对仓库根的文件路径 -> :/icons/resources/app.ico
// 若资源缺失则直接返回空图标，不做字体字形回退。
QIcon loadFromResource()
{
    QIcon icon{QStringLiteral(":/icons/resources/app.ico")};
    if(!icon.isNull() && !icon.availableSizes().isEmpty())
        return icon;
    return {};
}

} // namespace

QIcon AppIcon::make()
{
    return loadFromResource();
}
