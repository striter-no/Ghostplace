# GHOST PLACE

Проект по созданию клона Web в терминале. Пока что запуск и компиляция возможны только на ОС Linux

## Как пользоваться клиентом

После сборки клиента и запуска, он попытается автоматически подключиться к `ghost.main` (он должен быть на роутере). У вас отобразиться страница `ghost.main`.

Для базовой навигации предусмотрено колесико мыши (для скроллинга), для этого нажмите на пустое место в терминале и вы сможете использовать его для прокрутки страниц

Для перехода по другим страницам на роутере вам надо нажать `ctrl + f` и ввести домен (только английские буквы), затем нажать `enter` для перехода. Чтобы скрыть/показать адресную строку нажмите еще раз `ctrl + f`

## Инструкция по сборке

Вам потребуется установить библиотеку `liblua-5.4-dev` для работы с последними версиями. Пример, как это сделать на Ubuntu/Debian:

```sh
sudo apt update
sudo apt install liblua5.4-dev
```

Скрипт установит библиотеку [utf8proc](https://github.com/JuliaStrings/utf8proc) в ./code/lib, если у вас ее нет в глобальных путях.

#### Для сборки клиента

```sh
git clone https://github.com/striter-no/Ghostplace
cd ./Ghostplace
./code/scripts/cinstall

./code/scripts/rbuild client gcc-14 # Или любой другой GCC компилятор
./code/scripts/run client IP PORT # Запуск клиента. Укажите реальные IP и PORT роутера
```

Для создания и отправки вашего сайта на роутер

```sh
# Сделайте ваш сайт:
# Не менять имя index.ghml, styles.gss и assets/

mkdir your.site.com
cd ./your.site.com
mkdir assets 
touch index.ghml # Сюда что-нибудь напишите
touch styles.gss # и сюда
# пример сайта в ./assets/sites/ghost.main

python -m venv venv
source ./venv/bin/activate
pip install -r ./code/py/requirements.txt

# Чтобы добавить сайт (внимание на порты для добавления)
python ./code/py/cli_upload_site.py your.site.com http://router.ip:upl_port

# Далее тут будет ваш API ключ, поэтому при следующих запросах делайте так:
python ./code/py/cli_upload_site.py your.site.com http://router.ip:upl_port your-uuid4-api-key

# Для отключения venv
deactivate
```

#### Для сборки роутера

```sh
git clone https://github.com/striter-no/Ghostplace
cd ./Ghostplace

./code/scripts/sdeploy gcc-14 # Или любой другой GCC
./code/scripts/run router IP PORT # Для запуска (укажите реальные IP и PORT для привязки роутера)
```

Для того, чтобы не скачивать каждый раз по новой репозиторий для обновления есть скрипт `supdate`:

```sh
./code/scripts/supdate gcc-14 # Или любой другой GCC
./code/scripts/run router IP PORT
```

### Замечание для пользователей Termux

Для нормальной сборки вам будет необходимо установить библиотеки lua или удалить их из билд скриптов. Также для сборки `libutf8proc` вам нужен пакет `binutils` для команды `ar`. 

Если у вас проект собирает clang, а не gcc, то вам необходимо изменить многострочную структуру билд-скриптов с:

```sh
# ...
$CC -pedantic -Wall -Wextra -O3 -o "$EXEC_NAME" \
    ./code/"$EXEC_NAME".c \
    $SRC_FILES \
    -I ./code/ghpl-lib/include \
    -L ./code/ghpl-lib/lib \
    -lm -lutf8proc -lpthread -llua5.4 -ldl
# ...
```

на однострочную версию:

```sh
# ...
$CC -pedantic -Wall -Wextra -O3 -o "$EXEC_NAME" ./code/"$EXEC_NAME".c $SRC_FILES -I ./code/ghpl-lib/include -L ./code/ghpl-lib/lib -lm -lutf8proc -lpthread -llua5.4 -ldl
# ...
```