# webserv: `craabe` branch

One half of webserv: config parser, routing, response building.

> **Heavily WIP.** Terminal output currently shows debug/testing output, not the final behavior.

## Build and run

```sh
make        # build
make run    # build and start
```

## Design choices

NGINX serves as reference with deliberate deviations.

**1) Token length limit**
- not emulated: NGINX limits config tokens to 4096 bytes
- this is a technical limitation, not a logic choice; no reason to fake it

**2) `\0` handling**
- not emulated: NGINX's tokenizer accepts `\0` mid-file, which breaks its logic later on
- no reason to emulate this lack of handling

**3) Default port**
- not emulated: without `listen`, NGINX defaults to `:80` (root) or `:8000`
- subject: 'config defines the ports'; so: no `listen` = config error
- also not emulated: NGINX lets `listen 127.0.0.1` default to `port 80`
- webserv throws an error instead

**4) `error_page` / `index` inheritance**
- emulated: a location defining its own `error_page`/`index` does not inherit the server's

```NGINX
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
- webserv: GET is the only default -> upload/delete (POST/DELETE) need explicit `allow_methods`
- not emulated: NGINX's `limit_except` (inverted logic, needs a block with `deny all`); webserv uses a plain list: `allow_methods GET POST;`

**6) Virtual hosts**
- supported, although only optional per subject ('if you plan to implement virtual hosts'; little extra work)
- several hostnames on one IP/port (`localhost`, `api.localhost` -> `127.0.0.1`)
- server block is picked via the `Host` header
- no match or no `Host` -> first server defined for that IP/port

**7) Location matching**
- not emulated: NGINX matches prefixes character by character -> `location /files` also catches `/filesystem/a.txt`
- webserv requires whole path segments to match

**8) Regex**
- not supported, as not required by subject ('no regex required here')
- regex expressions are accepted as plain text wherever possible (`server_name ~^www\.` is stored as a plain name)
- regex location modifiers (`~`, `~*`) and `^~` (only meaningful alongside regex locations) throw: 'location modifiers are not supported'

**9) Gaps in listen addresses**
- not emulated: NGINX resolves an empty octet to 0 (`127..0.1` -> `127.0.0.1`), which is non-standard
- same goes for nginx's behavior with missing octets (`12.0.1` becomes `12.0.0.1` as per old IPv4 standard)
- webserv throws

**10) Multiple `listen` arguments**
- are not supported -> one `listen` = one `address:port` or `port`
- anything else throws

**11) `client_max_body_size`**
- not emulated: NGINX's signed 64-bit limit (`off_t`)
- webserv accepts values up to `std::size_t`'s limit

**12) `error_page` `=code`**
- not emulated: `error_page 404 =200 /example.html` would show the client a 200, hiding the actual error
- actual error codes are useful, especially during evaluation -> `=` argument throws

**13) IPv4**
- not emulated: support for `[::]:8080`
- not required by subject, not necessary for interfacing with browser/curl (default fallback to IPv4 if IPv6 doesn't answer)

**14) conf file `http { }` wrapper**
- not emulated: NGINX's `http {}` wrapper for the server directives
- webserv accepts only server directives at top level
- NOTE: NGINX BEHAVIOR MIGHT BE IMPLEMENTED LATER DOWN THE LINE

**15) return arguments**
- all NGINX forms are handled:
  - only code: `return 404`
  - redirect: `return 301 /new`
  - response body instead of redirect: `return 404 "not here"`
  - URL alone with standard code 302: `return http://42.com`

**16) no return variables**
- webserv does not emulate NGINX's handling of variables (out of project scope)
- --> variable arguments are taken literally or rejected, depending on context
  - `return 301 https://$host$request_uri` redirects to the literal text `https://$host$request_uri`
  - `return $scheme://42.com` throws ('invalid return code')

**17) server level return**
- `return` is allowed in the server block and applies before any location is chosen
- this is supported beyond the subject to emulate NGINX behavior

**18) hostname as listen address host**
- not emulated: NGINX accepts every hostname it can resolve as listen address
- webserv accepts IPv4-addresses (X.X.X.X) and specifically 'localhost' (regardless of lower-/upper-casing)
- NOTE: NGINX BEHAVIOR MIGHT BE IMPLEMENTED LATER DOWN THE LINE

**19) location syntax**
- emulated: `location = /path` (exact match: only `/path` itself, not `/path/sub`)
- emulated: nested locations; the inner path must lie within the outer one, the inner location inherits from the outer one
- not emulated: named locations (`location @name`); they are only reachable via `try_files`, which is not required by the subject -> throws 'location modifiers are not supported'
- not emulated: NGINX accepts paths without a leading `/` (`location a`), although they can never match a request -> webserv throws
- trailing slashes are normalized while parsing (`/files/` -> `/files`, except for `/`)
  - consequence: `location /a` and `location /a/` are duplicates in webserv, while NGINX treats them as two different locations

**20) error_page redirects**
- not emulated: `error_page 404 500` --> `500` as URI, which requires redirection
- webserv simply throws if URI does not start with `/`
