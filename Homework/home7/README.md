# README

## Условия 
ДЗ-7 ( от 14.10 срок сдачи до 28.10)
 
В аргументах командной строки программе задаётся имя бинарного файла, хранящего двоичное дерево поиска. 
Файл может отсутствовать или быть пустым, либо содержать единственную (пустую) запись. 
 
Бинарное дерево поиска в файле устроено следующим образом. Каждая запись занимает 4
слова (по 32 бита каждое слово). Первое слово — это ключ, второе слово — это значение,
третье слово — это номер записи, соответствующей вершине левого поддерева, и четвёртое
слово — это номер записи, соответствующей вершине правого поддерева. Нулевая запись
в файле не используется, и зарезервирована для обозначения «нулевого указателя». Корень
всего дерева находится в первой записи.
 
1. На стандартном потоке ввода до признака конца файла задаётся последовательность
пар <ключ,значение>. Элементы пары отделяются друг от друга пробельными символа-
ми. Необходимо для каждой пары выполнить добавление заданного значения в дерево. Если
ключ уже существует, новое значение заменяет старое.
 
Ввод можно перенаправить из файла, в котором записаны пары чисел:
./a.out filetree <file_key_v
 
2. Написать функцию вывода дерева из файла на экран, выводить в отсортированном порядке.