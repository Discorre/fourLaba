# Компилятор
CXX = g++

# Флаги компиляции
CXXFLAGS = -std=c++20 -Wall -Wextra -pthread

# Исходные файлы
SRCS = zad1.cpp zad2.cpp zad3.cpp

# Исполняемые файлы
EXECS = zad1.exe zad2.exe zad3.exe

# Правило по умолчанию
all: $(EXECS)

# Правило для сборки исполняемых файлов
%.exe: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

# Правила для запуска отдельных задач
run1: zad1.exe
	zad1.exe

run2: zad2.exe
	zad2.exe

run3: zad3.exe
	zad3.exe

# Правило для запуска всех задач
run_all: $(EXECS)
	@echo Запуск задачи 1:
	zad1.exe
	@echo.
	@echo Запуск задачи 2:
	zad2.exe
	@echo.
	@echo Запуск задачи 3:
	zad3.exe

# Правило для очистки
clean:
	rm -f /Q $(EXECS)

# Правило для пересборки
rebuild: clean all

# Цель для проверки синтаксиса
check: $(SRCS)
	$(CXX) $(CXXFLAGS) -fsyntax-only $(SRCS)

.PHONY: all run1 run2 run3 run_all clean rebuild check