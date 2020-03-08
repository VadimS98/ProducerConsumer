#include <iostream>
#include <thread>
#include <functional>
#include <chrono>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <future>

using namespace std;


mutex linemtx;
condition_variable dowork;

static const uint8_t ThreadAmount = 10;

class Request {
private:
	string request;
public:
	Request() : request("empty") {};
	void SetRequest() {
		request = "request";
	}
};

class Stopper {
private:
	bool stopId;
public:
	Stopper() : stopId(0) {};
	Stopper(bool new_stopId) : stopId(new_stopId) {};
	void SetStopId() {
		stopId = 1;
	}
};

Request* GetRequest(Stopper stopSignal);
void ProcessRequest(Request* request, Stopper stopSignal);

class Task {
private:
	Request* request;
	Stopper stopSignal;
public:
	Task(Request* nrequest, Stopper nstopSignal) : request(nrequest), stopSignal(nstopSignal){}
	void exe() {
		ProcessRequest(request, stopSignal);
		Delete();
	}
	void Delete() {
		delete[] request;
	}
};

class ThreadPool {
private:
	vector<thread> Pool;
	queue<Task> TaskQueue;
	Stopper stop;

public:
	void Detach() {
		for (int i = 0; i < ThreadAmount; ++i) {
			Pool[i].detach();
		}
	}

	void SetState(bool state) {
		stop = state;
	}

	void AddInQueue(Request* req, Stopper stop) {
		Task t(req, stop);
		TaskQueue.push(t);
		dowork.notify_one();
	}

	ThreadPool() {
		for (uint8_t i = 0; i < ThreadAmount; ++i) {
			Pool.emplace_back(thread(&ThreadPool::InfiniteLoop, this));
			//Pool.back().detach();
		}
	}

	void InfiniteLoop() {
		while (true) {
			{
				unique_lock<mutex> lock(linemtx);
				dowork.wait(lock);
				TaskQueue.front().exe();
			}
		}
	};
};

Request* GetRequest(Stopper stopSignal) {
	return 0;
};
void ProcessRequest(Request* request, Stopper stopSignal) {};

int main() {
	ThreadPool pool;
	auto start = chrono::high_resolution_clock::now() + chrono::seconds(5);
	Stopper stopSignal = false;

	while (chrono::high_resolution_clock::now() < start) {
		Request* request = GetRequest(stopSignal);

		if (request == NULL) {} // Add

		pool.AddInQueue(request, stopSignal);

	}
	pool.Detach();
}
