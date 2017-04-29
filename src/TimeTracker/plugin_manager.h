#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H

#include "duk/duktape.h"
#include <QDir>
#include <QMenu>
#include <QHash>

//const char* plugin_source = "function menuClicked(menuIdX) {"
//        "app.logString(menuIdX.toString())"
//    "}"
//    "var menuId = app.registerMenu({"
//"        ['Test Item', menuClicked]"
//"    });";

//struct PluginInfo {
//    QFile mainFile;
//};

//struct JSAction {
//    int callbackIndex;
//};

//struct JSMenu{
//    int id;
//    QVector<
//    int stashId;
//    QMenu* menu;
//};

class PluginManager
{
private:
    duk_context* ctx;
    QDir m_pluginsDir;
    bool m_initialized = false;


    // void script_addHook(QJSValue value);
public:
    PluginManager();

    bool initialize();
    bool shutdown();

    bool loadPlugins();

    // inline QJSEngine* engine() const { return &m_engine; }
};

void PluginManager_fatalHandler(void* userdata, const char* msg);


#endif // PLUGIN_MANAGER_H
