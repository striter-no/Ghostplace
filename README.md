# GHOST PLACE

Проект по созданию клона Web в терминале. Пока что запуск и компиляция возможны только на ОС Linux

## Как пользоваться проектом

## Инструкция по сборке

Вам надо установить (если у вас отстутствует) библиотеку [utf8proc](https://github.com/JuliaStrings/utf8proc). 
Можно скачать последнюю версию релиза и распоковав архив прописать

```sh
make
make install
```

Далее вы можете собрать клиент и сервер GhostPlace:

1. Сервер (или же роутер)

```sh
./scripts/rbuild router gcc-14 # Или любой другой GCC компилятор
./scripts/run router # Для запуска
```

2. Клиент (TUI программа + взаимодействие с сервером)

```sh
./scripts/rbuild client gcc-14 # Или любой другой GCC компилятор
./scripts/run client # Для запуска
```

Также для добавления новых сайтов предусмотрен скрипт `upload_site.py`. Для запуска:

```sh
python -m venv venv
source ./venv/bin/activate
pip install "flask[async]"

python ./upload_site.py
```

Для использования добавления нового сайта:

```sh
tar -cf ./ghost.main ./ghost.main # Обязательно папка
curl -X POST -F "file=@ghost.main.tar" http://localhost:5000/upload # URL для примера
```

> Скрипт на момент релиза не работает для запущенного сервера