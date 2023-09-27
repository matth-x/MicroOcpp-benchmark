# MicroOcpp benchmark (esp-idf)

Compiler options:

- Compiled with -Os
- CONFIG_COMPILER_OPTIMIZATION_ASSERTION_LEVEL set to Silent
- MicroOcpp log disabled (-DMOCPP_DBG_LEVEL=MOCPP_DL_NONE)
- Configured for two physical connectors (-DMOCPP_NUMCONNECTORS=3)

General setup:

- no Wi-Fi connection
- although the Mongoose WebSocket adapter is initialized, all traffic is routed though a loopback connection
- no filesystem access

### Results

#### Flash size

Total flash usage as printed by `idf.py size-components`:

| Archive file | Binary size (B) |
| --- | ---: |
| libMicroOcpp.a | 116,970 |
| libMicroOcppMongoose.a | 2,973 |

#### Heap usage

| Application part | Heap usage (B) |
| --- | ---: |
| Idle library | 12,308 |
| Queued GetDiagnostics | 172 |
| Running transaction | 268 |


#### Execution time

| Application part | Execution time (microseconds) |
| --- | ---: |
| Library initialization | 10,715 |
| loop() call maximum | 2,412 |
| loop() call average | 30 |
| Execute GetDiagnostics | 5,480 |
| Authorize, start and stop transaction | 14,452 |
| Library deinitialization | 1,541 |
