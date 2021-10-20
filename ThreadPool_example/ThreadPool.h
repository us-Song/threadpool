#pragma once
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool
{
public:
	ThreadPool(size_t num_threads);
	~ThreadPool();

	void EnqueueJob(std::function<void()>job);

private:
	size_t num_threads_;//개수==worker_threads_.size()
	std::vector<std::thread>worker_threads_;//스레드 보관 컨테이너
	std::queue<std::function<void()>>jobs_;// 스레드풀 사용을 원하는 함수 보관하는 컨테이너
	std::condition_variable cv_job_q_;
	std::mutex m_job_q_;

	bool stop_all;

	void WorkerThread();

};

