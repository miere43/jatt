#include "hotkey.h"
#include "error_macros.h"

#include <QCoreApplication>
#include <QDebug>

#ifdef Q_OS_WIN
#include <Windows.h>
#endif

HotkeyEventFilter* HotkeyEventFilter::m_eventFilter = nullptr;

bool convertQtKeycodes(Qt::KeyboardModifiers modifiers, Qt::Key key, quint32* winModifiers, quint32* winKey) {
    if (modifiers & Qt::MetaModifier || modifiers & Qt::KeypadModifier || modifiers & Qt::GroupSwitchModifier) {
        return false;
    }

#ifdef Q_OS_WIN
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
#else
    Q_UNUSED(winModifiers);
    Q_UNUSED(key);
    Q_UNUSED(winKey);
#endif
    return true;
}

QString formatErrorMessage(quint32 msg) {
#ifdef Q_OS_WIN
    wchar_t* errorMessage = nullptr;
    if (!FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, msg, 0, reinterpret_cast<wchar_t*>(&errorMessage), 65535*2, nullptr)) {
        return QStringLiteral("unknown");
    } else {
        QString qstring = QString::fromWCharArray(errorMessage);
        LocalFree(reinterpret_cast<HLOCAL>(errorMessage));
        return qstring;
    }
#else
    Q_UNUSED(msg);
    return QString();
#endif
}

Hotkey::Hotkey(void* window, int id, Qt::KeyboardModifiers modifiers, Qt::Key key, HotkeyCallback callback, void* userdata)
{
    Q_UNUSED(modifiers)
    Q_UNUSED(key)

    m_isActive = false;

    ERR_VERIFY(id >= 0x0000 && id <= 0xBFFF);
    ERR_VERIFY(window != nullptr);
    ERR_VERIFY_NULL(callback);

    m_errorMessage = QString();
    m_callback = callback;
    m_userdata = userdata;
    m_window = window;
    m_id = id;

#ifdef Q_OS_WIN
    m_winModifiers = MOD_CONTROL;
    m_winKey = VK_SPACE;

//    if (!convertQtKeycodes(modifiers, key, &m_winModifiers, &m_winKey)) {
//        m_errorMessage = QStringLiteral("Unsupported keyboard modifier or key.");
//        return;
//    }
    m_winModifiers |= MOD_NOREPEAT;
#endif
}

Hotkey::~Hotkey()
{
    // @TODO: setActive crashes program
    // setActive(false);
}

bool Hotkey::setActive(bool active)
{
    if (m_isActive == active)
        return true;

#ifdef Q_OS_WIN
    if (m_isActive)
    {
        if (!UnregisterHotKey((HWND)m_window, m_id))
        {
            m_errorMessage = formatErrorMessage(GetLastError());
            return false;
        }
        HotkeyEventFilter::unregisterHotkey(this);
    }
    else
    {
        if (!RegisterHotKey((HWND)m_window, m_id, m_winModifiers, m_winKey))
        {
            m_errorMessage = formatErrorMessage(GetLastError());
            return false;
        }
        if (!HotkeyEventFilter::isInstalled())
            HotkeyEventFilter::installEventFilter();
        HotkeyEventFilter::registerHotkey(this);
    }
    m_isActive = active;
#endif
    return true;
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

void HotkeyEventFilter::registerHotkey(Hotkey *hotkey)
{
    ERR_VERIFY_NULL(hotkey);
    ERR_VERIFY(!hotkey->isActive());
    ERR_VERIFY_NULL(hotkey->m_callback);
    ERR_VERIFY(HotkeyEventFilter::isInstalled());
    HotkeyEventFilter::m_eventFilter->m_hotkeys.append(hotkey);
}

void HotkeyEventFilter::unregisterHotkey(Hotkey *hotkey) {
    ERR_VERIFY_NULL(hotkey);
    ERR_VERIFY(hotkey->isActive());
    ERR_VERIFY(HotkeyEventFilter::isInstalled());
    HotkeyEventFilter::m_eventFilter->m_hotkeys.removeOne(hotkey);
}

bool HotkeyEventFilter::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) {
    Q_UNUSED(eventType)
    Q_UNUSED(message)
    Q_UNUSED(result)
#ifndef Q_OS_WIN
    return false;
#else
    MSG* msg = reinterpret_cast<MSG*>(message);
    if (msg->message == WM_HOTKEY) {
        for (Hotkey* hotkey : m_hotkeys) {
            if (hotkey->isActive() && hotkey->m_id == static_cast<int>(msg->wParam)) {
                ERR_VERIFY_CONTINUE(hotkey->m_callback);
                hotkey->m_callback(hotkey, hotkey->m_userdata);
                return true;
            }
        }
    }
    return false;
#endif
}
