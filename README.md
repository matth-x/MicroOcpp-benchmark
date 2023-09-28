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

System:

- ESP32-WROOM-32
- CPU frequency: 160 MHz
- ESP-IDF v4.4.1

### Results

#### Flash size

Total flash usage as printed by `idf.py size-components`:

| Archive file | Binary size (B) |
| --- | ---: |
| libMicroOcpp.a | 118,181 |
| libMicroOcppMongoose.a | 2,989 |

#### Heap usage

| Application part | Heap usage (B) |
| --- | ---: |
| Idle library | 12,308 |
| Queued GetDiagnostics | 172 |
| Running transaction | 268 |
| Peak usage | 21,916 |

#### Execution time

| Application part | Execution time (microseconds) |
| --- | ---: |
| Initialization (blocking part) | 10,694 |
| Initialization (parallel part) | 10,188 |
| loop() call average | 54 |
| Execute GetDiagnostics | 5,364 |
| Authorize, start and stop transaction | 14,212 |
| Library deinitialization | 1,665 |

#### Raw output

```
Benchark results ===
ececution times in microseconds:
initalization=10694
initalization_async=10188
loop_idle=54
GetDiagnostics=5364
transaction_cycle=14212
deinitialization=1665

heap occupation in Bytes:
library idle=12308
queued GetDiagnostics=172
running transaction=268
library idle after tx=12576
maximum heap usage=21916
delta largest free block=12288
ESP base before initalization=24964
ESP base after deinitialization=25108
   slack=144
```
