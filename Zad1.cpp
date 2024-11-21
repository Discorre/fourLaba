#include <iostream>
#include <thread>
#include <mutex>
#include <semaphore>
#include <barrier>
#include <atomic>
#include <condition_variable>
#include <chrono>
#include <vector>
#include <random>

using namespace std;

// Примитивы синхронизации
mutex mtx;
counting_semaphore<3> semaphore(3); // Максимум 3 потока могут работать одновременно
atomic_flag spinLock = ATOMIC_FLAG_INIT; // блокировка для Спинлока (изначально False- доступен)

// Функция для генерации случайного символа из ASCII  
char generate_random_char() {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(32, 126); // символы с кодами(которые доступны на клавиатуре)
    return static_cast<char>(dis(gen));
}


//  Monitor
class Monitor {
public:
    Monitor() : flag(false) {} // конструктор,который создает доступный монитор

    // Метод для захвата монитора
    void enter() {
        unique_lock<mutex> lock(mtx); //пытаемся захватить монитор
        while (flag) {  // проверка доступен ли монитор
            cond.wait(lock);  // если монитор занят,то нахоидся в сотоянии ожидания
        }
        flag = true; // захватываем монитор
    }

    // Метод для освобождения монитора
    void exit() {
        lock_guard<mutex> lock(mtx); // захватываем мьютексом чтобы поменять флаг
        flag = false; // меняем флаг ( монитор свободен)
        cond.notify_one();  // уведомляем другой поток,что монитор свободен
    }

private:
    bool flag;  // Флаг для контроля доступа
    mutex mtx;
    condition_variable cond; //переменная для уведомления потоков
};

void test_monitor(int num_threads) {
    cout << "Тестирование Monitor:" << endl;

    Monitor monitor; // создаем монитор
    vector<thread> threads; // вектор для потоков

    auto start = chrono::high_resolution_clock::now(); // таймер начала

    // Создаем потоки
    for (int i = 0; i < num_threads; ++i) {
        threads.push_back(thread([i, &monitor]() {
            monitor.enter(); // захватываем монитор
            {
                lock_guard<mutex> lock(mtx); // защищаем вывод
                char c = generate_random_char(); // генерируем символ
                cout << "Поток " << i << ": " << c << " (Monitor)" << endl;
            }
            monitor.exit(); // освобождаем монитор
        }));
    }

    for (auto& t : threads) {
        t.join();
    }

    auto end = chrono::high_resolution_clock::now(); // таймер конца
    chrono::duration<double> duration = end - start;
    cout << "Время работы Monitor: " << duration.count() << " секунд." << endl;
    cout << "---------------------------------" << endl;
}



//  SemaphoreSlim
class SemaphoreSlim {
public:
    SemaphoreSlim(int count) : count(count) {}

    // Метод для захвата семафора
    void wait() {
        unique_lock<mutex> lock(mtx); //пытаемся захватить ресурс
        while (count == 0) { // проверка на свободные ресурсы
            cond.wait(lock);  // Ожидаем освобождения ресурса
        }
        --count;  // Уменьшаем счетчик(захватываем ресурс)
    }

    // Метод для освобождения семафора
    void release() {
        lock_guard<mutex> lock(mtx); // захыватыаем поток,Чтобы изменить счетчик
        ++count;  // Увеличиваем счетчик(освобождая ресурс)
        cond.notify_one();  // Уведомляем один поток,что ресурс свободен
    }

private:
    int count;// сколько потоков могут использовать ресурс
    mutex mtx;
    condition_variable cond;
};


// Тестирование SemaphoreSlim
void test_semaphore_slim(int num_threads) {
    cout << "Тестирование SemaphoreSlim:" << endl;

    SemaphoreSlim sem(1);
    vector<thread> threads; //вектор для потоков

    auto start = chrono::high_resolution_clock::now(); // таймер начала

    // Создаем потоки, которые будут работать с SemaphoreSlim
    for (int i = 0; i < num_threads; ++i) {
        threads.push_back(thread([i, &sem]() {
            sem.wait();  // Поток ожидает, пока не освободится ресурс
            {
                lock_guard<mutex> lock(mtx); // мьютекс для вывода
                char c = generate_random_char();
                cout << "Поток " << i << ": " << c << " (SemaphoreSlim)" << endl;
            }
            sem.release();  // Освобождаем ресурс для других потоков
        }));
    }

    // Ожидаем завершения всех потоков
    for (auto& t : threads) {
        t.join();
    }

    auto end = chrono::high_resolution_clock::now(); // таймер конца
    chrono::duration<double> duration = end - start; // длительность
    cout << "Время работы SemaphoreSlim: " << duration.count() << " секунд." << endl;
    cout << "---------------------------------" << endl;
}

void test_mutex(int num_threads) {
    cout << "Тестирование мьютекса:" << endl;

    vector<thread> threads;

    auto start = chrono::high_resolution_clock::now();

    for (int i = 0; i < num_threads; ++i) {
        threads.push_back(thread([i]() {
            lock_guard<mutex> lock(mtx);
            char c = generate_random_char();
            cout << "Поток " << i << ": " << c << " (Мьютекс)" << endl;
        }));
    }

    for (auto& t : threads) {
        t.join();
    }

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;
    cout << "Время работы мьютекса: " << duration.count() << " секунд." << endl;
    cout << "---------------------------------" << endl;
}

void test_semaphore(int num_threads) {
    cout << "Тестирование Semaphore:" << endl;

    vector<thread> threads;
    counting_semaphore<3> sem(3);

    auto start = chrono::high_resolution_clock::now();

    for (int i = 0; i < num_threads; ++i) {
        threads.push_back(thread([i, &sem]() {
            sem.acquire();
            {
                lock_guard<mutex> lock(mtx);
                char c = generate_random_char();
                cout << "Поток " << i << ": " << c << " (Semaphore)" << endl;
            }
            sem.release();
        }));
    }

    for (auto& t : threads) {
        t.join();
    }

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;
    cout << "Время работы Semaphore: " << duration.count() << " секунд." << endl;
    cout << "---------------------------------" << endl;
}

void test_spinlock(int num_threads) {
    cout << "Тестирование SpinLock:" << endl;

    vector<thread> threads;

    auto start = chrono::high_resolution_clock::now();

    for (int i = 0; i < num_threads; ++i) {
        threads.push_back(thread([i]() {
            while (spinLock.test_and_set()) { /* активно ждём */ }
            {
                lock_guard<mutex> lock(mtx);
                char c = generate_random_char();
                cout << "Поток " << i << ": " << c << " (SpinLock)" << endl;
            }
            spinLock.clear();
        }));
    }

    for (auto& t : threads) {
        t.join();
    }

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;
    cout << "Время работы SpinLock: " << duration.count() << " секунд." << endl;
    cout << "---------------------------------" << endl;
}

void test_barrier(int num_threads) {
    cout << "Тестирование Barrier:" << endl;

    vector<thread> threads;
    barrier bar(num_threads);

    auto start = chrono::high_resolution_clock::now();

    for (int i = 0; i < num_threads; ++i) {
        threads.push_back(thread([i, &bar]() {
            {
                lock_guard<mutex> lock(mtx);
                char c = generate_random_char();
                cout << "Поток " << i << ": " << c << " (до Barrier)" << endl;
            }
            bar.arrive_and_wait();
            {
                lock_guard<mutex> lock(mtx);
                char c = generate_random_char();
                cout << "Поток " << i << ": " << c << " (после Barrier)" << endl;
            }
        }));
    }

    for (auto& t : threads) {
        t.join();
    }

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;
    cout << "Время работы Barrier: " << duration.count() << " секунд." << endl;
    cout << "---------------------------------" << endl;
}

void test_spinwait(int num_threads) {
    cout << "Тестирование SpinWait:" << endl;

    vector<thread> threads;

    auto start = chrono::high_resolution_clock::now();

    for (int i = 0; i < num_threads; ++i) {
        threads.push_back(thread([i]() {
            while (spinLock.test_and_set()) {
                this_thread::yield(); // Уступаем другим потокам
            }
            {
                lock_guard<mutex> lock(mtx);
                char c = generate_random_char();
                cout << "Поток " << i << ": " << c << " (SpinWait)" << endl;
            }
            spinLock.clear();
        }));
    }

    for (auto& t : threads) {
        t.join();
    }

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;
    cout << "Время работы SpinWait: " << duration.count() << " секунд." << endl;
    cout << "---------------------------------" << endl;
}



int main() {
    //cout << "Запуск потоков, генерирующих случайные символы:" << endl;

    const int num_threads = 8; // количество потоков
    vector<thread> threads; // вектор для потоков

    //cout << "Завершение работы всех потоков." << endl;
    //cout << "---------------------------------" << endl;

    // Тестируем примитивы
    test_mutex(num_threads);
    test_semaphore(num_threads);
    test_spinlock(num_threads);
    test_barrier(num_threads);
    test_spinwait(num_threads);
    test_monitor(num_threads);
    test_semaphore_slim(num_threads);

    return 0;
}