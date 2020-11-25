#ifndef HOTKEY_H
#define HOTKEY_H
#include <QtGlobal>
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
    Hotkey(void* window, int id, Qt::KeyboardModifiers modifiers, Qt::Key key, HotkeyCallback callback, void* userdata);
    ~Hotkey();

    bool setActive(bool active);

    inline bool isActive() const { return m_isActive; }
public:
    // @TODO: temp
    QString m_errorMessage = QString();
private:
    quint32 m_winModifiers = 0;
    quint32 m_winKey = 0;

    bool m_isActive = false;
    int m_id = 0;
    void* m_window = nullptr;
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
