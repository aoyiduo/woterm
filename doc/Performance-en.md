# WoTerm Performance Test Report

This performance test is conducted using **version v10**, mainly because versions after v10 have undergone deep kernel optimizations.  

Thanks to the encouragement and support from netizens, and also thanks to the author of **WindTerm**. Although we have never met and had no technical exchanges, WindTerm is truly an excellent software. Some of its features were referenced and implemented in **WoTerm**.

---

## 🖥 Test Environment

| Component | Configuration |
|-----------|---------------|
| **WoTerm Host** | 12th Gen i5, 48GB RAM |
| **Test Server** | 2 cores, 8GB RAM, Fedora, running on a virtual machine (NAT mode) |

---

## ⚙ Test Commands

| Command | Description |
|---------|-------------|
| `time seq 10000000` | Generates 10 million lines of records |
| `urandom_test.sh` | Generates 100MB of random data |

---

## 📦 Test Program Versions

| Program | Version |
|---------|---------|
| **WoTerm** | v10.2.1 |
| **WindTerm** | v2.7.0 |
| **PuTTY** | v0.83 |

---

## ⏱ Test Methodology

- **Test Date:** April 18, 2025  
- Each comparison command was executed at least **10 times**.  
- The **maximum and minimum values** were removed to calculate the **average performance**.

---
## Test processing time
In the terminal, issue the command time seq 10000000 to the service, then use a phone stopwatch to record the total execution time.
Although the command will print its own total duration at the end, that printed value only reflects the moment the SSH thread finishes receiving the data—far from fair.
Because the software’s data path is:

*SSH-thread receive → main-thread dispatch → terminal-thread process → main-thread display*

we therefore rely on the phone’s stopwatch instead.
<div>Command processing time<br><img src="timeseq1.png"/></div>

## Execution of urandom_test Test Comparison
In this comparison, we are looking at the throughput (in terms of records processed per second).
<div>Execution throughput<br><img src="urandom_test_speed.png"/></div>

## Memory Usage After Executing the urandom_test Command
This shows the additional memory usage after executing the urandom_test command, with the baseline memory usage (after startup) deducted.
<div>Memory usage<br><img src="urandom_test_memory.png"/></div>


*Note: The performance results are based on the configurations listed above and may vary on different hardware or environments.*
