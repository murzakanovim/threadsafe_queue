#include <queue>
#include <mutex>
#include <condition_variable>

template <typename T>
class threadsafe_queue
{
private:
    mutable std::mutex mut;
    std::queue<T> data_queue;
    std::condition_variable data_cond;

public:
    threadsafe_queue();
    threadsafe_queue(const threadsafe_queue &other);

    threadsafe_queue &operator=(const threadsafe_queue &) = delete;

    void push(T new_value);

    bool try_pop(T &value);
    std::shared_ptr<T> try_pop();

    void wait_and_pop(T &value);
    std::shared_ptr<T> wait_and_pop();

    bool empty() const;
};

template <typename T>
bool threadsafe_queue<T>::empty() const
{
    std::lock_guard<std::mutex> lk(mut);
    return data_queue.empty();
}

template <typename T>
threadsafe_queue<T>::threadsafe_queue()
{
}

template <typename T>
threadsafe_queue<T>::threadsafe_queue(const threadsafe_queue &other)
{
    std::lock_guard<std::mutex> lk(other.mut);
    data_queue = other.data_queue;
}

template <typename T>
void threadsafe_queue<T>::push(T new_value)
{
    std::lock_guard<std::mutex> lk(mut);
    data_queue.push(new_value);
    data_cond.notify_one();
}

template <typename T>
bool threadsafe_queue<T>::try_pop(T &value)
{
    std::lock_guard<std::mutex> lk(mut);
    if (data_queue.empty())
        return false;
    value = data_queue.front();
    data_queue.pop();
    return true;
}

template <typename T>
std::shared_ptr<T> threadsafe_queue<T>::try_pop()
{
    std::lock_guard<std::mutex> lk(mut);
    if (data_queue.empty())
        return std::shared_ptr<T>();
    std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
    data_queue.pop();
    return res;
}

template <typename T>
void threadsafe_queue<T>::wait_and_pop(T &value)
{
    std::unique_lock<std::mutex> lk(mut);
    data_cond.wait(lk, [this]
                   { return !data_queue.empty(); });
    value = data_queue.front();
    data_queue.pop();
}

template <typename T>
std::shared_ptr<T> threadsafe_queue<T>::wait_and_pop()
{
    std::unique_lock<std::mutex> lk(mut);
    data_cond.wait(lk, [this]
                   { return !data_queue.empty(); });
    std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
    data_queue.pop();
    return res;
}
