#include "hotkey.h"

#include <QCoreApplication>
#include <QDebug>

HotkeyEventFilter* HotkeyEventFilter::m_eventFilter = nullptr;

bool convertQtKeycodes(Qt::KeyboardModifiers modifiers, Qt::Key key, UINT* winModifiers, UINT* winKey) {
    if (modifiers & Qt::MetaModifier || modifiers & Qt::KeypadModifier || modifiers & Qt::GroupSwitchModifier) {
        return false;
    }

    *winModifiers = 0;
    if (modifiers & Qt::ShiftModifier) {
        *winModifiers |= MOD_SHIFT;
    }
    if (modifiers & Qt::ControlModifier) {
        *winModifiers |= MOD_CONTROL;
    }
    if (modifiers & Qt::AltModifier) {
        *winModifiers |= MOD_ALT;
    }

    switch (key) {
    case Qt::Key_Space:
        *winKey = VK_SPACE;
        break;
    case Qt::Key_B:
        *winKey = 0x42;
        break;
    default:
        return false;
    }

    return true;
}

QString formatErrorMessage(DWORD msg) {
    const wchar_t* errorMessage = nullptr;
    if (!FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, 0, msg, 0, (LPWSTR)&errorMessage, 65535*2, nullptr)) {
        return QStringLiteral("unknown");
    } else {
        QString qstring = QString::fromWCharArray(errorMessage);
        LocalFree((HLOCAL)errorMessage);
        return qstring;
    }
}

Hotkey::Hotkey(HWND window, int id, Qt::KeyboardModifiers modifiers, Qt::Key key, HotkeyCallback callback, void* userdata)
{
    Q_ASSERT(id >= 0x0000 && id <= 0xBFFF);
    Q_ASSERT(window);
    Q_ASSERT(callback);

    m_errorMessage = QString::null;
    m_callback = callback;
    m_userdata = userdata;
    m_window = window;
    m_id = id;

    UINT winModifiers;
    UINT winKey;

    if (!convertQtKeycodes(modifiers, key, &winModifiers, &winKey)) {
        m_errorMessage = QStringLiteral("Unsupported keyboard modifier or key.");
        m_isActive = false;
        return;
    }

    winModifiers |= MOD_NOREPEAT;
    if (RegisterHotKey(window, id, winModifiers, winKey)) {
        m_isActive = true;
        if (!HotkeyEventFilter::isInstalled()) {
            HotkeyEventFilter::installEventFilter();
        }
        HotkeyEventFilter::registerHotkey(this);
    } else {
        m_errorMessage = formatErrorMessage(GetLastError());
        m_isActive = false;
    }
}

Hotkey::~Hotkey() {
    unregister();
}

void Hotkey::unregister() {
    if (m_isActive) {
        if (!UnregisterHotKey(m_window, m_id)) {
            qWarning() << "Unable to unregister hotkey with ID" << m_id;
        }
        HotkeyEventFilter::unregisterHotkey(this);
        m_isActive = false;
    }
}

bool HotkeyEventFilter::isInstalled() {
    return m_eventFilter != nullptr;
}

void HotkeyEventFilter::installEventFilter() {
    if (HotkeyEventFilter::isInstalled())
        return;
    HotkeyEventFilter::m_eventFilter = new HotkeyEventFilter();

    QCoreApplication* app = QCoreApplication::instance();
    app->installNativeEventFilter(m_eventFilter);
}

void HotkeyEventFilter::registerHotkey(Hotkey *hotkey) {
    Q_ASSERT(hotkey);
    Q_ASSERT(hotkey->isActive());
    Q_ASSERT(hotkey->m_callback);
    Q_ASSERT(HotkeyEventFilter::isInstalled());
    HotkeyEventFilter::m_eventFilter->m_hotkeys.append(hotkey);
}

void HotkeyEventFilter::unregisterHotkey(Hotkey *hotkey) {
    Q_ASSERT(hotkey);
    Q_ASSERT(HotkeyEventFilter::isInstalled());
    HotkeyEventFilter::m_eventFilter->m_hotkeys.removeOne(hotkey);
}

bool HotkeyEventFilter::nativeEventFilter(const QByteArray &eventType, void *message, long *result) {
    Q_UNUSED(eventType);
    Q_UNUSED(message);
    Q_UNUSED(result);
#ifndef Q_OS_WIN32
    return false;
#else
    MSG* msg = (MSG*)message;
    if (msg->message == WM_HOTKEY) {
        for (Hotkey* hotkey : m_hotkeys) {
            if (hotkey->m_id == msg->wParam) {
                hotkey->m_callback(hotkey, hotkey->m_userdata);
                return true;
            }
        }
    }
    return false;
#endif
}
