# HTTP FFI Demo

This demo shows the current FFI surface calling an external URL.

It uses `extern "C"` declarations for libc `puts` and `system`, then lets
`curl` perform the HTTP request.

Run it manually when the machine has outbound network access:

```bash
build/vyb demos/ffi_http_get/main.vyb
```

The demo calls `http://google.com` and prints curl's response headers. It
requires `curl` and outbound network access.
