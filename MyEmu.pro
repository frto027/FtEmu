TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    ScreenReshape.c \
    TestDisplay.c \
    testjoystickdisplay.c \
    bmpfont.c \
    main.c \
    BlockDisplay.c \
    CpuEmulate.c \
    CpuInstruction.c \
    PpuEmulate.c \
    NesParser.c

#LIBS += -L$$PWD/lib-mingw-w64/ -lglfw3dll
#LIBS += -lopengl32
#INCLUDEPATH += $$PWD/GLFW
#DEPENDPATH += $$PWD/GLFW

LIBS += -lGL
LIBS += -lGLU
LIBS += -lglut
LIBS += -lX11 -lXxf86vm -lXrandr -lpthread -lXi -ldl -lXinerama -lXcursor
LIBS += -lglfw3

HEADERS += \
    GlobalDefine.h \
    ScreenReshape.h \
    TestDisplay.h \
    TestJoystickDisplay.h \
    bmpfont.h \
    BlockDisplay.h \
    CpuEmulate.h \
    CpuInstruction.h \
    NesParser.h \
    PpuEmulate.h

DISTFILES +=
