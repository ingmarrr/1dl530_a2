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
};

/* non-concurrent sorted singly-linked list */
template<typename T>
class sorted_list {
	node<T>*	first = nullptr;
	tatas_lock	mutex;

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
			while(first != nullptr) {
				remove(first->value);
			}
		}
		/* insert v into the list */
		void insert(T v) {
			/* std::lock_guard<tatas_lock> lock(mutex); */
			mutex.lock();
			/* first find position */
			node<T>* pred = nullptr;
			node<T>* succ = first;
			while(succ != nullptr && succ->value < v) {
				pred = succ;
				succ = succ->next;
			}
			
			/* construct new node */
			node<T>* current = new node<T>();
			current->value = v;

			/* insert new node between pred and succ */
			current->next = succ;
			if(pred == nullptr) {
				first = current;
			} else {
				pred->next = current;
			}
			mutex.unlock();
		}

		void remove(T v) {
			mutex.lock();
			/* first find position */
			node<T>* pred = nullptr;
			node<T>* current = first;
			while(current != nullptr && current->value < v) {
				pred = current;
				current = current->next;
			}
			if(current == nullptr || current->value != v) {
				/* v not found */
				mutex.unlock();
				return;
			}
			/* remove current */
			if(pred == nullptr) {
				first = current->next;
			} else {
				pred->next = current->next;
			}
			mutex.unlock();
			delete current;
		}

		/* count elements with value v in the list */
		std::size_t count(T v) {
			mutex.lock();
			std::size_t cnt = 0;
			/* first go to value v */
			node<T>* current = first;
			while(current != nullptr && current->value < v) {
				current = current->next;
			}
			/* count elements */
			while(current != nullptr && current->value == v) {
				cnt++;
				current = current->next;
			}
			mutex.unlock();
			return cnt;
		}
};

#endif // lacpp_sorted_list_hpp
