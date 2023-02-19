#include <conio.h>
struct SharedResource
{
	SharedResource() :
		cv(), mtx(), killflag(false)
	{

	}
	//These are resources that are shared between threads
	
	std::condition_variable cv;
	std::mutex mtx;
	bool killflag;
};
class A
{
public:
	A() = delete;
	~A()
	{
		delete[] this->buffer;
		delete this->name;
	}
	A& operator=(const A& object) //Deep assignment operator
	{
		if (!(this == &object))
		{
			
			//name

			this->name = new char[strlen(object.name) + 1]();
			strncpy_s(this->name, strlen(object.name) + 1, object.name, strlen(object.name) + 1);

			//buffer
			this->buffer = new int[4]{};
			for (int i = 0; i < 4; ++i)
			{
				this->buffer[i] = object.buffer[i];
			}
		}
		return *this;
	}
	A(const A& object) //Deep copy constructor
	{
		//name

		this->name = new char[strlen(object.name)+1]();
		strncpy_s(this->name, strlen(object.name)+1, object.name, strlen(object.name)+1);

		//buffer
		this->buffer = new int[4]{};
		for (int i = 0; i < 4; ++i)
		{
			this->buffer[i] = object.buffer[i];
		}
		
	}
	A(A&& object) //Move constructor
	{
		
		this->buffer = object.buffer;
		this->name = object.name;
		object.buffer = nullptr;
		object.name = nullptr;
	}
	A& operator=(A&& object) //Move assignment operator
	{
		if (this != &object)
		{

			this->buffer = object.buffer;
			this->name = object.name;
			object.buffer = nullptr;
			object.name = nullptr;
		}
		return *this;
	}
	A(const char* const name_) //Special constructor

	{
		this->name = new char[strlen(name_)+1];
		strncpy_s(this->name, strlen(name_)+1, name_, strlen(name_)+1);
		buffer = new int[4]{1,2,3,4};
	}


	void operator() (SharedResource& sharedResource)
	{
		size_t index{ 0 };
		while (true)
		{
			std::unique_lock<std::mutex> lock(sharedResource.mtx);
			if (sharedResource.cv.wait_for(lock, std::chrono::microseconds(1), [&]() {return sharedResource.killflag; }))
				break;

			/*
			Once notified or once rel_time has passed, the function unblocks and calls lck.lock(),
			leaving lck in the same state as when the function was called.
			Then the function returns (notice that this last mutex locking may block again the thread before returning).
			*/
			//So all the following codes are protected!
			std::cout << "\nbuffer: " << "is " << " important " << this->buffer[index] << "\n";
			lock.unlock();
			//The following are all local variables and don't have to be protected!
			++index;
			if (index >= 4)
			{
				index = 0;
			}

		}
	}
private:
	char* name;
	int* buffer;

};

int main()
{

	A a("A");

	//Resources shared between threads 
	SharedResource sharedResource;

	//Spawn threads
	
	/*
	
	 it's possible to move an object into a std::thread instead of making a copy. 
	 When an object is moved into a std::thread, the move constructor of the object is called instead of the copy constructor.
	
	*/
	std::thread thA(std::move(a), std::ref(sharedResource));


	thA.detach();
	std::this_thread::sleep_for(1ms);
	while (true)
	{

		std::unique_lock<std::mutex> lock(sharedResource.mtx);
		std::cout << "Hi " << "How are you! " << "I love you!" << " wow!" << " \n";
		std::this_thread::sleep_for(0.01ms);
	}
	//Key Press
	_getch();

	//Signal to the threads
	{


		std::lock_guard<std::mutex> lock(sharedResource.mtx);
		sharedResource.killflag = true;
	}
	sharedResource.cv.notify_one();
	std::this_thread::sleep_for(1s); //This is going to get fixed in future versions. 

}