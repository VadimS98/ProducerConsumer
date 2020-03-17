#include <iostream>
#include <thread>
#include <functional>
#include <chrono>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <future>
#include <vector>
#include <stdlib.h>

using namespace std;

class Request {
public:
	Request() : a(0) {}
	Request(int new_a) : a(new_a) {}

	int GetA() {
		return a;
	}
	void SetA(int na) {
		a = na;
	}
private:
	int a;
};


class Stopper {
public:
	Stopper() : stopSignal(false) {}
	void setStopper(bool new_stopper) {
		stopSignal = new_stopper;
	}
	bool getStopper() {
		return stopSignal;
	}
private:
	bool stopSignal;
};

class ThreadPool {
public:
	using Task = function<void()>;

	explicit ThreadPool(size_t amThreads) {
		start(amThreads);
	}

	~ThreadPool() {
		stop();
	}

	void enqueue(Task task) {
		{
			unique_lock<mutex> lock{ eventmutex };
			Tasks.emplace(move(task));
		}

		event.notify_one();
	}

private:
	vector<thread> Pool;
	condition_variable event;
	mutex eventmutex;
	queue<Task> Tasks;
	Stopper stopSignal;

	void start(size_t amThreads) {
		for (auto i = 0u; i < amThreads; ++i) {
			Pool.emplace_back([=] {
				while (true) {
					Task task;
					{
						unique_lock<mutex> lock{ eventmutex };
						event.wait(lock, [=] { return stopSignal.getStopper() || !Tasks.empty(); });
						if (stopSignal.getStopper()) {
							break;
						}

						task = move(Tasks.front());
						Tasks.pop();
					}
					task();
				}

				});
		}
	}

	void stop() noexcept {
		{
			unique_lock<mutex> lock{ eventmutex };
			stopSignal.setStopper(true);;
		}
		event.notify_all();

		for (auto& thread : Pool) {
			thread.join();
		}
	}
};

	mutex mm;
void ProcessRequest(Request* request, Stopper stopSignal) {
	this_thread::sleep_for(chrono::milliseconds(500));
	mm.lock();
	cout << request->GetA() << endl;
	mm.unlock();
}

Request r;

Request* GetRequest() {
	r.SetA(rand() % 100 + 1);
	return &r;
}

int main() {
	using namespace std::placeholders;
	ThreadPool pool{ 10 };
	Request *request;
	Stopper stopSignal;
	auto start = chrono::high_resolution_clock::now() + chrono::seconds(30);

	while (chrono::high_resolution_clock::now() < start) {
		request = GetRequest();
		auto f1 = bind(ProcessRequest, request, stopSignal);
		pool.enqueue(f1);		
	}
	return 0;
}
