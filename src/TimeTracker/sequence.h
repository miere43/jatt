#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <string.h>

template <typename T>
class Sequence
{
public:
    Sequence(int capacity = 10) {
        m_capac
    }

    void reserve(int amount) {
        if (m_capacity <= amount) {
            return;
        }

        if (m_data) {
            T* newData = new T[amount];
            if (!newData) {
                Q_ASSERT(false); // out of memory
                return;
            }
            memcpy(newData, m_data, sizeof(T) * m_length);
            delete m_data;
        }
    }

private:
    int m_length;
    int m_capacity;
    T* m_data;
};

#endif // SEQUENCE_H
