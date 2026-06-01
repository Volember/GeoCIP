# GeoCIP

Lightweight single-header C library for IP geolocation.

GeoCIP allows you to retrieve information about your public IP address including:

* Public IP
* Country
* Country code
* City
* ISP
* Timezone

Designed to be:

* Single-header
* Cross-platform (Windows / Linux / Unix)
* Dependency-free
* Lightweight
* Easy to integrate

---

## Features

✔ Single header library (`geocip.h`)
✔ Pure C (C89/C99 compatible)
✔ Windows support via WinSock
✔ Linux / Unix support via POSIX sockets
✔ No external libraries required
✔ Simple API

---

## Installation

Just copy `geocip.h` into your project.

In **one** source file:

```c
#define IPLOCATE_IMPLEMENTATION
#include "geocip.h"
```

In all other files:

```c
#include "geocip.h"
```

---

## API

### Initialize networking

```c
int iplocate_init(void);
```

Returns:

* `1` → success
* `0` → initialization failed

---

### Cleanup networking

```c
void iplocate_cleanup(void);
```

Required on Windows.

---

### Retrieve IP information

```c
ip_info_t iplocate_get_info(void);
```

Returns:

```c
typedef struct
{
    char country[128];
    char countryCode[16];
    char city[128];
    char timezone[128];
    char isp[128];
    char ip[64];
    int success;
} ip_info_t;
```

---

## Example

```c
#define IPLOCATE_IMPLEMENTATION
#include "geocip.h"

#include <stdio.h>

int main()
{
    if (!iplocate_init())
    {
        printf("Initialization failed\n");
        return 1;
    }

    ip_info_t info = iplocate_get_info();

    if (info.success)
    {
        printf("IP: %s\n", info.ip);
        printf("Country: %s (%s)\n",
               info.country,
               info.countryCode);

        printf("City: %s\n", info.city);
        printf("Timezone: %s\n", info.timezone);
        printf("ISP: %s\n", info.isp);
    }
    else
    {
        printf("Failed to retrieve IP information\n");
    }

    iplocate_cleanup();

    return 0;
}
```

---

## Example Output

```text
IP: 185.xxx.xxx.xxx
Country: Italy (IT)
City: Rome
Timezone: Europe/Rome
ISP: Fastweb
```

---

## How it works

GeoCIP performs:

1. TCP connection to IP geolocation service
2. HTTP request
3. Response parsing
4. Returns parsed data through a simple C structure

---

## Notes

Currently GeoCIP uses:

ip-api.com

for IP geolocation data.

Free API limitations may apply.

---

## License

MIT License
