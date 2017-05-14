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
    do { \
        if (!(cond)) { \
            _callErrorListeners(__FUNCTION__, __FILE__, __LINE__, "Condition \"" #cond "\" is false."); \
            return; \
        } \
    } while (0)

/** Just like Q_ASSERT, but returns specified value if 'cond' evaluates to false and reports error to registered error handlers. **/
#define ERR_VERIFY_V(cond, returnValue) \
    do { \
        if (!(cond)) { \
            _callErrorListeners(__FUNCTION__, __FILE__, __LINE__, "Condition \"" #cond "\" is false, returning \"" #returnValue "\"."); \
            return (returnValue); \
        } \
    } while (0)

/** Just like Q_ASSERT, but executes 'continue' statement if 'cond' evaluates to false and reports error to registered error handlers. **/
#define ERR_VERIFY_CONTINUE(cond) \
    do { \
        if (!(cond)) { \
            _callErrorListeners(__FUNCTION__, __FILE__, __LINE__, "Condition \"" #cond "\" is false."); \
            continue;         \
        } \
    } while (0)

#endif // ERROR_MACROS_H
