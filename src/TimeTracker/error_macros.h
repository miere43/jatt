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

/** Just like Q_ASSERT, but returns if 'cond' evaluates to false and reports error to registered error handlers. **/
#define ERR_VERIFY(cond) \
    { \
        if (!(cond)) { \
            _callErrorListeners(__FUNCTION__, __FILE__, __LINE__, "Condition " #cond " failed."); \
            return; \
        } \
    }

/** Just like Q_ASSERT, but returns specified value if 'cond' evaluates to false and reports error to registered error handlers. **/
#define ERR_VERIFY_V(cond, returnValue) \
    { \
        if (!(cond)) { \
            _callErrorListeners(__FUNCTION__, __FILE__, __LINE__, "Condition " #cond " failed"); \
            return (returnValue); \
        } \
    }

/** Just like Q_ASSERT, but executes 'continue' statement if 'cond' evaluates to false and reports error to registered error handlers. **/
#define ERR_VERIFY_CONTINUE(cond) \
    { \
        if (!(cond)) { \
            _callErrorListeners(__FUNCTION__, __FILE__, __LINE__, "Condition " #cond " failed."); \
            continue;         \
        } \
    }
#endif // ERROR_MACROS_H
