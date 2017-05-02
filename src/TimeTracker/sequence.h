//#ifndef SEQUENCE_H
//#define SEQUENCE_H

//#include <string.h>

//template <typename T>
//class Sequence {
//private:
//    int m_length;
//    int m_capacity;
//    T*  m_data;

//    int reserveOne() {
//        if ()
//    }

//public:
//    Sequence(int capacity = 10) {
//        Q_ASSERT(capacity >= 0);
//        m_capacity = capacity;
//    }

//    void deallocate() {
//        delete m_data;
//        m_data = nullptr;
//        m_length = 0;
//        m_capacity = 0;
//    }

//    void reserve(int amount) {
//        Q_ASSERT(amount >= 0);
//        if (amount <= 0 || m_capacity <= amount) {
//            return;
//        }

//        if (m_data) {
//            T* newData = new T[amount];
//            if (!newData) {
//                Q_ASSERT(false); // out of memory
//                return;
//            }
//            memcpy(newData, m_data, sizeof(T) * m_length);
//            delete m_data;
//        }
//    }

//    void pushBack(T value) {
//        reserve(m_length + 1);
//        m_data[]
//    }
//};

//#endif // SEQUENCE_H
