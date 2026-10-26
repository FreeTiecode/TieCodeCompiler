#ifndef TIE_CXX_THREAD_H
#define TIE_CXX_THREAD_H

#include <atomic>
#include <condition_variable>
#include <future>
#include <iostream>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <vector>
#include <chrono>
#include "tie_cxx_foundation.h"

namespace tie {
#if __cplusplus >= 201703L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L)
  template<typename F, typename... Args>
  using result_of_t = std::invoke_result_t<F, Args...>;
  template<typename F, typename... Args>
  using result_of = std::invoke_result<F, Args...>;
#else
  template<typename F, typename... Args>
  using result_of_t = typename std::result_of<F(Args...)>::type;
  template<typename F, typename... Args>
  using result_of = std::result_of<F(Args...)>;
#endif

  class CTieThreadPool : public CTieObject {
  public:
    explicit CTieThreadPool(size_t threads);
    ~CTieThreadPool() override;
    CTieThreadPool(const CTieThreadPool&) = delete;
    CTieThreadPool& operator=(const CTieThreadPool&) = delete;
    CTieThreadPool(CTieThreadPool&&) = delete;
    CTieThreadPool& operator=(CTieThreadPool&&) = delete;

    template <class F, class... Args>
    auto Enqueue(F&& f, Args&&... args) -> std::future<result_of_t<F, Args...>>;

    void WaitAll();

    size_t GetThreadCount() const { return workers.size(); }

    T_STRING GetTieClassName() override {
      return _TSTR("CTieThreadPool");
    }

  private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    std::mutex queue_mutex;
    std::condition_variable condition;
    std::condition_variable completion_condition;

    std::atomic_bool stop;
    std::atomic_uint active_tasks;
  };

  inline CTieThreadPool::CTieThreadPool(size_t threads) : stop(false), active_tasks(0) {
    for (size_t i = 0; i < threads; ++i) {
      workers.emplace_back([this] {
        for (;;) {
          std::function<void()> task;
          {
            std::unique_lock<std::mutex> lock(this->queue_mutex);
            this->condition.wait(
              lock, [this] { return this->stop || !this->tasks.empty(); });

            if (this->stop && this->tasks.empty()) return;

            task = std::move(this->tasks.front());
            this->tasks.pop();
            active_tasks++;
          }

          task();

          {
            std::unique_lock<std::mutex> lock(this->queue_mutex);
            active_tasks--;
            if (active_tasks == 0 && tasks.empty()) {
              completion_condition.notify_all();
            }
          }
        }
      });
    }
  }

  inline CTieThreadPool::~CTieThreadPool() {
    {
      std::unique_lock<std::mutex> lock(queue_mutex);
      stop = true;
    }

    condition.notify_all();
    for (std::thread& worker : workers) {
      worker.join();
    }
  }

  template <class F, class... Args>
  auto CTieThreadPool::Enqueue(F&& f, Args&&... args) -> std::future<result_of_t<F, Args...>> {
    using return_type = result_of_t<F, Args...>;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
      std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<return_type> res = task->get_future();
    {
      std::unique_lock<std::mutex> lock(queue_mutex);

      if (stop) {
        throw std::runtime_error("enqueue on stopped ThreadPool");
      }

      tasks.emplace([task]() { (*task)(); });
    }

    condition.notify_one();
    return res;
  }

  inline void CTieThreadPool::WaitAll() {
    std::unique_lock<std::mutex> lock(queue_mutex);
    completion_condition.wait(
      lock, [this]() { return tasks.empty() && active_tasks == 0; });
  }

  class CTieExecutors : public CTieObject {
  public:
    CTieExecutors(const CTieExecutors&) = delete;
    CTieExecutors& operator=(const CTieExecutors&) = delete;

    static std::shared_ptr<CTieThreadPool>& GetGlobalExecutor() {
      static std::shared_ptr<CTieThreadPool> global_executor = std::make_shared<CTieThreadPool>(2);
      return global_executor;
    }

    T_STRING GetTieClassName() override {
      return _TSTR("CTieExecutors");
    }

  private:
    CTieExecutors() = default;
  };

  struct LooperMessage {
    std::function<void()> task;
    T_LONG execute_time;
    int what{0};
  };

  class CTieMessageLooper : public CTieObject {
  public:
    CTieMessageLooper() = default;

    ~CTieMessageLooper() override {
      Stop();
    }

    /// 在当前线程运行Looper循环（阻塞调用，必须显式调用Quit才能退出）
    void Run() {
      if (running_) return;

      running_ = true;
      quit_requested_ = false;
      is_main_looper_ = true;

      MessageLoop();
    }

    /// 启动Looper（后台线程模式）
    void Start() {
      if (running_) return;

      running_ = true;
      quit_requested_ = false;
      looper_thread_ = std::thread([this] {
        MessageLoop();
      });
    }

    /// 请求退出Looper（不会立即退出，会处理完当前消息）
    void Quit() {
      if (!running_) return;

      std::unique_lock<std::mutex> lock(queue_mutex_);
      quit_requested_ = true;
      queue_condition_.notify_all();
    }

    /// 强制停止Looper（立即退出，不保证处理完所有消息）
    void Stop() {
      if (!running_) return;

      running_ = false;
      quit_requested_ = true;
      queue_condition_.notify_all();

      if (!is_main_looper_ && looper_thread_.joinable()) {
        looper_thread_.join();
      }
      is_main_looper_ = false;
    }

    /// 等待Looper空闲（所有消息处理完成）
    void WaitForIdle() {
      std::unique_lock<std::mutex> lock(queue_mutex_);
      idle_condition_.wait(lock, [this]() {
        return message_queue_.empty() && active_tasks_ == 0;
      });
    }

    void PostMessage(int what, std::function<void()> task) {
      PostMessageDelayed(what, std::move(task), 0);
    }

    void PostMessageDelayed(int what, std::function<void()> task, T_LONG delay_ms) {
      if (!running_ && !is_main_looper_) {
        throw std::runtime_error("PostMessageDelayed called on stopped looper");
      }

      LooperMessage msg;
      msg.what = what;
      msg.task = std::move(task);
      msg.execute_time = GetCurrentTimeMillis() + delay_ms;

      {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        message_queue_.push(std::move(msg));
        active_tasks_++;
      }
      queue_condition_.notify_one();
    }

    /// Looper Run之前预先执行的逻辑
    void PreScheduleMessage(int what, std::function<void()> task) {
      PreScheduleMessageDelayed(what, std::move(task), 0);
    }

    void PreScheduleMessageDelayed(int what, std::function<void()> task, T_LONG delay_ms) {
      if (running_) {
        throw std::runtime_error("PreScheduleMessage called on running looper");
      }

      LooperMessage msg;
      msg.what = what;
      msg.task = std::move(task);
      msg.execute_time = GetCurrentTimeMillis() + delay_ms;

      {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        pre_scheduled_messages_.push_back(std::move(msg));
      }
    }

    bool IsRunning() const { return running_; }
    bool IsQuitRequested() const { return quit_requested_; }

    T_STRING GetTieClassName() override {
      return _TSTR("CTieMessageLooper");
    }

  private:
    std::atomic_bool running_{false};
    std::atomic_bool quit_requested_{false};
    std::atomic_bool is_main_looper_{false};
    std::atomic_uint active_tasks_{0};
    std::thread looper_thread_;
    std::queue<LooperMessage> message_queue_;
    std::vector<LooperMessage> pre_scheduled_messages_;
    std::mutex queue_mutex_;
    std::condition_variable queue_condition_;
    std::condition_variable idle_condition_;

    static T_LONG GetCurrentTimeMillis() {
      auto now = std::chrono::system_clock::now();
      return std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
    }

    void MessageLoop() {
      // 处理预先Post要处理的消息
      {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        for (auto& msg : pre_scheduled_messages_) {
          message_queue_.push(std::move(msg));
          active_tasks_++;
        }
        pre_scheduled_messages_.clear();
      }

      while (running_ && !quit_requested_) {
        LooperMessage msg;
        bool has_message = false;

        {
          std::unique_lock<std::mutex> lock(queue_mutex_);

          // 等待条件：有消息到达或退出请求
          if (message_queue_.empty()) {
            queue_condition_.wait(lock, [this]() {
              return !message_queue_.empty() || quit_requested_;
            });
          }

          // 检查退出请求
          if (quit_requested_) {
            break;
          }

          if (!message_queue_.empty()) {
            auto& front_msg = message_queue_.front();
            T_LONG current_time = GetCurrentTimeMillis();

            if (front_msg.execute_time <= current_time) {
              msg = std::move(front_msg);
              message_queue_.pop();
              has_message = true;
            } else {
              auto wait_time = std::chrono::milliseconds(
                front_msg.execute_time - current_time);
              queue_condition_.wait_for(lock, wait_time);
            }
          }
        }

        if (has_message && msg.task) {
          // 执行task
          msg.task();

          // 更新active task计数
          {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            active_tasks_--;
            if (active_tasks_ == 0 && message_queue_.empty()) {
              idle_condition_.notify_all();
            }
          }
        }
      }

      // 清理剩余消息（只有调用Stop会处理剩下的消息，调用Quit不会）
      if (!quit_requested_) {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        while (!message_queue_.empty()) {
          auto& msg = message_queue_.front();
          if (msg.task) {
            msg.task();
          }
          message_queue_.pop();
        }
      }

      active_tasks_ = 0;
      idle_condition_.notify_all();
      running_ = false;
    }
  };

  inline CTieMessageLooper& GetMainLooper() {
    static CTieMessageLooper message_looper;
    return message_looper;
  }

  inline void Sleep(T_LONG millis) {
    std::this_thread::sleep_for(std::chrono::milliseconds(millis));
  }
}

#endif  // TIE_CXX_THREAD_H
