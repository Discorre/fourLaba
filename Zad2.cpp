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
    std::string name;
    double price;
    int quantity;
};

// Структура, описывающая чек
struct Receipt {
    int id;
    std::vector<Product> products;
};

// Класс для обработки данных о покупках
class SalesProcessor {
private:
    std::vector<Receipt> receipts;
    std::unordered_map<std::string, int> totalProductQuantity;
    std::unordered_map<std::string, std::vector<std::pair<int, double>>> productReceipts;
    std::mutex mtx;

public:
    // Конструктор принимает список чеков
    SalesProcessor(const std::vector<Receipt>& recs) : receipts(recs) {}

    // Однопоточная обработка данных
    void processSingleThread() {
        for (const auto& receipt : receipts) {
            processReceipt(receipt);
        }
    }

    // Многопоточная обработка данных
    void processMultiThread(int numThreads) {
        std::vector<std::thread> threads;
        int chunkSize = receipts.size() / numThreads;

        std::vector<std::unordered_map<std::string, int>> localProductQuantities(numThreads);
        std::vector<std::unordered_map<std::string, std::vector<std::pair<int, double>>>> localProductReceipts(numThreads);

        for (int i = 0; i < numThreads; ++i) {
            int start = i * chunkSize;
            int end = (i == numThreads - 1) ? receipts.size() : start + chunkSize;

            threads.emplace_back([this, start, end, &localProductQuantities, &localProductReceipts, i]() {
                for (int j = start; j < end; ++j) {
                    processReceipt(receipts[j], localProductQuantities[i], localProductReceipts[i]);
                }
            });
        }

        for (auto& t : threads) {
            t.join();
        }

        // Объединяем локальные результаты
        for (const auto& localQuantity : localProductQuantities) {
            for (const auto& [product, quantity] : localQuantity) {
                totalProductQuantity[product] += quantity;
            }
        }

        for (const auto& localReceipts : localProductReceipts) {
            for (const auto& [product, receipts] : localReceipts) {
                productReceipts[product].insert(productReceipts[product].end(), receipts.begin(), receipts.end());
            }
        }
    }

    // Функция для обработки чека
    void processReceipt(const Receipt& receipt) {
        for (const auto& product : receipt.products) {
            std::lock_guard<std::mutex> lock(mtx);
            totalProductQuantity[product.name] += product.quantity;
            productReceipts[product.name].emplace_back(receipt.id, product.price);
        }
    }

    // Перегруженная функция для обработки чека с локальными данными
    void processReceipt(const Receipt& receipt, std::unordered_map<std::string, int>& localQuantity, std::unordered_map<std::string, std::vector<std::pair<int, double>>>& localReceipts) {
        for (const auto& product : receipt.products) {
            localQuantity[product.name] += product.quantity;
            localReceipts[product.name].emplace_back(receipt.id, product.price);
        }
    }

    // Вывод результатов обработки
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
    std::ifstream file(filename);
    std::string line;
    std::unordered_map<int, Receipt> receiptsMap;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        int id;
        std::string name;
        double price;
        int quantity;
        
        if (!(iss >> id >> name >> price >> quantity)) {
            std::cerr << "Ошибка при чтении строки: " << line << "\n";
            continue;
        }

        receiptsMap[id].id = id;
        receiptsMap[id].products.push_back({name, price, quantity});
    }

    std::vector<Receipt> receipts;
    for (auto& [_, receipt] : receiptsMap) {
        receipts.push_back(std::move(receipt));
    }

    return receipts;
}

int main() {
    system("chcp 65001");

    // Загружаем данные о чеках из файла
    std::vector<Receipt> receipts = loadReceiptsFromFile("receipts.txt");

    SalesProcessor processor(receipts);

    // Измерение времени для однопоточной обработки
    auto start = std::chrono::high_resolution_clock::now();
    processor.processSingleThread();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> singleThreadDuration = end - start;

    // Вывод результатов однопоточной обработки
    std::cout << "Время однопоточной обработки: " << singleThreadDuration.count() << " секунд\n";
    //processor.printResults();

    // Обнуление результатов для многопоточной обработки
    SalesProcessor multiThreadProcessor(receipts);

    // Измерение времени для многопоточной обработки
    int numThreads = 8;
    start = std::chrono::high_resolution_clock::now();
    multiThreadProcessor.processMultiThread(numThreads);
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> multiThreadDuration = end - start;

    // Вывод результатов многопоточной обработки
    std::cout << "Время многопоточной обработки: " << multiThreadDuration.count() << " секунд\n";
    //multiThreadProcessor.printResults();

    return 0;
}