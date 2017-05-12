#ifndef HOTKEY_H
#define HOTKEY_H

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <QString>
#include <QAbstractNativeEventFilter>
#include <QKeySequence>
#include <QVector>

class Hotkey;
class HotkeyEventFilter;
typedef void (*HotkeyCallback)(Hotkey* hotkey, void* userdata);

class Hotkey
{
    friend class HotkeyEventFilter;
public:
    Hotkey(HWND window, int id, Qt::KeyboardModifiers modifiers, Qt::Key key, HotkeyCallback callback, void* userdata);
    ~Hotkey();

    inline bool isActive() const { return m_isActive; }
    inline QString errorMessage() const { return m_errorMessage; }

    void unregister();
private:
    QString m_errorMessage;
    bool m_isActive = false;
    int m_id;
    HWND m_window;
    void* m_userdata = nullptr;
    HotkeyCallback m_callback;
};

class HotkeyEventFilter : public QAbstractNativeEventFilter
{
public:
    static bool isInstalled();
    static void installEventFilter();
    static void registerHotkey(Hotkey* hotkey);
    static void unregisterHotkey(Hotkey* hotkey);

    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;
private:
    QVector<Hotkey*> m_hotkeys;
    static HotkeyEventFilter* m_eventFilter;
};

#endif // HOTKEY_H
