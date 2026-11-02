# webserv: `craabe` branch

One half of webserv: config parser, request parsing, response generation.

> **Heavily WIP.** Terminal output currently shows debug/testing output, not the final behavior.

## Build and run

```sh
make        # build
make run    # build and start
```

## Design choices

NGINX serves as reference with deliberate derivations.

**1) Token length limit**
- not emulated: NGINX limits config tokens to 4096 bytes
- this is a technical limitation, not a logic choice; no reason to fake it

**2) `\0` handling**
- not emulated: NGINX's tokenizer accepts `\0` mid-file, which breaks its logic later on
- no reason to emulate this lack of handling

**3) Default port**
- not emulated: without `listen`, NGINX defaults to `:80` (root) or `:8000`
- subject: "config defines the ports"; so: no `listen` = config error

**4) `error_page` / `index` inheritance**
- emulated: a location defining its own `error_page`/`index` does not inherit the server's

```nginx
server {
    error_page 404 /errors/404.html;
    error_page 500 /errors/500.html;
    location /upload {
        error_page 413 /errors/too_big.html;
    }
}
```
- `/upload` has 413, but NOT 404 or 500

**5) Allowed methods**
- not emulated: NGINX restricts no methods by default
- webserv: GET is the only default -> upload/delete locations need explicit `allow_methods`

**6) Virtual hosts**
- supported, although out of scope per subject (little extra work)
- several hostnames on one IP/port (`localhost`, `api.localhost` -> `127.0.0.1`)
- server block is picked via the `Host` header
- no match or no `Host` -> first server defined for that IP/port

**7) Location matching**
- not emulated: NGINX matches prefixes character by character -> `location /files` also catches `/filesystem/a.txt`
- webserv requires whole path segments to match

**8) Regex**
- not supported, as not required by subject
- regex expressions are accepted as plain text wherever possible

**9) Gaps in listen addresses**
- not emulated: NGINX resolves an empty octet to 0 (`127..0.1` -> `127.0.0.1`), which is non-standard
- webserv throws

**10) Multiple `listen` arguments**
- not emulated: one `listen` = one `address:port` or `port`
- anything else throws

**11) `client_max_body_size`**
- not emulated: NGINX's signed 64-bit limit (`off_t`)
- webserv accepts values up to `std::size_t`'s limit

**12) `error_page` `=code`**
- not emulated: `error_page 404 =200 /example.html` would show the client a 200, hiding the actual error
- actual error codes are useful, especially during evaluation -> `=` argument throws
