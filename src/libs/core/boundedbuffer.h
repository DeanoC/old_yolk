#ifndef YOLK_CORE_BOUNDEDBUFFER_H_
#define YOLK_CORE_BOUNDEDBUFFER_H_ 1

#include <boost/circular_buffer.hpp>
#include <boost/bind.hpp>

namespace Core {
	// based on teh boost::circular_buffer example
template <class T>
   class BoundedBuffer {
   public:

      typedef boost::circular_buffer<T> container_type;
      typedef typename container_type::size_type size_type;
      typedef typename container_type::value_type value_type;

      explicit BoundedBuffer(size_type capacity) : unread(0), container(capacity) {}

      void push_front(const value_type& item) {
         std::unique_lock<std::mutex> lock(mutx);
         notFull.wait(lock, boost::bind(&BoundedBuffer<value_type>::isNotFull, this));
         container.push_front(item);
         ++unread;
         lock.unlock();
         notEmpty.notify_one();
      }

      void pop_back(value_type* pItem) {
         std::unique_lock<std::mutex> lock(mutx);
         notEmpty.wait(lock, boost::bind(&BoundedBuffer<value_type>::isNotEmpty, this));
         *pItem = container[--unread];
         lock.unlock();
         notFull.notify_one();
      }

      bool isNotEmpty() const { return unread > 0; }
      bool isNotFull() const { return unread < container.capacity(); }

   private:
      BoundedBuffer(const BoundedBuffer&);              // Disabled copy constructor
      BoundedBuffer& operator = (const BoundedBuffer&); // Disabled assign operator

      size_type 				unread;
      container_type 			container;
      Core::mutex 				mutx;
      Core::condition_variable 	notEmpty;
      Core::condition_variable 	notFull;
   };
};

#endif