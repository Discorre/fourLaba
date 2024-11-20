#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <unordered_map>
#include <mutex>
#include <fstream>
#include <sstream>

// Структура, описывающая товар
struct Product {
    std::string name; // Название товара
    double price;     // Цена товара
    int quantity;     // Количество товара
};

// Структура, описывающая чек
struct Receipt {
    int id;                    // Уникальный идентификатор чека
    std::vector<Product> products; // Список товаров в чеке
};

// Класс для обработки данных о покупках
class SalesProcessor {
private:
    std::vector<Receipt> receipts; // Список всех чеков
    std::unordered_map<std::string, int> totalProductQuantity; // Суммарное количество проданных товаров по названию
    std::unordered_map<std::string, std::vector<std::pair<int, double>>> productReceipts; // Карта товаров и чеков, в которых они присутствуют
    std::mutex mtx; // Мьютекс для синхронизации доступа к общим данным в многопоточном режиме

public:
    // Конструктор, инициализирующий объект списком чеков
    SalesProcessor(const std::vector<Receipt>& recs) : receipts(recs) {}

    // Однопоточная обработка данных
    void processSingleThread() {
        for (const auto& receipt : receipts) {
            processReceipt(receipt); // Обрабатываем каждый чек по порядку
        }
        std::cout << std::endl;
        printResults(); // Выводим результаты
    }

    // Многопоточная обработка данных
    void processMultiThread(int numThreads) {
        std::vector<std::thread> threads; // Вектор потоков
        int chunkSize = receipts.size() / numThreads; // Размер блока чеков, обрабатываемого каждым потоком

        // Локальные контейнеры для хранения данных каждого потока
        std::vector<std::unordered_map<std::string, int>> localProductQuantities(numThreads);
        std::vector<std::unordered_map<std::string, std::vector<std::pair<int, double>>>> localProductReceipts(numThreads);

        // Создаем потоки
        for (int i = 0; i < numThreads; ++i) {
            int start = i * chunkSize; // Начальный индекс блока
            int end = (i == numThreads - 1) ? receipts.size() : start + chunkSize; // Конечный индекс блока

            threads.emplace_back([this, start, end, &localProductQuantities, &localProductReceipts, i]() {
                // Обработка блоков чеков каждым потоком
                for (int j = start; j < end; ++j) {
                    processReceipt(receipts[j], localProductQuantities[i], localProductReceipts[i]);
                }
            });
        }

        // Ожидание завершения всех потоков
        for (auto& t : threads) {
            t.join();
        }

        // Слияние локальных данных в общий результат
        for (const auto& localQuantity : localProductQuantities) {
            for (const auto& [product, quantity] : localQuantity) {
                totalProductQuantity[product] += quantity; // Суммируем количество проданных товаров
            }
        }

        for (const auto& localReceipts : localProductReceipts) {
            for (const auto& [product, receipts] : localReceipts) {
                productReceipts[product].insert(productReceipts[product].end(), receipts.begin(), receipts.end()); // Объединяем данные о чеках
            }
        }
        std::cout << std::endl;
        printResults();
    }

    // Функция для обработки одного чека (однопоточная версия)
    void processReceipt(const Receipt& receipt) {
        for (const auto& product : receipt.products) {
            std::lock_guard<std::mutex> lock(mtx); // Блокируем доступ для синхронизации
            totalProductQuantity[product.name] += product.quantity; // Увеличиваем общее количество проданных товаров
            productReceipts[product.name].emplace_back(receipt.id, product.price); // Добавляем информацию о чеке
        }
    }

    // Перегруженная функция для обработки одного чека (многопоточная версия)
    void processReceipt(const Receipt& receipt, std::unordered_map<std::string, int>& localQuantity, std::unordered_map<std::string, std::vector<std::pair<int, double>>>& localReceipts) {
        for (const auto& product : receipt.products) {
            localQuantity[product.name] += product.quantity; // Увеличиваем количество проданных товаров в локальном контейнере
            localReceipts[product.name].emplace_back(receipt.id, product.price); // Добавляем информацию о чеке в локальный контейнер
        }
    }

    // Вывод результатов обработки на экран
    void printResults() const {
        for (const auto& [product, quantity] : totalProductQuantity) {
            std::cout << "Товар: " << product << ", Общее количество продано: " << quantity << "\n";
            std::cout << "Чеки, содержащие " << product << ":\n";
            for (const auto& [receiptId, price] : productReceipts.at(product)) {
                std::cout << " - Номер чека: " << receiptId << ", Цена: " << price << "\n";
            }
        }
    }
};

// Функция для загрузки данных о чеках из файла
std::vector<Receipt> loadReceiptsFromFile(const std::string& filename) {
    std::ifstream file(filename); // Открываем файл
    std::string line;
    std::unordered_map<int, Receipt> receiptsMap; // Используем временную карту для объединения данных по идентификаторам чеков

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        int id;
        std::string name;
        double price;
        int quantity;

        // Читаем строку с данными
        if (!(iss >> id >> name >> price >> quantity)) {
            std::cerr << "Ошибка при чтении строки: " << line << "\n"; // Если ошибка, выводим сообщение и пропускаем строку
            continue;
        }

        // Добавляем данные в соответствующий чек
        receiptsMap[id].id = id;
        receiptsMap[id].products.push_back({name, price, quantity});
    }

    // Переносим данные из карты в вектор
    std::vector<Receipt> receipts;
    for (auto& [_, receipt] : receiptsMap) {
        receipts.push_back(std::move(receipt));
    }

    return receipts;
}

int main() {

    // Загружаем данные о чеках из файла
    std::vector<Receipt> receipts = loadReceiptsFromFile("receiptsUltraMini.txt");

    SalesProcessor processor(receipts);

    // Измерение времени для однопоточной обработки
    auto start = std::chrono::high_resolution_clock::now();
    processor.processSingleThread();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> singleThreadDuration = end - start;

    // Вывод времени однопоточной обработки
    std::cout << "Время однопоточной обработки: " << singleThreadDuration.count() << " секунд\n";

    // Обнуление результатов для многопоточной обработки
    SalesProcessor multiThreadProcessor(receipts);

    // Измерение времени для многопоточной обработки
    int numThreads = 4; // Количество потоков
    start = std::chrono::high_resolution_clock::now();
    multiThreadProcessor.processMultiThread(numThreads);
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> multiThreadDuration = end - start;

    // Вывод времени многопоточной обработки
    std::cout << "Время многопоточной обработки: " << multiThreadDuration.count() << " секунд\n";

    return 0;
}
