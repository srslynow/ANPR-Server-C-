#include <mutex>
#include <condition_variable>

class Semaphore {
public:
	Semaphore(int countt)
	{
		count = countt;
	}

	inline void notify()
	{
		std::unique_lock<std::mutex> lock(mtx);
		count++;
		cv.notify_one();
	}

	inline void wait()
	{
		std::unique_lock<std::mutex> lock(mtx);

		while (count == 0){
			cv.wait(lock);
		}
		count--;
	}

	inline bool tryAcquire()
	{
		std::unique_lock<std::mutex> lock(mtx);

		if (count == 0)
		{
			return false;
		}
		else
		{
			count--;
			return true;
		}
	}

private:
	std::mutex mtx;
	std::condition_variable cv;
	int count;
};