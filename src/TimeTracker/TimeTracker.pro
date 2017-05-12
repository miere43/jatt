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

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        mainwindow.cpp \
    application_state.cpp \
    core_types.cpp \
    block_allocator.cpp \
    database_manager.cpp \
    date_time.cpp \
    utilities.cpp \
    add_activity_dialog.cpp \
    add_field_dialog.cpp \
    activity_list_model.cpp \
    display_format.cpp \
    activity_recorder.cpp \
    activity_visualizer.cpp \
    add_activity_info_dialog.cpp \
    activity_item_delegate.cpp \
    plugin_manager.cpp \
    duk/duktape.c \
    edit_activity_field_dialog.cpp

HEADERS  += mainwindow.h \
    application_state.h \
    core_types.h \
    block_allocator.h \
    database_manager.h \
    date_time.h \
    utilities.h \
    add_activity_dialog.h \
    add_field_dialog.h \
    activity_list_model.h \
    display_format.h \
    activity_recorder.h \
    activity_visualizer.h \
    add_activity_info_dialog.h \
    activity_item_delegate.h \
    plugin_manager.h \
    duk/duk_config.h \
    duk/duktape.h \
    edit_activity_field_dialog.h \
    duktape_prepare_eval.h

FORMS    += mainwindow.ui \
    add_activity_dialog.ui \
    add_field_dialog.ui \
    add_activity_info_dialog.ui \
    edit_activity_field_dialog.ui
