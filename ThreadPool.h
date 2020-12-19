#include <condition_variable>
#include <functional>
#include <list>
#include <mutex>
#include <thread>

class ThreadPool
{
  public:
    using Job = std::function<void()>;

    explicit ThreadPool(int threads = 2)
    {
        while (threads-- > 0)
        {
            m_threads.emplace_back(
                [this] {
                    Job j;
                    while (true)
                    {
                        {
                            std::unique_lock guard(m_jobsMutex);
                            m_waker.wait(guard, [this] { return !m_jobs.empty() || m_stop; });
                            if (m_stop) return;
                            j = std::move(m_jobs.back());
                            m_jobs.pop_back();
                        }
                        j();
                    }
                });
        }
    }

    template<class Fn>
    void add(Fn&& fn)
    {
        std::unique_lock guard(m_jobsMutex);
        m_jobs.push_back(std::forward<Fn>(fn));
        guard.unlock();
        m_waker.notify_one();
    }

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

    ~ThreadPool()
    {
        {
            std::unique_lock guard(m_jobsMutex);
            m_stop = true;
            guard.unlock();
            m_waker.notify_all();
        }
        for (auto& th : m_threads) th.join();
    }

  private:
    std::condition_variable m_waker;
    std::mutex m_jobsMutex;

    std::list<Job> m_jobs;
    bool m_stop {false};

    std::list<std::thread> m_threads;
};
