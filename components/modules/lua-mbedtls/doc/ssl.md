SSL/TLS
=======

```Lua
local mbedtls = require 'mbedtls'
local ssl = mbedtls.ssl
```
### ssl.newconfig(mode, [cacert], [cert], [pkey], [psk_identity], [psk])

#### Parameter Descriptions

1. **`mode`** (required)
   - **Type**: string
   - **Description**: Specifies the SSL/TLS operation mode. It can be one of the following:
     - `tls-client`: For establishing a TLS client connection.
     - `tls-server`: For establishing a TLS server connection.
     - `dtls-client`: For establishing a DTLS client connection.
     - `dtls-server`: For establishing a DTLS server connection.

2. **`cacert`** (optional)
   - **Type**: string (PEM format)
   - **Description**: If provided, a CA certificate chain is loaded from memory in PEM format, and peer verification is enabled. This parameter ensures that the client or server can validate the other party's certificate during the connection establishment.

3. **`cert`** (optional)
   - **Type**: string (PEM format)
   - **Description**: If provided, the server's certificate (peer certificate) is loaded from memory in PEM format. This parameter is required in server mode so that the server can prove its identity to the client. If not provided, a built-in self-signed certificate will be used by default in server mode.

4. **`pkey`** (optional)
   - **Type**: string (PEM format)
   - **Description**: If provided, the private key (private key) associated with the `cert` is loaded from memory in PEM format. The private key is used for encryption and decryption operations, ensuring that only the entity with the correct private key can communicate. If not provided in server mode, the system will use the default self-signed certificate without requiring an additional private key.

5. **`psk_identity`** (optional)
   - **Type**: string
   - **Description**: A string used for the pre-shared key (PSK) identity. When using PSK for authentication, this parameter specifies the identity of the client or server. This identity will be used during the connection establishment for authentication.

6. **`psk`** (optional)
   - **Type**: string (hexadecimal format)
   - **Description**: The content of the pre-shared key (PSK). This parameter is provided for PSK authentication. PSK is a lightweight authentication mechanism suitable for resource-constrained environments. It is used to encrypt communication but needs to be shared between the client and the server.

#### Return Value
- Returns a new configuration handle that can be used for subsequent SSL/TLS context creation and operations.

#### Example
##### Example1
```lua
local config = ssl.newconfig("tls-server", cacert_content, cert_content, pkey_content, "client_identity", "0123456789abcdef")
```
In this example, cacert_content is the CA certificate chain, cert_content is the server certificate, pkey_content is the associated private key, client_identity is the PSK identity, and 0123456789abcdef is the PSK content.

##### Example2
create a TLS client configuration using the ssl.newconfig function, which is typically used to establish an HTTPS connection.
```lua
create a TLS client configuration using the ssl.newconfig function, which is typically used to establish an HTTPS connection.
-- Assume cacert_content contains the CA certificate chain in PEM format
local cacert_content = [[
-----BEGIN CERTIFICATE-----
MIIDXTCCAkWgAwIBAgIJALd8R1K6zZ4xMA0GCSqGSIb3DQEBCwUAMEUxCzAJBgNV
BAYTAkFVMRMwEQYDVQQIDApTb21lLVN0YXRlMQ8wDQYDVQQKDAZTeW5vbTEfMB0G
A1UEAwwWc3lub21pbmctY2VydGlmaWNhdGUwHhcNMTkwNTE5MTY1MTQ0WhcNMjkw
NTE3MTY1MTQ0WhAAMFwxCzAJBgNVBAYTAkFVMRMwEQYDVQQIDApTb21lLVN0YXRl
MQ8wDQYDVQQKDAZTeW5vbTEfMB0GA1UEAwwWc3lub21pbmctY2VydGlmaWNhdGUw
ggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoICAQDL8G4R7c7v5XzU3g0L1Pp4
mD7FJzK2DkW8H0zZ2C1g4Uo3sYZg+zj1g0G5A1pB5G9k5+q5t9Z3pR8T7p8M9qF7
Z5Y4R9Bz2Z4j7l4Q3L4G+H6d+7P0/8q1u1gD9p5u3i9f3R+G2W3H4M8f2u2L4H4K
c1F1V2I1V4J0G2W2X3M7Z1P1Z0P3A1Z2P6D2Y0K9C6T8D1E1N4C0T3N2J2H1K3A
1Z1R2L4F7D3L2T4S1G1F3K1H0R0D8H4R0Z6F4D8L2D1M3T1P5N2K1J8C3M1O6G4E
0F0D3C2D4D6H1H0F0F1D3C0N2F4Z3G3W3Q3X2B2R0F1F0H3B0L4K1M3M1C7F5D1
0J1G7K2H8D5H1F1T8N0Y3C4D1A0C5Z1Z3Y0P2C2K5A9B9D6J2E4S1H2D3Q4F1E9E
Q4Y3Z2N3J3C8D1B6H7L8R0W4K6A0F1F1D3B2D3G2Y4U0P3D2B9F2Y1F5F3D2Y2H3
Y1N3P1N5M2M2H0O1Z8D3B1Z0C2Y3R9B0P1F1A3D2F3Q2A1F0H0Y3J1Z2G1P1K1D
3M3B2A0B6C4D1E3G1C2D2Y3D2E5B0L2F8N1M4Q4G1C5D3J5F0T1V3N1M5K0H3H9
K2D5D3C1Y3D8D8E1R7P0C3C3K2D3C1F7A0L1J1M0D3B3A1F9D1C6D5E4Z3Y1K3K
Z3A0J0D6E1F1B1P4D2K2T1Z8X0F3Z3F0H3B2D4D3J3A1A0A1B1D2E8G0D2H1E3Z
1G1F9X0F0D1B4Z1P3F1Y0C3C1D8Q8A1Z6C0L0G2Y0K1X3D3D0Y0Z4A1H1C7B1N4
-----END CERTIFICATE-----
]]

-- Assume cert_content contains the client certificate in PEM format
local cert_content = [[
-----BEGIN CERTIFICATE-----
MIIDXTCCAkWgAwIBAgIJALd8R1K6zZ4xMA0GCSqGSIb3DQEBCwUAMEUxCzAJBgNV
BAYTAkFVMRMwEQYDVQQIDApTb21lLVN0YXRlMQ8wDQYDVQQKDAZTeW5vbTEfMB0G
A1UEAwwWc3lub21pbmctY2VydGlmaWNhdGUwHhcNMTkwNTE5MTY1MTQ0WhcNMjkw
NTE3MTY1MTQ0WhAAMFwxCzAJBgNVBAYTAkFVMRMwEQYDVQQIDApTb21lLVN0YXRl
MQ8wDQYDVQQKDAZTeW5vbTEfMB0GA1UEAwwWc3lub21pbmctY2VydGlmaWNhdGUw
ggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoICAQDL8G4R7c7v5XzU3g0L1Pp4
mD7FJzK2DkW8H0zZ2C1g4Uo3sYZg+zj1g0G5A1pB5G9k5+q5t9Z3pR8T7p8M9qF7
Z5Y4R9Bz2Z4j7l4Q3L4G+H6d+7P0/8q1u1gD9p5u3i9f3R+G2W3H4M8f2u2L4H4K
c1F1V2I1V4J0G2W2X3M7Z1P1Z0P3A1Z2P6D2Y0K9C6T8D1E1N4C0T3N2J2H1K3A
1Z1R2L4F7D3L2T4S1G1F3K1H0R0D8H4R0Z6F4D8L2D1M3T1P5N2K1J8C3M1O6G4E
0F0D3C2D4D6H1H0F0F1D3C0N2F4Z3G3W3Q3X2B2R0F1F0H3B0L4K1M3M1C7F5D1
0J1G7K2H8D5H1F1T8N0Y3C4D1A0C5Z1Z3Y0P2C2K5A9B9D6J2E4S1H2D3Q4F1E9E
Q4Y3Z2N3J3C8D1B6H7L8R0W4K6A0F1F1D3B2D3G2Y4U0P3D2B9F2Y1F5F3D2Y2H3
Y1N3P1N5M2M2H0O1Z8D3B1Z0C2Y3R9B0P1F1A3D2F3Q2A1F0H0Y3J1Z2G1P1K1D
3M3B2A0B6C4D1E3G1C2D2Y3D2E5B0L2F8N1M4Q4G1C5D3J5F0T1V3N1M5K0H3H9
K2D5D3C1Y3D8D8E1R7P0C3C3K2D3C1F7A0L1J1M0D3B3A1F9D1C6D5E4Z3Y1K3K
Z3A0J0D6E1F1B1P4D2K2T1Z8X0F3Z3F0H3B2D4D3J3A1A0A1B1D2E8G0D2H1E3Z
1G1F9X0F0D1B4Z1P3F1Y0C3C1D8Q8A1Z6C0L0G2Y0K1X3D3D0Y0Z4A1H1C7B1N4
-----END CERTIFICATE-----
]]

-- Assume pkey_content contains the client private key in PEM format
local pkey_content = [[
-----BEGIN PRIVATE KEY-----
MIIEvQIBADANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQ...
-----END PRIVATE KEY-----
]]

-- Create a new TLS client configuration
local config = ssl.newconfig("tls-client", cacert_content, cert_content, pkey_content)
```
-- Example of how to use the config
-- Here you would typically create a new SSL context and perform the handshake,
-- connect to the server, send requests, etc.

-- Note: Ensure that the CA certificate, client certificate, and private key are valid and correctly formatted.

##### Example3
    Example for a PSK-Only Configuration

```lua
-- Define the PSK identity and the PSK itself
local psk_identity = "Client_identity"
local psk_key = "0123456789abcdef"  -- This should be a hexadecimal string representing the PSK

-- Create a new PSK-only TLS client configuration
local config = ssl.newconfig("tls-client", nil, nil, nil, psk_identity, psk_key)

-- Example of how to use the config
-- Here you would typically create a new SSL context and perform the handshake,
-- connect to the server, send requests, etc.

-- Note: Ensure that the server you are connecting to is configured to accept PSK authentication.
```

### ssl.newcontext(cfg, rcb, wcb, [ref])
Returns a new I/O context handle based on configuration `cfg` and I/O callbacks that have the following signatures:

* `rcb([ref,] size) -> data`
* `wcb([ref,] data) -> size`

The callbacks are called each time data needs to be transferred through the underlying communication channel.

The read callback `rcb` is allowed to receive fewer bytes than requested. If performing non-blocking I/O, an empty string must be returned when an operation would block. In this case, the calling function returns a special error string `want-read` indicating that it has to be called again once data becomes available on the underlying transport. In case of DTLS, it is also important to check if a timeout must be observed while waiting for data (see `ctx:gettimeout()`).

The write callback `wcb` is allowed to send fewer bytes than requested. It must always return the number of bytes actually sent. If performing non-blocking I/O, zero must be returned when an operation would block. In this case, the calling function returns a special error string `want-write` indicating that it has to be called again once the underlying transport is ready to send data.

Callback exceptions are propagated to the calling function, hence it is possible to generate custom callback errors via standard `error()`.


Context Methods
---------------

### ctx:gettimeout()
_DTLS only._ Returns a fractional timeout value in seconds within which the previous unfinished operation (the one that returned either `want-read` or `want-write`) must be invoked again. Returns `nil` if no timeout is active.

### ctx:setbio(rcb, wcb, [ref])
Assigns new I/O callbacks and optional reference `ref` (see `ssl.newcontext()`).

### ctx:setpeerid(peerid)
_DTLS server only._ Sets `peerid` as a peer's identity on the underlying transport, e.g. a string `address#port` for UDP. This identity is then used to verify a _ClientHello_ message as part of a DTLS handshake.

### ctx:sethostname(hostname)
 _Client only._ Sets `hostname` (or resets if `nil`) to check against the received server certificate. It sets the ServerName TLS extension too if that extension is enabled.

### ctx:handshake()
Performs a handshake and returns `true` when finished. On error, returns `nil` and the error message.

Except for DTLS server, it is not required to explicitly call this function before an I/O operation because a handshake is performed implicitly in that case.

_DTLS server only._ Prior to calling, a peer's identity must be set on the context. In order to facilitate source address verification, this function operates in a two-step fashion. The first successful return signals that a verified _ClientHello_ message has been received. The second successful return indicates that the handshake has been completed. In other words, it is required to call this function at least once to make sure that either an ongoing session can proceed (successful return) or an unverified _ClientHello_ message has been received (fatal error).

### ctx:read(size)
Attempts to read at most `size` bytes from the secure channel and returns the data actually read. On error, returns `nil` and the error message.

### ctx:write(data, [pos], [len])
Attempts to write `data` bytes to the secure channel and returns the number of bytes actually written. On error, returns `nil` and the error message. Optional `pos` marks the beginning of data (default is 1). Optional `len` marks the size of data (default is `#data - pos + 1`).

### ctx:closenotify()
Sends a _CloseNotify_ message to the peer indicating an intent to gracefully shut down the secure channel.

### ctx:reset()
Resets the context to make it suitable for a new session.


Context Errors
--------------

The following errors are non-fatal, i.e. they do not reset a context and are expected in the normal course of a secure session.

| Error               | Description                                              |
|---------------------|----------------------------------------------------------|
| `invalid operation` | Invalid arguments/configuration for the operation.       |
| `want-read`         | The underlying transport is not ready for reading.       |
| `want-write`        | The underlying transport is not ready for writing.       |
| `close-notify`      | A _CloseNotify_ message has been received from the peer. |

All other errors are fatal and implicitly reset a context thus making it suitable for a new session.


