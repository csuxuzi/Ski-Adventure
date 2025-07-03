cmake_minimum_required(VERSION 3.16)
project(SkiAdventure VERSION 1.0 LANGUAGES CXX)

# --- CMake 和 Qt 的基本设置 ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 启用Qt的MOC, RCC, UIC工具
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# --- 寻找 Qt 6 的库 ---
find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia)


# --- 定义可执行文件目标 ---
# 1. 创建目标，这里只放一个最核心的文件，比如 main.cpp
add_executable(SkiAdventure
    src/main.cpp
    src/ui/ImageButton.h src/ui/ImageButton.cpp
    src/ui/placeholderdialog.h src/ui/placeholderdialog.cpp
    src/ui/GameScreen.h src/ui/GameScreen.cpp
    src/game/GameObject.h src/game/GameObject.cpp

    src/game/player.h src/game/player.cpp
    src/game/obstacle.h src/game/obstacle.cpp
    src/game/stone.h src/game/stone.cpp
    src/game/house.h src/game/house.cpp
    src/game/seesaw.h src/game/seesaw.cpp
    src/game/signboard.h src/game/signboard.cpp
    src/ui/pausedialog.h src/ui/pausedialog.cpp
    src/game/avalanche.h src/game/avalanche.cpp
    src/ui/gameoverdialog.h src/ui/gameoverdialog.cpp
    src/game/mount.h src/game/mount.cpp
    src/game/penguin.h src/game/penguin.cpp
    src/game/yeti.h src/game/yeti.cpp
    src/game/coin.h src/game/coin.cpp
    src/fx/EffectManager.h src/fx/EffectManager.cpp
    src/game/cardeffects.h
    src/ui/cardwidget.h src/ui/cardwidget.cpp
    src/ui/cardselectiondialog.h src/ui/cardselectiondialog.cpp
    src/ui/helpdialog.h src/ui/helpdialog.cpp
    src/game/tree.h src/game/tree.cpp
    src/ui/imagedisplaydialog.h src/ui/imagedisplaydialog.cpp
    src/game/carddatabase.h src/game/carddatabase.cpp
    src/ui/debuginfodialog.h src/ui/debuginfodialog.cpp
)


# --- 为已创建的目标添加源文件 ---
# 2. 现在目标已经存在，我们可以安全地为它添加所有其他的源文件
#    注意：头文件(.h)通常不需要在这里列出，CMake会自动处理依赖
target_sources(SkiAdventure PRIVATE
    # 头文件 (.h)
    src/ui/MainWindow.h
    src/ui/MainMenuScreen.h
    src/ui/SettingsDialog.h
    src/audio/AudioManager.h
    src/game/GameObject.h
    src/ui/GameScreen.h
    src/game/Player.h
    src/ui/ImageButton.h           # <-- 【新增行】
    src/ui/PlaceholderDialog.h     # <-- 【新增行】
    src/game/stone.h
    src/game/house.h
    src/game/seesaw.h
    src/game/avalanche.h
    src/ui/gameoverdialog.h
    src/fx/EffectManager.h
    src/game/penguin.h
    src/game/yeti.h
    src/game/coin.h
    # 源文件 (.cpp)
    src/ui/MainWindow.cpp
    src/ui/MainMenuScreen.cpp
    src/ui/SettingsDialog.cpp
    src/audio/AudioManager.cpp
    src/game/GameObject.cpp
    src/ui/GameScreen.cpp
    src/game/stone.cpp
    src/game/Player.cpp
    src/game/house.cpp
    src/game/penguin.cpp
    src/game/coin.cpp
    src/game/yeti.cpp
    src/ui/ImageButton.cpp         # <-- 【新增行】
    src/ui/PlaceholderDialog.cpp   # <-- 【新增行】
    src/game/seesaw.cpp
    src/game/avalanche.cpp
    src/ui/gameoverdialog.cpp
    src/fx/EffectManager.cpp

    # 资源文件 (.qrc)
    resources.qrc
    # 如果您有其他资源文件，也可以在这里继续添加
)


# --- 为目标设置头文件搜索路径 ---
# 3. 告诉编译器去哪里寻找 #include "..." 的文件
target_include_directories(SkiAdventure PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)


# --- 为目标链接所需的库 ---
# 4. 将Qt的模块链接到我们的程序中
target_link_libraries(SkiAdventure PRIVATE
    Qt6::Widgets
    Qt6::Multimedia
)
