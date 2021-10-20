#include "ThreadPool.h"
#include <iostream>
using namespace std;


void ThreadPool::WorkerThread()
{
	while (true)
	{
		unique_lock<mutex>lock(m_job_q_);
		cv_job_q_.wait(lock, [this]() {return !this->jobs_.empty() || stop_all; });//���ٽ� ���(ȣ��ǰų� ���޵Ǵ� ��ġ���� �ٷ� �Լ� ����)
		if (stop_all && this->jobs_.empty())
			return;

		function<void()>job = move(jobs_.front());
		jobs_.pop();//�Ǿ��� job ����, ������ ť���� �۾� ó��.
		lock.unlock();

		//�ش� job����
		job();
	}
}

ThreadPool::ThreadPool(size_t num_threads)
	:num_threads_(num_threads),stop_all(false)
{
	worker_threads_.reserve(num_threads_);
	for (size_t i = 0; i < num_threads_; ++i)
	{
		worker_threads_.emplace_back([this]() {this->WorkerThread(); });//���ٽ� ���(ȣ��ǰų� ���޵Ǵ� ��ġ���� �ٷ� �Լ� ����)
	}
}



void ThreadPool::EnqueueJob(std::function<void()>job)// �۾��߰�
{
	if (stop_all)//�̹� �۾��� ���� ���¶�� �����ϸ� �ȵ�
	{
		throw runtime_error("ThreadPool ��� ������");
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
		pool.EnqueueJob([i]() {work(i % 3 + 1, i); });//�� �Լ� ������ ť�� ����.
	}

	return 0;
}