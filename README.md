# morele.net LEDControllerServer

Tu jakiś opis

## Kompilacja

Aby skompilować należy najpierw zainstalować następujące pakiety:

* pandoc
* wiringpi
* git

```bash
sudo apt update
sudo apt install -y wiringpi git pandoc
git clone https://github.com/ravkr/lampki
cd lampki
.gen.sh
./compile.sh
```

## Uruchamianie

* Uruchomienie jako demon:
```bash
sudo ./lampki.bin
```

* Uruchomienie z opcją verbose
```bash
sudo ./lampki.bin -v
```

* Instalacja jako usługę systemową
```bash
sudo ./lampki.bin -i
```

## Zapytania

| Zapytanie | Opis | Przykładowa odpowiedź |
| - | - | - |
| / | Zwraca stronę testową systemu | *[kod HTML]* |
| /0_1_1&2_3_0&4_1_2 | Główne zapytanie zawiera prośby, które diody zapalić lub zgasić. Każda prośba oddzielona jest od poprzedniej znakiem *&*. Składnia pojedynczej prośby: *sektor_kolor_stan* | `{"good":[{"sektor":0,"kolor":1,"stan":1}, {"sektor":2,"kolor":3,"stan":0}],"bad":[{"sektor":4,"kolor":1,"stan":2,"error":3}]}` |
| /dump | Zwraca zrzut bufora diód | `00100100 11011011` |
| /reset | Resetuje wszystkie diody | `{"errorCode":0}` |
| /reset/**x** | Resetuje diody w kolorze o indeksie **x** | `{"errorCode":0}
| /show | Wyświetla w konsoli serwera układ zapalonych diód | `{"errorCode":0}` |
| /favicon.ico lub /favicon.png | Zwraca ikonkę (zapytanie domyślnie wysyłane przez przeglądarki) | *[ikona]* |
| /quit | Wyłącza serwer (lepiej nie próbujcie tego w domu) | `{"errorCode":0}` |
| /state | Zwraca ustawienia i listę zapalonych diód | `{"settings":{"sektory":50,"kolory":5},"leds":[{"sektor":25,"kolor":4},{"sektor":46,"kolor":1}]}` |
| /rand | Zapala losowe diody | `{"errorCode":0}` |
| /997 | Mrugają niebieskie i czerwone diody | `{"errorCode":0}` |
| /test | Zapala i gasi wszystkie diody | `{"errorCode":0}` |
| /welcome | Sekwencję powitalna | `{"errorCode":0}` |
| /xmas | Sekwencja bożonarodzeniowa (start/stop) | `{"errorCode":0}` |
| /readme | Zwraca readme w postaci HTML | *[kod HTML]* |
| /readme_src | Zwraca readme w postaci markdown | *[plik .md]* |

## Opcje linii poleceń

| Opcja | Działanie |
| - | - |
| -h, --help | Wyświetla pomoc |
| -v, --verbose | Włącza tryb gadatliwy |
| -i, --install | Instaluje usługę systemowego |
| -u, --uninstall | Odinstalowywuje usługę systemową |
| --kill | Wyłącza proces działający w tle |
| -p80, -p 80 | Ustawia port, na którym słuchać będzie serwer |

## Konfiguracja plikiem /etc/moreleled.conf
```
[port]
Tutaj podajemy port na którym ma słuchać domyślnie serwer
[allowedDomains]
tutaj podajemy domeny, którym zezwalamy na próby zapalania lampek
[bannedIPs]
tutaj podajemy adresy IP, które nie mogą nawiązywać połączenia z serwerem

W wypadku gdy sekcje 'allowedDomains' i 'bannedIPs' zostawimy puste, zezwalamy wszystkim na połączenia
```

## Używanie połączenia HTTPS
Jeżeli konieczne jest korzystanie z protokołu HTTPS, należy odpowiednio skonfigurować serwer WWW. Przykładowy plik konfiguracyjny na przykładzie serwera nginx
```
server {
    listen 80 default_server;
    listen [::]:80 default_server;
    listen 443 ssl default_server;
    listen [::]:443 ssl default_server;
    ssl on;
    ssl_certificate /home/pi/git/lampki/ssl/example-org.crt;
    ssl_certificate_key /home/pi/git/lampki/ssl/example-org.key;
    root /var/www/html;
    index index.html index.htm index.nginx-debian.html;
    server_name _;
    location / {
        proxy_pass http://127.0.0.1:8080$uri;
        proxy_pass_header Server;
        proxy_hide_header Date;
        proxy_set_header Accept-Encoding '';
        sub_filter '"accessible":false' '"accessible":true';
        sub_filter_once off;
        sub_filter_types application/json;
    }
}
```
