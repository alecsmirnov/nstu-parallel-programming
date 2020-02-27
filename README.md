# Лабораторные работы по дисциплине "Параллельное программирование" на факультете ПМИ, НГТУ
&nbsp;  

## 1. Программирование независимых потоков
### Условия задачи

Вводная часть:  
1) Доработать программу из примера в методическом указании.
2) Оценить стоимость запуска одного потока операционной системой. Изменяя количество операций (можно использовать любую арифметическую 
операцию), которые исполняет функция потока, определить такое их количество, чтобы порождение потока было оправданным.
3) Разработать программу, которая обеспечивает параллельное применение заданной функции к каждому элементу массива. Размер массива, 
применяемая функция и количество потоков задаются динамически.

Задание для самостоятельной работы: 
Создать упрощенный HTTP-сервер, отвечающий на любой запрос клиента (например, браузера) строкой «Request number <номер запроса> 
has been processed», где под номером запроса понимается порядковый номер, присвоенный запросу сервером. Нумерация начинается с единицы.
1) Доработать однопоточную версию сервера, доступную по адресу: http://rosettacode.org/wiki/Hello_world/Web_server#C. Обработка 
каждого запроса выполняется в отдельном потоке: при получении запроса создается новый поток для его обработки, после отправки результата 
клиенту поток завершает свою работу. Соединение с клиентом закрывается сразу после обработки запроса.
2) Оценить производительность сервера с помощью утилиты ab4, входящей в комплект поставки веб-сервера Apache.
3) Оценить максимальное количество потоков, с которым может работать сервер, для различных размеров стека по умолчанию (2 Мбайт, 1 Мбайт, 512 Кбайт).
4) Добавить в обработчик запроса от клиента запуск простейшего PHP-скрипта, возвращающего версию PHP (\<?php echo phpversion();?\>). 
Вернуть номер версии клиенту. Оценить изменение производительности сервера с помощью утилиты ab.
