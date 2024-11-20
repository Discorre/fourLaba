import random

# Данные о товарах
products = [
    ("Яблоко", 1.0), ("Банан", 0.5), ("Апельсин", 1.5), ("Киви", 2.0), 
    ("Виноград", 3.0), ("Персик", 1.2), ("Лимон", 1.2), ("Груша", 1.1), 
    ("Арбуз", 5.0), ("Дыня", 4.5), ("Слива", 2.0), ("Гранат", 3.0),
    ("Малина", 3.5), ("Черника", 4.0), ("Клубника", 2.5), ("Грейпфрут", 1.8),
    ("Манго", 2.8), ("Фейхоа", 3.2), ("Персик", 1.6), ("Апельсин", 2.1)
]

# Функция для генерации случайных товаров для чека
def generate_receipt(receipt_id):
    num_items = random.randint(1, 5)  # Чек будет содержать от 1 до 5 товаров
    receipt_data = []
    for _ in range(num_items):
        product, price = random.choice(products)  # Выбираем случайный товар
        quantity = random.randint(1, 10)  # Случайное количество товара
        receipt_data.append((product, price, quantity))
    return receipt_id, receipt_data

# Генерация 1000 чеков
with open('receiptsmini.txt', 'w', encoding='utf-8') as file:
    for receipt_id in range(1, 101):
        receipt_id, items = generate_receipt(receipt_id)
        receipt_line = f"{receipt_id} "
        # Формируем строку для одного чека
        receipt_line += ", ".join([f"{product} {price} {quantity}" for product, price, quantity in items])
        file.write(receipt_line + "\n")

print("Файл receipts.txt с 1000 чеками успешно создан!")
