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
    NesParser.c \
    GlobalClocks.c

#LIBS += -L$$PWD/lib-mingw-w64/ -lglfw3dll
#LIBS += -lopengl32
#INCLUDEPATH += $$PWD/GLFW
#DEPENDPATH += $$PWD/GLFW

unix:LIBS += -lGL -lGLU -lglut -lX11 -lXxf86vm -lXrandr -lpthread -lXi -ldl -lXinerama -lXcursor -lglfw3

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
    PpuEmulate.h \
    GlobalClocks.h

DISTFILES +=

win32: LIBS += -lopengl32
win32: LIBS += -L$$PWD/../../thing/openGL/glfw-3.2.1.bin.WIN32/glfw-3.2.1.bin.WIN32/lib-mingw/ -lglfw3dll

INCLUDEPATH += $$PWD/../../thing/openGL/glfw-3.2.1.bin.WIN32/glfw-3.2.1.bin.WIN32/lib-mingw
DEPENDPATH += $$PWD/../../thing/openGL/glfw-3.2.1.bin.WIN32/glfw-3.2.1.bin.WIN32/lib-mingw

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../../thing/openGL/glfw-3.2.1.bin.WIN32/glfw-3.2.1.bin.WIN32/lib-mingw/glfw3dll.lib
else:win32-g++: PRE_TARGETDEPS += $$PWD/../../thing/openGL/glfw-3.2.1.bin.WIN32/glfw-3.2.1.bin.WIN32/lib-mingw/libglfw3dll.a
