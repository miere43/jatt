#include "plugin_manager.h"

#include <QCoreApplication>
#include <QMessageBox>
#include <QDebug>

#include "application_state.h"
#include "mainwindow.h"
#include "core_types.h"

//QHash<int, JSMenu> m_jsMenus;

void PluginManager_fatalHandler(void* userdata, const char* msg) {
    PluginManager* manager = (PluginManager*)userdata;
    Q_ASSERT(manager);
    Q_UNUSED(manager);

    qDebug() << msg;
    QMessageBox::critical(nullptr, "Plugin Fatal Error", "Plugin Fatal Error: " + QString::fromUtf8(msg));

    // @TODO: can I even do such weird stuff, I cannot return from this function
    // because I will stuck inside empty forever loop.
    QCoreApplication::exit(-2);
    exit(-2);
}

//duk_ret_t testFunc(duk_context* ctx) {
//    const char* obj = duk_get_string_default(ctx, -1, "null");
//    QMessageBox::critical(nullptr, "addHook called", obj);
//    duk_push_int(ctx, 10);
//    return 1;
//}

//int createMenuItemId() {
//    static int id = 0;
//    return id++;
//}

duk_ret_t app_debugLog(duk_context* ctx) {
    const char* string = duk_to_string(ctx, -1);
    qDebug() << string << "(from script)";
    return 0;
}

//duk_ret_t app_MenuItem_addItem(duk_context* ctx) {
//    duk_require_string(ctx, -1);
//    duk_require_function(ctx, -2);

//    duk_push_this(ctx); // -3
//    if (!duk_get_prop_string(ctx, -3, "_id")) { // -4
//        return DUK_RET_TYPE_ERROR;
//    }
//    int id = duk_get_int(ctx, -4);
//    duk_pop();

//    duk_push_global_stash(ctx);
//    duk_dup(ctx, -2);
//    duk_put_prop_index(ctx, -5, id);
//    duk_pop(ctx);

//    const char* itemName = duk_safe_to_string(ctx, -1);
//    JSMenu& i = m_jsMenus.value(id);
//    i.menu->addAction(itemName);
//}

//duk_ret_t app_MenuItem_constructor(duk_context* ctx) {
//    if (!duk_is_constructor_call(ctx)) {
//        return DUK_RET_TYPE_ERROR;
//    }
//    duk_require_string(ctx, -1); // -1 'menuName'
//    const char* menuName = duk_safe_to_string(ctx, -1);

//    duk_push_this(ctx); // -2
//    duk_push_string(ctx, "addItem");
//    duk_push_c_function(ctx, app_MenuItem_addItem, 2);
//    duk_put_prop(ctx, -2);

//    JSMenu i;
//    i.menu = new QMenu(QString(menuName));
//    i.id = createMenuItemId();
//    m_jsMenus.insert(i.id, i);

//    duk_push_int(ctx, i.menu);
//    duk_put_prop_string(ctx, -2, "_id");

//    return 0; // will return 'this'
//}

//duk_ret_t app_addMenuItemDef(duk_context* ctx) {

//}

#define _HIDDEN_PROPERTY(x) "\xff" x
const char* ACTIVITY_ID_SYMBOL = _HIDDEN_PROPERTY("id");
#undef _HIDDEN_PROPERTY

//Activity* forEachActivity_current = nullptr;

Activity* dukForEachActivityProlog(duk_context* ctx) {
    duk_push_this(ctx);
    duk_push_string(ctx, ACTIVITY_ID_SYMBOL);
    duk_get_prop(ctx, 0);
    return (Activity*)duk_get_pointer(ctx, -1);
}

duk_ret_t app_forEachActivity_activityId(duk_context* ctx) {
    Activity* activity = dukForEachActivityProlog(ctx);
    if (!activity) return 0; // undefined
    duk_push_int(ctx, activity->id);
    return 1;
}


duk_ret_t app_forEachActivity_activityDisplayString(duk_context* ctx) {
    Activity* activity = dukForEachActivityProlog(ctx);
    if (!activity) return 0; // undefined

    QString displayString = activity->displayString();
    duk_push_string(ctx, displayString.toUtf8().data());
    return 1;
}

duk_ret_t app_forEachActivity(duk_context* ctx) {
    duk_require_function(ctx, 0); // -1
    duk_idx_t idx = duk_push_object(ctx); // -2
//    duk_put_function_list(ctx, idx, forEachActivityFunctionList);
////    duk_put_
//    duk_push_string(ctx, "hello");
    duk_push_c_function(ctx, app_forEachActivity_activityId, 0);
    duk_put_prop_string(ctx, idx, "id");
    duk_push_c_function(ctx, app_forEachActivity_activityDisplayString, 0);
    duk_put_prop_string(ctx, idx, "displayString");

    const QVector<Activity*>& activities = g_app.mainWindow()->currentActivities();
    for (int i = 0; i < activities.count(); ++i) {
        Activity* a = activities.at(i);

        duk_push_null(ctx);
        duk_put_prop_string(ctx, idx, ACTIVITY_ID_SYMBOL);
        duk_push_pointer(ctx, a);
        duk_put_prop_string(ctx, idx, ACTIVITY_ID_SYMBOL);

        duk_dup(ctx, 0); // function
        duk_dup(ctx, idx); // argument
        duk_call(ctx, 1);
        duk_pop(ctx); // ignore return value
    }

    // duk_remove(ctx, idx);

    return 0;
}

//duk_ret_t app_createMenu(duk_context* ctx) {
//    duk_require_string(ctx, -1);
//    const char* menuName = duk_get_string(ctx, -1);
//    QMenu* menu = new QMenu();

//    return 0;
//}

duk_ret_t app_showMessageBox(duk_context* ctx) {
    duk_require_string(ctx, 0);
    const char* str = duk_get_string(ctx, 0);
    int choice = QMessageBox::information(nullptr, "Script Message", str);
    duk_push_int(ctx, choice);
    return 1;
}

//duk_ret_t app_getter_Activity_id(duk_context* ctx) {
//    duk_push_this(ctx);

//}

//duk_ret_t app_Activity_prototypeDef(duk_context* ctx) {
////    duk_eval()
//}

const duk_function_list_entry appModuleFuncs[] = {
    { "debugLog", app_debugLog, 1 },
    { "forEachActivity", app_forEachActivity, 1 },
    { "showMessageBox", app_showMessageBox, 1 },
//    { "createMenu", app_createMenu, 1 },
    { 0, 0, 0 }
};

PluginManager::PluginManager()
{

}

bool PluginManager::initialize() {
    if (m_initialized) return true;

    ctx = duk_create_heap(nullptr, nullptr, nullptr, this, PluginManager_fatalHandler);
    if (!ctx) {
        return false;
    }

    duk_push_global_object(ctx);

    // Add 'app' module to global object.
    duk_idx_t ao = duk_push_object(ctx);
    duk_push_string(ctx, "debugLogProtected");
    duk_push_c_function(ctx, app_debugLog, 1);
    duk_def_prop(ctx, ao, DUK_DEFPROP_SET_ENUMERABLE | DUK_DEFPROP_HAVE_VALUE);

    duk_put_function_list(ctx, -1, appModuleFuncs);
    duk_put_prop_string(ctx, -2, "app");

//    duk_push_object()

    duk_pop(ctx);

//    duk_push_global_stash(ctx);
//    duk_push_array(ctx);
//    duk_put_prop_string(ctx, -1, "_menuCallbacks");
//    duk_pop(ctx);

    // duk_eval_string_noresult(ctx, "app.debugLog(app.showMessageBox('null'));");

    m_initialized = true;
    return true;
}

PluginManager::EvaluationState PluginManager::evaluate(const char* scriptSource, const char* fileName, QString* error) {
    Q_ASSERT(scriptSource);

    if (fileName == nullptr) {
        fileName = "eval";
    }

    duk_push_string(ctx, fileName);
    EvaluationState status = (duk_pcompile_string(ctx, 0, scriptSource) == DUK_EXEC_SUCCESS) ? SuccessfulEvaluation : CompilationError;
    if (status == SuccessfulEvaluation) {
        status = (duk_pcall(ctx, 0) == DUK_EXEC_SUCCESS) ? SuccessfulEvaluation : ExecutionError;
    }

    if (status != SuccessfulEvaluation && error != nullptr) {
        duk_size_t errorMessageLength;
        const char* errorMessage =  duk_safe_to_lstring(ctx, -1, &errorMessageLength);
        *error = QString::fromUtf8(errorMessage, (int)errorMessageLength);
    }

    duk_pop(ctx); // ignore return value / error

    return status;
}

bool PluginManager::shutdown() {
    if (ctx) {
        duk_destroy_heap(ctx);
        ctx = nullptr;
    }

    return true;
}

bool PluginManager::loadPlugins() {
//    if (!m_pluginsDir.exists())
//        return false;
//    m_pluginsDir.refresh();
//    m_pluginsDir.setFilter(QDir::Dirs);

//    QFileInfoList list = m_pluginsDir.entryInfoList();
//    for (int i = 0; i < list.count(); ++i) {
//        const QFileInfo& file = list.at(i);
//        if (!file.isDir())
//            continue;
//        QDir dir = QDir(file.path());
//        QFile pluginFile = QFile(dir.filePath("index.js"));
//        if (!pluginFile.exists())
//            continue;
//    }

//    QJSEngine.evaluate()
    return true;
}
