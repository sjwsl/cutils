#include <atomic>
#include <mutex>

namespace cutils {

template<typename T>
struct Node
{
    Node(const T &value) : data(value) { }
    T data;
    Node *next = nullptr;
};

template<typename T>
class WithLockList
{
    std::mutex mtx;
    Node<T> *head;
public:
    void pushFront(const T &value)
    {
        auto *node = new Node<T>(value);
        std::lock_guard<mutex> lock(mtx);
        node->next = head;
        head = node;
    }
};

template<typename T>
class LockFreeList
{
    std::atomic<Node<T> *> head;
public:
    void pushFront(const T &value)
    {
        auto *node = new Node<T>(value);
        node->next = head.load();
        while(!head.compare_exchange_weak(node->next, node)); 
    }
};

} // namespace cutils