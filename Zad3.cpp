#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>

using namespace std;

const int num_philosophers = 5; // Количество философов

mutex forks[num_philosophers]; // Массив мьютексов, представляющих вилки
mutex output_mutex; // Мьютекс для синхронизации вывода в консоль

// Функция, представляющая действия философа
void philosopher(int id) {
    while (true) {
        // Философ размышляет
        {
            lock_guard<mutex> lock(output_mutex); // Захват мьютекса для безопасного вывода
            cout << "Философ " << id << " размышляет." << endl;
        }

        // Выбор вилок: сначала берется вилка с меньшим номером, затем с большим
        mutex& left_fork = forks[min(id, (id + 1) % num_philosophers)];
        mutex& right_fork = forks[max(id, (id + 1) % num_philosophers)];

        // Захват обеих вилок
        lock(left_fork, right_fork); // Захват обеих вилок одновременно, чтобы избежать deadlock
        lock_guard<mutex> lg1(left_fork, adopt_lock); // Фиксация первой вилки
        lock_guard<mutex> lg2(right_fork, adopt_lock); // Фиксация второй вилки

        // Философ ест
        {
            lock_guard<mutex> lock(output_mutex); // Захват мьютекса для вывода состояния
            cout << "Философ " << id << " ест." << endl;
        }
        this_thread::sleep_for(chrono::milliseconds(5000)); // Имитация времени на прием пищи

        // Освобождение вилок происходит автоматически при выходе из области видимости блоков lock_guard
        {
            lock_guard<mutex> lock(output_mutex); // Захват мьютекса для вывода состояния
            cout << "Философ " << id << " освободил вилки." << endl;
        }
    }
}

int main() {
    system("chcp 65001"); // Установка кодировки для поддержки русских символов в консоли

    vector<thread*> philosopher_threads; // Вектор указателей на потоки для философов

    // Создание потоков для каждого философа и добавление их в вектор
    for (int i = 0; i < num_philosophers; ++i) {
        philosopher_threads.push_back(new thread(philosopher, i));
    }

    // Ожидание завершения работы всех потоков
    for (auto& t : philosopher_threads) {
        t->join(); // Ожидание завершения потока
        delete t; // Освобождение памяти
    }

    return 0;
}
