#include <cstddef>
#include <mutex>
#ifndef lacpp_sorted_list_hpp
#define lacpp_sorted_list_hpp lacpp_sorted_list_hpp

/* a sorted list implementation by David Klaftenegger, 2015
 * please report bugs or suggest improvements to david.klaftenegger@it.uu.se
 */

/* struct for list nodes */
template<typename T>
struct node {
	T			value;
	node<T>*	next;
	std::mutex	mutex;
};

/* non-concurrent sorted singly-linked list */
template<typename T>
class sorted_list {
	node<T>*	first = nullptr;
	std::mutex	head_mutex;

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
		std::unique_lock<std::mutex> head_lock(head_mutex);
		/* first find position */
		node<T>* pred = nullptr;
		node<T>* head = first;

		// List traversal:
		// For each node in the list where we have a successor and the successors
		// value is less than v, we:
		// - unlock the head_lock, because we do not need it anymore and other threads
		// can continue with their operations.
		// - update the predesessor to be the current node
		// - create a new lock for the predesessor as we might modify it later down
		// - update the successor
		// - update the head lock to represent the new head
		while(head != nullptr && head->value < v) {
			pred = head;
			head = head->next;
			head_lock.unlock();
			std::unique_lock<std::mutex> pred_lock(pred->mutex);
			head_lock = std::move(pred_lock);
		}
		
		/* construct mew node */
		node<T>* curr = new node<T>();
		curr->value	= v;
		curr->next	= head; /* insert new node between pred and succ */

		if(pred == nullptr) {
			first = curr;
		} else {
			pred->next = curr;
		} 
	}

	void remove(T v) {
		std::unique_lock<std::mutex> head_lock(head_mutex);
		/* first find position */
		node<T>* pred = nullptr;
		node<T>* curr = first;
		while(curr != nullptr && curr->value < v) {
			pred = curr;
			curr = curr->next;
			head_lock.unlock();
			std::unique_lock<std::mutex> pred_lock(pred->mutex);
			head_lock = std::move(pred_lock);
		}
		if(curr == nullptr || curr->value != v) {
			/* v not found */
			return;
		}

		if(pred == nullptr) {
			first = curr->next;
		} else {
			std::unique_lock<std::mutex> pred_lock(pred->mutex);
			pred->next = curr->next;
		}

		delete curr;
	}

	/* count elements with value v in the list */
	std::size_t count(T v) {
		std::unique_lock<std::mutex> head_lock(head_mutex);
		std::size_t cnt = 0;
		/* first go to value v */
		node<T>* curr = first;
		while(curr != nullptr && curr->value < v) {
			std::unique_lock<std::mutex> curr_lock(curr->mutex);
			if (curr_lock)
			{
				head_lock.unlock();
			}
			curr = curr->next;
			head_lock = std::move(curr_lock);
		}
		/* count elements */
		while(curr != nullptr && curr->value == v) {
			cnt++;
			std::unique_lock<std::mutex> curr_lock(curr->mutex);
			if (curr_lock)
			{
				head_lock.unlock();
			}
			curr = curr->next;
			head_lock = std::move(curr_lock);
		}
		return cnt;
	}
};

#endif // lacpp_sorted_list_hpp
