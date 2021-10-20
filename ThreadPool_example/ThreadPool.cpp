#include "ThreadPool.h"
#include <iostream>
using namespace std;


void ThreadPool::WorkerThread()
{
	while (true)
	{
		unique_lock<mutex>lock(m_job_q_);
		cv_job_q_.wait(lock, [this]() {return !this->jobs_.empty() || stop_all; });//람다식 사용(호출되거나 전달되는 위치에서 바로 함수 정의)
		if (stop_all && this->jobs_.empty())
			return;

		function<void()>job = move(jobs_.front());
		jobs_.pop();//맨앞의 job 제거, 오래된 큐부터 작업 처리.
		lock.unlock();

		//해당 job수행
		job();
	}
}

ThreadPool::ThreadPool(size_t num_threads)
	:num_threads_(num_threads),stop_all(false)
{
	worker_threads_.reserve(num_threads_);
	for (size_t i = 0; i < num_threads_; ++i)
	{
		worker_threads_.emplace_back([this]() {this->WorkerThread(); });//람다식 사용(호출되거나 전달되는 위치에서 바로 함수 정의)
	}
}



void ThreadPool::EnqueueJob(std::function<void()>job)// 작업추가
{
	if (stop_all)//이미 작업을 멈춘 상태라면 실행하면 안됨
	{
		throw runtime_error("ThreadPool 사용 중지됨");
	}
	{
		lock_guard<mutex>lock(m_job_q_);
		jobs_.push(move(job));
	}
	cv_job_q_.notify_one();
}

ThreadPool::~ThreadPool()
{
	stop_all = true;
	cv_job_q_.notify_all();

	for (auto& t : worker_threads_)
	{
		t.join();
	}
}

void work(int t, int id)
{
	cout << id << " start " << endl;
	this_thread::sleep_for(chrono::seconds(t));
	cout << id << " end after " << t << endl;
}




int main()
{

	ThreadPool pool(3);
	int n = 0;
	for (int i = 0; i < 10; i++)
	{
		n++;
		pool.EnqueueJob([i]() {work(i % 3 + 1, i); });//이 함수 동작을 큐에 삽입.
	}

	return 0;
}