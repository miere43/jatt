#ifndef ERROR_MACROS_H
#define ERROR_MACROS_H

#include <QString>

typedef void (*ErrorListener)(const char* function, const char* file, int line, const char* message, void* userdata);
struct ErrorListenerListItem {
    ErrorListener func;
    void* userdata;

    ErrorListenerListItem(ErrorListener func, void* userdata, ErrorListenerListItem* next);

    ErrorListenerListItem* next;
};

extern ErrorListenerListItem* errorListenerList;

void addErrorListener(ErrorListener func, void* userdata);
void _callErrorListeners(const char* function, const char* file, int line, const char* message);

#if defined(QT_DEBUG) && defined(APP_BREAK_ON_ERROR)
    #ifdef Q_CC_MSVC
        #define _APP_DEBUGBREAK() __debugbreak()
    #else
        #define _APP_DEBUGBREAK()
    #endif
#else
    #define _APP_DEBUGBREAK()
#endif

/** Just like Q_ASSERT, but returns if 'cond' evaluates to false and reports error to registered error handlers. **/
#define ERR_VERIFY(cond) \
    do { \
        if (!(cond)) { \
            _callErrorListeners(__FUNCTION__, __FILE__, __LINE__, "Condition \"" #cond "\" is false."); \
            _APP_DEBUGBREAK(); \
            return; \
        } \
    } while (0)

/** Just like Q_ASSERT, but returns specified value if 'cond' evaluates to false and reports error to registered error handlers. **/
#define ERR_VERIFY_V(cond, returnValue) \
    do { \
        if (!(cond)) { \
            _callErrorListeners(__FUNCTION__, __FILE__, __LINE__, "Condition \"" #cond "\" is false, returning \"" #returnValue "\"."); \
            _APP_DEBUGBREAK(); \
            return (returnValue); \
        } \
    } while (0)

/** Just like Q_ASSERT, but returns if pointer is null and reports error to registered error handlers. **/
#define ERR_VERIFY_NULL(m_pointer) \
    do { \
        if (!(m_pointer)) { \
            _callErrorListeners(__FUNCTION__, __FILE__, __LINE__, "\"" #m_pointer "\" is null."); \
            _APP_DEBUGBREAK(); \
            return; \
        } \
    } while (0)

/** Just like Q_ASSERT, but returns specified value if pointer is null and reports error to registered error handlers. **/
#define ERR_VERIFY_NULL_V(m_pointer, m_returnValue) \
    do { \
        if (!(m_pointer)) { \
            _callErrorListeners(__FUNCTION__, __FILE__, __LINE__, "\"" #m_pointer "\" is null, returning \"" #m_returnValue "\""); \
            _APP_DEBUGBREAK(); \
            return m_returnValue; \
        } \
    } while (0)

/** Just like Q_ASSERT, but executes 'continue' statement if 'cond' evaluates to false and reports error to registered error handlers. **/
#define ERR_VERIFY_CONTINUE(cond) \
    do { \
        if (!(cond)) { \
            _callErrorListeners(__FUNCTION__, __FILE__, __LINE__, "Condition \"" #cond "\" is false."); \
            _APP_DEBUGBREAK(); \
            continue; \
        } \
    } while (0)

#endif // ERROR_MACROS_H
