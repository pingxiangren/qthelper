# ----------------------------------------------------
# qthelper.pri - QtHelper 模块定义文件
# ----------------------------------------------------

# 1. 定义模块的根目录
# $$PWD 表示当前 .pri 文件所在的目录
QTHELPER_BASE_DIR = $$PWD

# 2. 添加模块所需的 Qt 组件
QT += core gui 
# 根据 LogHelper 的实际依赖添加或删除

# 3. 添加头文件搜索路径
# 允许调用方使用 #include "LogHelper/loghelper.h"
INCLUDEPATH += $$QTHELPER_BASE_DIR 

# 4. 添加头文件
# 确保所有公共头文件都被正确识别
HEADERS += \
    $$QTHELPER_BASE_DIR/LogHelper/loghelper.h

# 5. 添加源文件
SOURCES += \
    $$QTHELPER_BASE_DIR/LogHelper/loghelper.cc

# 6. 配置构建模式
# 确保文件被正确编译
CONFIG(release, debug|release):QMAKE_CXXFLAGS += -O2
# 其他 QMAKE 变量...