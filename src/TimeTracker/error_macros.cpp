#include "error_macros.h"

ErrorListenerListItem* errorListenerList = nullptr;

ErrorListenerListItem::ErrorListenerListItem(ErrorListener func, void* userdata, ErrorListenerListItem* next) {
    this->func = func;
    this->userdata = userdata;
    this->next = next;
}

void addErrorListener(ErrorListener func, void *userdata) {
    if (!errorListenerList) {
        errorListenerList = new ErrorListenerListItem(func, userdata, nullptr);
    } else {
        ErrorListenerListItem* curr = errorListenerList;
        do {
            if (curr->next) {
                curr = curr->next;
            } else {
                curr->next = new ErrorListenerListItem(func, userdata, nullptr);
                return;
            }
        } while (1);
    }
}

void shutdownErrorListeners() {
    ErrorListenerListItem* curr = errorListenerList;
    while (curr) {
        ErrorListenerListItem* next = curr->next;
        delete curr;
        curr = next;
    }
}

void _callErrorListeners(const char *function, const char *file, int line, const char *message) {
    ErrorListenerListItem* curr = errorListenerList;
    while (curr != nullptr) {
        curr->func(function, file, line, message, curr->userdata);
        curr = curr->next;
    }
}
