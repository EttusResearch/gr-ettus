//
// Copyright 2012-2013,2016 Ettus Research LLC
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

// This is copied from atomic.hpp in UHD. UHD deprecated the reusable_barrier.

#include <uhd/utils/atomic.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/interprocess/detail/atomic.hpp>

class reusable_barrier{
public:

    reusable_barrier():_size (0) {}

    reusable_barrier(const size_t size):_size(size) {}

    //! Resize the barrier for N threads
    void resize(const size_t size){
        _size = size;
    }

    /*!
     * Force the barrier wait to throw a boost::thread_interrupted
     * The threads were not getting the interruption_point on windows.
     */
    void interrupt(void)
    {
        _done.inc();
    }

    //! Wait on the barrier condition
    inline void wait(void)
    {
        if (_size == 1) return;

        //entry barrier with condition variable
        _entry_counter.inc();
        _entry_counter.cas(0, _size);
        boost::mutex::scoped_lock lock(_mutex);
        while (_entry_counter.read() != 0)
        {
            this->check_interrupt();
            _cond.timed_wait(lock, boost::posix_time::milliseconds(1));
        }
        lock.unlock(); //unlock before notify
        _cond.notify_one();

        //exit barrier to ensure known condition of entry count
        _exit_counter.inc();
        _exit_counter.cas(0, _size);
        while (_exit_counter.read() != 0) this->check_interrupt();
    }

    //! Wait on the barrier condition
    inline void wait_others(void)
    {
        while (_entry_counter.read() != (_size-1)) this->check_interrupt();
    }

private:
    size_t _size;
    ::uhd::atomic_uint32_t _entry_counter;
    ::uhd::atomic_uint32_t _exit_counter;
    ::uhd::atomic_uint32_t _done;
    boost::mutex _mutex;
    boost::condition_variable _cond;

    UHD_INLINE void check_interrupt(void)
    {
        if (_done.read() != 0) throw boost::thread_interrupted();
        boost::this_thread::interruption_point();
        boost::this_thread::yield();
    }
};
