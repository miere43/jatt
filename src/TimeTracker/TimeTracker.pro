#-------------------------------------------------
#
# Project created by QtCreator 2017-02-20T22:54:28
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TimeTracker
TEMPLATE = app
# CONFIG -= exceptions
CONFIG += c++14

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS APP_BREAK_ON_ERROR

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

RC_ICONS = appicon.ico

msvc: LIBS += -luser32

SOURCES += main.cpp\
        mainwindow.cpp \
    application_state.cpp \
    core_types.cpp \
    block_allocator.cpp \
    database_manager.cpp \
    right_click_push_button.cpp \
    utilities.cpp \
    add_activity_dialog.cpp \
    add_field_dialog.cpp \
    activity_list_model.cpp \
    activity_recorder.cpp \
    activity_visualizer.cpp \
    add_activity_info_dialog.cpp \
    activity_item_delegate.cpp \
    edit_activity_field_dialog.cpp \
    hotkey.cpp \
    error_macros.cpp \
    statistics_dialog.cpp \
    activity_browser.cpp \
    activity_table_model.cpp \
    search_query.cpp

HEADERS  += mainwindow.h \
    application_state.h \
    core_types.h \
    block_allocator.h \
    database_manager.h \
    right_click_push_button.h \
    utilities.h \
    add_activity_dialog.h \
    add_field_dialog.h \
    activity_list_model.h \
    activity_recorder.h \
    activity_visualizer.h \
    add_activity_info_dialog.h \
    activity_item_delegate.h \
    edit_activity_field_dialog.h \
    hotkey.h \
    error_macros.h \
    statistics_dialog.h \
    activity_browser.h \
    activity_table_model.h \
    search_query.h

FORMS    += mainwindow.ui \
    add_activity_dialog.ui \
    add_field_dialog.ui \
    add_activity_info_dialog.ui \
    edit_activity_field_dialog.ui \
    statistics_dialog.ui \
    activity_browser.ui

RESOURCES += \
    resource.qrc
