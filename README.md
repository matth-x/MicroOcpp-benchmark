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
| Library initialization | 10,929 |
| loop() call maximum | 2,476 |
| loop() call average | 42 |
| Execute GetDiagnostics | 5,558 |
| Authorize, start and stop transaction | 14,615 |
| Library deinitialization | 1,631 |

#### Raw output

```
Benchark results ===
ececution times in microseconds:
initalization=10929
loop_init_max=2476
loop_idle=42
GetDiagnostics=5558
transaction_cycle=14615
deinitialization=1631

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
