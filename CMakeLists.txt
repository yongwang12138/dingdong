cmake_minimum_required(VERSION 3.20)
project(DingDong LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 构建产物统一输出到仓库根目录的 bin/（打包前对该目录执行 windeployqt）
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
qt_standard_project_setup()

add_subdirectory(src)

# 动画帧（idle 序列）
# 注意：这里必须写在根目录，且路径相对仓库根——qt_add_resources 按「文件相对当前
# CMakeLists 所在目录」生成别名，别名拼上 PREFIX 后即运行期路径
# :/anim/resources/anim/idle/N.png（见 src/ui/dingdongwidget.cpp）。
# 若移到 src/CMakeLists.txt 下，别名会变成 ../resources/... 导致加载失败。
qt_add_resources(dingdong "anim_res"
    PREFIX "/anim"
    FILES
        resources/anim/idle/1.png
        resources/anim/idle/2.png
        resources/anim/idle/3.png
        resources/anim/idle/4.png
        resources/anim/idle/5.png
        resources/anim/idle/6.png
        resources/anim/idle/7.png
        resources/anim/idle/8.png
        resources/anim/idle/9.png
        resources/anim/idle/10.png
        resources/anim/idle/11.png
        resources/anim/idle/12.png
        resources/anim/idle/13.png
        resources/anim/idle/14.png
        resources/anim/idle/15.png
        resources/anim/idle/16.png
        resources/anim/idle/17.png
        resources/anim/idle/18.png
        resources/anim/idle/19.png
        resources/anim/idle/20.png
        resources/anim/idle/21.png
        resources/anim/idle/22.png
        resources/anim/idle/23.png
        resources/anim/idle/24.png
        resources/anim/idle/25.png
        resources/anim/idle/26.png
        resources/anim/idle/27.png
        resources/anim/idle/28.png
        resources/anim/idle/29.png
        resources/anim/idle/30.png
        resources/anim/idle/31.png
        resources/anim/idle/32.png
        resources/anim/idle/33.png
        resources/anim/idle/34.png
        resources/anim/idle/35.png
        resources/anim/idle/36.png
        resources/anim/idle/37.png
        resources/anim/idle/38.png
        resources/anim/idle/39.png
        resources/anim/idle/40.png
        resources/anim/idle/41.png
)

# 应用图标（托盘 / 窗口标题栏）：编进 exe 的 Qt 资源，运行期从 :/icons/... 读取，
# 不再需要把 app.ico 复制到 exe 旁边（exe 文件图标仍由 resources/win_appicon.rc 负责）。
# 资源别名规则与上方 anim_res 一致：PREFIX + 相对当前 CMakeLists 目录的文件路径。
qt_add_resources(dingdong "appicon_res"
    PREFIX "/icons"
    FILES
        resources/app.ico
)
