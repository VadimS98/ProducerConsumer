#include <iostream>
#include <thread>
#include <functional>
#include <chrono>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <future>
#include <vector>

using namespace std;

class Request {
public:
	Request() : a(0) {};
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

void ProcessRequest(Request* request, Stopper stopSignal) {

}

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
	Stopper stopSignal{};
	Request request;
	//_Binder<std::_Unforced, void (*)(Request * request, Stopper stopSignal), const std::_Ph<1>&, const std::_Ph<2>&>  f1 = bind(&ProcessRequest, placeholders::_1, placeholders::_2);
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

void add(Request* request, Stopper stopSignal)
{
	//return 5;
}



int main() {
	ThreadPool{ 10 };
	/*auto a = 5;
	auto f1 = bind(&ProcessRequest, placeholders::_1, placeholders::_2);
	auto add_func = bind(&add, placeholders::_1, placeholders::_2);*/
	return 0;
}
