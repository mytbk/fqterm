# HTTP regression tests

Configure with `-DFQTERM_BUILD_TESTS=ON`, build, then run `ctest --output-on-failure`.
The tests use loopback HTTP/HTTPS servers and do not contact a real BBS.
The TLS certificate and private key in this directory are public test fixtures
for an intentionally untrusted server. They must never be used for production.
The timeout test waits for the production 30 second inactivity timeout.

Qt platform/image plugins and its OpenSSL runtime must be on the test process's
search path. Results are written to `build/tests/http-test-results.txt`.
