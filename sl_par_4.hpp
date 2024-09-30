#include <atomic>
#include <cstddef>
#include <thread>
#ifndef lacpp_sorted_list_hpp
#define lacpp_sorted_list_hpp lacpp_sorted_list_hpp

/* a sorted list implementation by David Klaftenegger, 2015
 * please report bugs or suggest improvements to david.klaftenegger@it.uu.se
 */
struct tatas_lock {
	std::atomic_bool state;

public:
	void lock() {
		while (true) {
			while (state.load(std::memory_order_relaxed)) 
			{
				std::this_thread::yield();
			}
			if (!state.exchange(true, std::memory_order_acquire))
			{
				return;
			}
		}
	}

	void unlock() { state.store(false, std::memory_order_relaxed); }
};

/* struct for list nodes */
template<typename T>
struct node {
	T			value;
	node<T>*	next;
	tatas_lock	mutex;
};

/* non-concurrent sorted singly-linked list */
template<typename T>
class sorted_list {
	node<T>*	head = nullptr;
	tatas_lock	head_mutex;

public:
	/* default implementations:
	 * default constructor
	 * copy constructor (note: shallow copy)
	 * move constructor
	 * copy assignment operator (note: shallow copy)
	 * move assignment operator
	 *
	 * The first is required due to the others,
	 * which are explicitly listed due to the rule of five.
	 */
	sorted_list() = default;
	sorted_list(const sorted_list<T>& other) = default;
	sorted_list(sorted_list<T>&& other) = default;
	sorted_list<T>& operator=(const sorted_list<T>& other) = default;
	sorted_list<T>& operator=(sorted_list<T>&& other) = default;
	~sorted_list() {
		while(head != nullptr) {
			remove(head->value);
		}
	}

	void insert(T v) {
		head_mutex.lock();
		node<T>* pred = nullptr;
		node<T>* succ = head;
		if (succ != nullptr) succ->mutex.lock();

		while(succ != nullptr && succ->value < v) {
			if (pred == nullptr) head_mutex.unlock();
			else pred->mutex.unlock();
			pred = succ;
			if (succ->next != nullptr) succ->next->mutex.lock();
			succ = succ->next;
		}
		
		/* construct new node */
		node<T>* new_node = new node<T>();
		new_node->value = v;

		/* insert new node between pred and succ */
		new_node->next = succ;
		if(pred == nullptr) {
			head = new_node;
			head_mutex.unlock();
		} else {
			pred->next = new_node;
		}

		if (pred != nullptr) pred->mutex.unlock();
		if (succ != nullptr) succ->mutex.unlock();
	}

	void remove(T v) {
		head_mutex.lock();
		node<T>* pred = nullptr;
		node<T>* curr = head;
		if (curr != nullptr) curr->mutex.lock();

		while(curr != nullptr && curr->value < v) {
			if (pred == nullptr) head_mutex.unlock();
			else pred->mutex.unlock();
			pred = curr;
			if (curr->next != nullptr) curr->next->mutex.lock();
			curr = curr->next;
		}

		if(curr == nullptr || curr->value != v)
		{
			if (pred != nullptr) pred->mutex.unlock();
			else head_mutex.unlock();
			curr->mutex.unlock();
			return;
		}

		if(pred == nullptr) {
			head = curr->next;
			head_mutex.unlock();
		} else {
			pred->next = curr->next;
			pred->mutex.unlock();
		}

		curr->mutex.unlock();
		delete curr;
	}

	/* count elements with value v in the list */
	std::size_t count(T v) {
		std::size_t cnt = 0;
		std::unique_lock<tatas_lock> head_lock(head_mutex);
		node<T>* curr = head;

		while(curr != nullptr && curr->value < v) {
			std::unique_lock<tatas_lock> curr_lock(curr->mutex);
			head_lock.unlock();
			curr = curr->next;
			head_lock = std::move(curr_lock);
		}

		/* count elements */
		while(curr != nullptr && curr->value == v) {
			std::unique_lock<tatas_lock> curr_lock(curr->mutex);
			cnt++;
			node<T>* next = curr->next;
			head_lock.unlock();
			curr = next;
			head_lock = std::move(curr_lock);
		}

		return cnt;
	};
};

#endif // lacpp_sorted_list_hpp
