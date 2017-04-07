#-------------------------------------------------
#
# Project created by QtCreator 2017-02-20T22:54:28
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TimeTracker
TEMPLATE = app

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
    newcategorydialog.cpp \
    application_state.cpp \
    session_list_model.cpp \
    core_types.cpp \
    session_item_delegate.cpp \
    block_allocator.cpp \
    session_recorder.cpp \
    database_manager.cpp \
    recording_list_model.cpp \
    recording_item_delegate.cpp \
    recording_dialog.cpp \
    session_visualizer.cpp \
    edit_session_dialog.cpp \
    edit_recording_dialog.cpp \
    edit_tags_dialog.cpp \
    tag_list_model.cpp \
    date_time.cpp \
    utilities.cpp \
    add_activity_dialog.cpp

HEADERS  += mainwindow.h \
    newcategorydialog.h \
    application_state.h \
    session_list_model.h \
    core_types.h \
    session_item_delegate.h \
    block_allocator.h \
    session_recorder.h \
    database_manager.h \
    common.h \
    recording_list_model.h \
    recording_item_delegate.h \
    recording_dialog.h \
    session_visualizer.h \
    edit_session_dialog.h \
    edit_recording_dialog.h \
    edit_tags_dialog.h \
    tag_list_model.h \
    date_time.h \
    utilities.h \
    add_activity_dialog.h

FORMS    += mainwindow.ui \
    newcategorydialog.ui \
    edit_session_dialog.ui \
    edit_recording_dialog.ui \
    edit_tags_dialog.ui \
    add_activity_dialog.ui
