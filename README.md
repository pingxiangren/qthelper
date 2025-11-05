# LogHelper
enable `qInfo qDebug qWarning etc.` sync into `.log` file

## how to use
1. add `LogHelper/loghelper.h` in the `main.cc`
2. add `qInstallMessageHandler(customMessageHandler);` behind to the line `QApplication a(argc, argv);` which in `main` function

# QssHelper
load `qss` file

# How To Use
1. cd your project path
2. use `git submodule add https://github.com/pingxiangren/qthelper.git libs/qthelper`
3. just add one line `include(RELATIVE_PATH/qthelper.pri)` in the `.pro` file, such as `include(libs/qthelper/qthelper.pri)`
