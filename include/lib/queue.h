#include <lib/list.h>

namespace libcxx
{
template <typename T, libcxx::node<T> T::*Link,
          class Container = libcxx::list<T, Link>>
class queue
{
private:
    Container container;

public:
    queue(){};
    ~queue(){};

    queue(const queue& other) = delete;
    queue& operator=(const queue& other) = delete;

    bool empty()
    {
        return this->container.empty();
    }

    size_t size()
    {
        return this->container.size();
    }

    T& back()
    {
        return this->container.back();
    }

    T& front()
    {
        return this->container.front();
    }

    void push(T& node)
    {
        this->container.push_back(node);
    }

    void pop()
    {
        this->container.pop_front();
    }

    // Special for Quark
    void erase(T& node)
    {
        this->container.erase(this->container.iterator_to(node));
    }

    // Escape hatch
    Container& get_container()
    {
        return this->container;
    }
};
} // namespace libcxx
