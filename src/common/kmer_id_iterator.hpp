/**
 * @file    counting_iterator.hpp
 * @ingroup bliss::iterators
 * @author  tpan
 * @brief   contains a counting iterator
 * @details
 *
 * Copyright (c) 2014 Georgia Institute of Technology.  All Rights Reserved.
 *
 * TODO add License
 */
#ifndef COUNTING_ITERATOR_HPP_
#define COUNTING_ITERATOR_HPP_

#include <cstddef>

#include <type_traits>
#include <iterator>


namespace bliss
{
  namespace iterator
  {

    /**
     * @class    bliss::iterator::CountingIterator
     * @brief    iterator that iterates from start to end in specified steps size
     * @details  counting iterator counts from the start to the end in regular steps.
     *           this is useful to establish index values for some other array.
     * @tparam T the type of data to over.
     */
    template<typename T>
    class CountingIterator :  public std::iterator<std::random_access_iterator_tag, T>
    {
      protected:

        /// difference type
        using D = std::ptrdiff_t;

        /// the start of the iterator
        mutable T start;

        /// the stride for each iteration.
        mutable T stride;

        /// current value;
        T val;

      public:

        /**
         * constructor
         * @param _start      first value
         * @param _stride     the distance traversed during each call to the increment/decrement method.
         */
        CountingIterator(const T& _start, const T &_stride) : start(_start), stride(_stride), val(_start) {};

        /**
         * constructor, with stride defaults to 1.
         * @param _start    first value
         */
        CountingIterator(const T& _start) : start(_start), stride(1), val(_start) {};

        /**
         * default constructor, sets start to 0, stride to 1
         */
        CountingIterator() : start(0), stride(1), val(0) {};

        /**
         * default copy constructor
         * @param other  instance of CountingIterator to copy from
         */
        CountingIterator(const CountingIterator<T> & other) : start(other.start), stride(other.stride), val(other.val) {};

        /**
         * default copy assignment operator
         * @param other  instance of CountingIterator to copy from
         * @return reference to self
         */
        CountingIterator<T>& operator=(const CountingIterator<T> & other) {
          start = other.start;
          stride = other.stride;
          val = other.val;
          return *this;
        }

        /**
         * default move constructor
         * @param other  instance of CountingIterator to move from
         */
        CountingIterator(CountingIterator<T> && other) : start(other.start), stride(other.stride), val(other.val) {
          other.start = 0;
          other.stride = 1;
          other.val = 0;
        };

        /**
         * default move assignment operator
         * @param other  instance of CountingIterator to move from
         * @return reference to self
         */
        CountingIterator<T>& operator=(CountingIterator<T> && other) {
          start = other.start;     other.start = 0;
          stride = other.stride;   other.stride = 1;
          val = other.val;         other.val = 0;
          return *this;
        };

        /**
         * default destructor
         */
        virtual ~CountingIterator() {};


        /**
         * @brief   pre-increment operator: ++iter
         * @return  reference to incremented iterator
         */
        CountingIterator<T>& operator++() {
          val += stride;
          return *this;
        }

        /**
         * @brief   post-increment operator: iter++
         * @param   dummy for c++ to identify this as post increment.
         * @return  incremented copy of iterator
         */
        CountingIterator<T> operator++(int) {
          CountingIterator<T> out(*this);
          ++out;
          return out;
        }

        /**
         * @brief compare to other iterator for equality.  only inspect the val
         * @param other   iterator to compare to.
         * @return  bool, true if equal, false otherwise.
         */
        bool operator==(const CountingIterator<T>& other) {
          return val == other.val;
        }

        /**
         * @brief compare to other iterator for inequality
         * @param other   iterator to compare to.
         * @return  bool, true if not equal, false otherwise.
         */
        bool operator!=(const CountingIterator<T>& other) {
          return !(this->operator==(other));
        }

        /**
         * @brief dereference function, *iter
         * @return  current value
         */
        const T operator*() const {
          return val;
        }

        /**
         * @brief   pre-decrement operator: --iter
         * @return  reference to decremented iterator
         */
        CountingIterator<T>& operator--() {
          val -= stride;
          return *this;
        }

        /**
         * @brief   post-decrement operator: iter--
         * @param   dummy for c++ to identify this as post decrement.
         * @return  decremented copy of iterator
         */
        CountingIterator<T> operator--(int) {
          CountingIterator<T> out(*this);
          --out;
          return out;
        }

        /**
         * @brief   arithmetic increment by multiple steps
         * @param diff    number of steps to increment by
         * @return  incremented copy of iterator
         */
        CountingIterator<T> operator+(const D& diff) {
          CountingIterator<T> out(*this);
          out += diff;
          return out;
        }


        /**
         * @brief   arithmetic decrement by multiple steps
         * @param diff    number of steps to decrement by
         * @return  decremented copy of iterator
         */
        CountingIterator<T> operator-(const D& diff) {
          CountingIterator<T> out(*this);
          out -= diff;
          return out;
        }

        /**
         * @brief difference between 2 iterators;
         * @param other         the iterator to subtract by
         * @return              distance between the iterators
         */
        D operator-(const CountingIterator<T>& other) {
          return (val - other.val) / stride;
        }


        /**
         * @brief compare to other iterator: >
         * @param other   iterator to compare to.
         * @return  bool, true if greater than, false otherwise.
         */
        bool operator>(const CountingIterator<T>& other) {
          return val > other.val;
        }

        /**
         * @brief compare to other iterator: <
         * @param other   iterator to compare to.
         * @return  bool, true if less than, false otherwise.
         */
        bool operator<(const CountingIterator<T>& other) {
          return val < other.val;
        }

        /**
         * @brief compare to other iterator: >=
         * @param other   iterator to compare to.
         * @return  bool, true if greater than or equal to, false otherwise.
         */
        bool operator>=(const CountingIterator<T>& other) {
          return val >= other.val;
        }

        /**
         * @brief compare to other iterator: <=
         * @param other   iterator to compare to.
         * @return  bool, true if less than or equal to, false otherwise.
         */
        bool operator<=(const CountingIterator<T>& other) {
          return val <= other.val;
        }

        /**
         * @brief   arithmetic increment by multiple steps
         * @param diff    number of steps to increment by
         * @return  reference to incremented iterator
         */
        CountingIterator<T>& operator+=(const D& diff) {
          val += diff * stride;
          return *this;
        }

        /**
         * @brief   arithmetic decrement by multiple steps
         * @param diff    number of steps to decrement by
         * @return  reference to decremented iterator
         */
        CountingIterator<T>& operator-=(const D& diff) {
          val -= diff * stride;
          return *this;
        }

        /**
         * @brief   offset dereference operator
         * @param i offset at which the value is retrieved.
         * @return  value for ith offset
         */
        T operator[](const D& i) {
          return val + stride * i;
        }

    };

    /**
     * @brief increment operator, with first operand being a number and second being an iterator  n + iter;
     * @tparam T    value type for which to perform the increment operation
     * @param diff          number of steps to increment by
     * @param self          iterator to increment
     * @return              copy of incremented iterator
     */
    template<typename T>
    CountingIterator<T> operator+(const typename std::iterator_traits<CountingIterator<T> >::difference_type& diff, CountingIterator<T>& self) {
      return self + diff;
    }




  } /* namespace iterator */
} /* namespace bliss */

#endif /* COUNTING_ITERATOR_HPP_ */
