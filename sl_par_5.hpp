#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <thread>
#ifndef lacpp_sl_hpp_5
#define lacpp_sl_hpp_5 lacpp_sl_hpp_5

#ifndef __clh_lock

#define __clh_lock __clh_lock
#define CACHELINE_SIZE 64

#include <atomic>
#include <thread>

class mcs_mutex {
private:
    struct mcs_node_t {
        std::atomic<mcs_node_t*>	next{nullptr};
        std::atomic<bool>			locked{false};
    };

    std::atomic<mcs_node_t*> tail{nullptr};

    static thread_local mcs_node_t* local;

public:
    mcs_mutex() = default;

    void lock() {
        mcs_node_t* node = local;
        if (!node) {
            node = new mcs_node_t();
            local = node;
        }

        mcs_node_t* predecessor = tail.exchange(node, std::memory_order_acq_rel);

        if (predecessor != nullptr) {
            node->locked.store(true, std::memory_order_relaxed);
            predecessor->next.store(node, std::memory_order_release);

            while (node->locked.load(std::memory_order_acquire)) {
                std::this_thread::yield();
            }
        }
    }

    void unlock() {
        mcs_node_t* node = local;

        if (node->next.load(std::memory_order_relaxed) == nullptr) {
            mcs_node_t* expected = node;
            if (tail.compare_exchange_strong(expected, nullptr, std::memory_order_release, std::memory_order_relaxed)) {
                return;
            }

            while (node->next.load(std::memory_order_acquire) == nullptr) {
                std::this_thread::yield();
            }
        }

        node->next.load(std::memory_order_relaxed)->locked.store(false, std::memory_order_release);
        node->next.store(nullptr, std::memory_order_relaxed);
    }

    ~mcs_mutex() {
        mcs_node_t* last = tail.load(std::memory_order_relaxed);
        if (last) {
            delete last;
        }
    }

    mcs_mutex(const mcs_mutex&) = delete;
    mcs_mutex& operator=(const mcs_mutex&) = delete;
    mcs_mutex(mcs_mutex&&) = delete;
    mcs_mutex& operator=(mcs_mutex&&) = delete;
};

thread_local mcs_mutex::mcs_node_t* mcs_mutex::local = nullptr;

/*inline static void cpu_relax() */ 
/*{ */
/*    /1* asm volatile("pause\n": : :"memory"); *1/ */
/*    std::this_thread::yield(); */
/*} */

/*struct alignas(CACHELINE_SIZE) qnode_t */ 
/*{ */
/*	std::atomic_bool	locked; */

/*public: */
/*	qnode_t()			: locked(false) {} */
/*	qnode_t(bool init)	: locked(init) {} */

/*}; */
/*/1* static_assert(sizeof(qnode_t) == CACHELINE_SIZE, "[error] expected node to take up one cache line."); *1/ */

/*/1** */
/* * @brief The Craig-Landin-Hagersten (CLH) Lock is due to Travis Craig, Erik Hagersten, and Anders Landin[2,3] and like the Anderson Lock it uses the queue lock strategy, however, unlike the bounded queue of Anderson it uses an unbounded and, interestingly, implicit linked list of synchronisation variables. Being queue based it also offer the guarantees of first-come-first-served fairness and no thread starvation. */
/* * */
/* * [2] T. Craig. Building FIFO and priority-queueing spin locks from atomic swap. Technical Report TR 93-02-02, University of Washington, Department of Computer Science, February 1993. */
/* * [3] P. Magnussen, A. Landin, and E. Hagersten. Queue locks on cache coherent multiprocessors. In Proc. of the Eighth International Symposium on Parallel Processing (IPPS), pp. 165– 171, April 1994. IEEE Computer Society, April 1994. Vancouver */
/* *1/ */
/*class clh_mutex { */

/*    // neat cacheline padding trick by  David Geier geidav.wordpress.com */
/*    // cacheline padding is essential to avoid false sharing */
/*    using node_t = std::pair<bool, uint8_t[CACHELINE_SIZE - sizeof(bool)]>; */

/*    static_assert(sizeof(node_t) == CACHELINE_SIZE, ""); */

/*public: */

/*    clh_mutex() : tail(new node_t{}) {} */

/*    void lock() { */
/*        local_node->first = true; */
/*        prior_node = tail.exchange(local_node.get()); */
/*        while (prior_node->first) { */
/*            cpu_relax(); */
/*        } */
/*    } */

/*    void unlock() { */
/*        local_node->first = false; */
/*        node_t* tmp = prior_node; */
/*        prior_node = local_node.release(); */
/*        local_node.reset(tmp); */
/*    } */

/*    ~clh_mutex() { */
/*        delete tail.load(); */
/*    } */

/*private: */

/*    std::atomic<node_t*> tail; */

/*    static thread_local std::unique_ptr<node_t> local_node; */
/*    static thread_local node_t* prior_node; */

/*}; */

/* struct for list nodes */
template<typename T>
struct node {
	T			value;
	node<T>*	next;
	mcs_mutex	mutex;
};

/* non-concurrent sorted singly-linked list */
template<typename T>
class sorted_list {
	node<T>*	head = nullptr;
	mcs_mutex	head_mutex;

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
			if (curr != nullptr) curr->mutex.unlock();
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
		std::unique_lock<mcs_mutex> head_lock(head_mutex);
		node<T>* curr = head;

		while(curr != nullptr && curr->value < v) {
			std::unique_lock<mcs_mutex> curr_lock(curr->mutex);
			head_lock.unlock();
			curr = curr->next;
			head_lock = std::move(curr_lock);
		}

		/* count elements */
		while(curr != nullptr && curr->value == v) {
			std::unique_lock<mcs_mutex> curr_lock(curr->mutex);
			cnt++;
			node<T>* next = curr->next;
			head_lock.unlock();
			curr = next;
			head_lock = std::move(curr_lock);
		}

		return cnt;
	};
};

#endif // lacpp_sl_hpp_5
#endif // !__clh_lock
