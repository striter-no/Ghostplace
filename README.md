# GHOST PLACE

Проект по созданию клона Web в терминале. Пока что запуск и компиляция возможны только на ОС Linux

## Как пользоваться клиентом

После сборки клиента и запуска, он попытается автоматически подключиться к `ghost.main` (он должен быть на роутере). У вас отобразиться страница `ghost.main`.

Для базовой навигации предусмотрено колесико мыши (для скроллинга), для этого нажмите на пустое место в терминале и вы сможете использовать его для прокрутки страниц

Для перехода по другим страницам на роутере вам надо нажать `ctrl + f` и ввести домен (только английские буквы), затем нажать `enter` для перехода. Чтобы скрыть/показать адресную строку нажмите еще раз `ctrl + f`

## Инструкция по сборке

Скрипт установит библиотеку [utf8proc](https://github.com/JuliaStrings/utf8proc) в /usr/local/lib, если у вас ее нет. Также убедитесь, что компилятор видит путь `/usr/local/lib` 

#### Для сборки клиента

```sh
git clone https://github.com/striter-no/Ghostplace
cd ./Ghostplace
./scripts/cinstall

./scripts/rbuild client gcc-14 # Или любой другой GCC компилятор
./scripts/run client IP PORT # Запуск клиента. Укажите реальные IP и PORT роутера
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

source ./venv/bin/activate

# Чтобы добавить сайт (внимание на порты для добавления)
python ./cli_upload_site.py your.site.com http://router.ip:upl_port

# Далее тут будет ваш API ключ, поэтому при следующих запросах делайте так:
python ./cli_upload_site.py your.site.com http://router.ip:upl_port your-uuid4-api-key

# Для отключения venv
deactivate
```

#### Для сборки роутера

```sh
git clone https://github.com/striter-no/Ghostplace
cd ./Ghostplace

./scripts/sdeploy gcc-14 # Или любой другой GCC
./scripts/run router IP PORT # Для запуска (укажите реальные IP и PORT для привязки роутера)
```

Для того, чтобы не скачивать каждый раз по новой репозиторий для обновления есть скрипт `supdate`:

```sh
./scripts/supdate gcc-14 # Или любой другой GCC
./scripts/run router IP PORT
```