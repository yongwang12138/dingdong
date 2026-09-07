qt_add_executable(dingdong
    main.cpp

    core/config.h
    core/config.cpp
    core/reminder.h
    core/reminder.cpp

    ui/dingdongwidget.h
    ui/dingdongwidget.cpp
    ui/settingdialog.h
    ui/settingdialog.cpp

    platform/appinit.h
    platform/appinit.cpp
    platform/appicon.h
    platform/appicon.cpp
    platform/logger.h
    platform/logger.cpp

    ${CMAKE_SOURCE_DIR}/resources/win_appicon.rc
)

# 以 src/ 作为引用根：#include "core/config.h" / "ui/settingdialog.h" / "platform/logger.h"
target_include_directories(dingdong PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(dingdong PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
