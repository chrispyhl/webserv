# webserv

A non-blocking HTTP/1.1 server written in C++17, inspired by NGINX. Team project at [42 Heilbronn](https://www.42heilbronn.de).

> **Status:** in progress

## Work in this repository

This is a two-person project with a clear split of responsibilities:

| Area | Owner | Branch |
|---|---|---|
| Configuration file parser | [craabe](https://github.com/gr1es) | [`craabe`](../../tree/craabe) |
| Socket layer and `poll()` event loop | [cpohl](https://github.com/chrispyhl) | [`cpohl`](../../tree/cpohl) |
