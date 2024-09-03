#include "extension.h"

/**
 * @brief A thread-safe queue implementation.
 * 
 * This class provides a queue that can be safely used across multiple threads.
 * It uses mutex and condition variable for synchronization.
 * 
 * @tparam T The type of elements stored in the queue.
 */
template <class T>
class ThreadSafeQueue {
private:
	std::queue<T> queue;
	mutable std::mutex mutex;
	std::condition_variable cond;

public:
	/**
	 * @brief Default constructor.
	 */
	ThreadSafeQueue() = default;

	/**
	 * @brief Deleted copy constructor to prevent accidental copying.
	 */
	ThreadSafeQueue(const ThreadSafeQueue&) = delete;

	/**
	 * @brief Deleted assignment operator to prevent accidental assignment.
	 */
	ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;

	/**
	 * @brief Pushes an item onto the queue.
	 * 
	 * @param item The item to be pushed.
	 */
	void Push(T item) {
		std::unique_lock<std::mutex> lock(mutex);
		queue.push(std::move(item));
		lock.unlock();
		cond.notify_one();
	}

	/**
	 * @brief Tries to pop an item from the queue.
	 * 
	 * @param[out] item The popped item, if successful.
	 * @return true if an item was popped, false if the queue was empty.
	 */
	bool TryPop(T& item) {
		std::unique_lock<std::mutex> lock(mutex);
		if (queue.empty()) {
			return false;
		}
		item = std::move(queue.front());
		queue.pop();
		return true;
	}

	/**
	 * @brief Waits for an item and pops it from the queue.
	 * 
	 * This method will block until an item is available.
	 * 
	 * @return The popped item.
	 */
	T WaitAndPop() {
		std::unique_lock<std::mutex> lock(mutex);
		cond.wait(lock, [this] { return !queue.empty(); });
		T item = std::move(queue.front());
		queue.pop();
		return item;
	}

	/**
	 * @brief Clears all items from the queue.
	 */
	void Clear() {
		std::unique_lock<std::mutex> lock(mutex);
		queue = std::queue<T>();
	}

	/**
	 * @brief Checks if the queue is empty.
	 * 
	 * @return true if the queue is empty, false otherwise.
	 */
	bool Empty() const {
		std::lock_guard<std::mutex> lock(mutex);
		return queue.empty();
	}

	/**
	 * @brief Gets the current size of the queue.
	 * 
	 * @return The number of items in the queue.
	 */
	size_t Size() const {
		std::lock_guard<std::mutex> lock(mutex);
		return queue.size();
	}
};