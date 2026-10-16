# Assignment 1: Basic Matrix Multiplication in Python, Java, and C

**Author:** Hugo Barreto Suárez  
**Course:** Big Data (3º Grado en Ciencia e Ingeniería de Datos - ULPGC)  
**Academic Year:** 2026–2027  
**Report:** [Assignment01_BarretoSuarez_Report.pdf](./Assignment01_BarretoSuarez_Report.pdf)

This repository contains the source code, benchmark scripts, raw measurements, generated figures, and the final research report for **Assignment 1**, which compares the execution time and memory usage of the basic $O(n^3)$ triple-loop dense matrix multiplication algorithm across Python, Java, and C.

## 1. Repository Structure

```text
.
├── src/
│   ├── matmul.c                          # C implementation (1D contiguous row-major array)
│   ├── MatMul.java                       # Java implementation (2D double[][] array)
│   └── matmul.py                         # Python implementation (2D list of lists)
├── notebooks/
│   └── experiments_and_analysis.ipynb    # Automated build, validation, benchmark & plotting
├── data/
│   ├── raw_measurements.csv              # Raw output for all 5 repetitions per size
│   ├── summary_results.csv               # Aggregated medians, IQRs, and checksums
│   └── execution_limits.csv              # Execution status (COMPLETED, TIMEOUT, SKIPPED)
├── figures/
│   ├── runtime_comparison.png            # Log-log plot of median kernel execution time
│   └── memory_comparison.png             # Plot of median memory usage vs. matrix size
├── Assignment01_BarretoSuarez_Report.pdf # Final research report (PDF)
└── README.md                             # Project documentation and instructions
```

*(Note: Make sure the source files are placed inside the `src/` folder before running the notebook, or adjust the paths accordingly).*

## 2. Environment and Dependencies

The experiments reported in the PDF were executed on the following setup:

* **OS:** Windows 11 (64-bit)
* **CPU:** Intel Core Ultra 7 155H (16 physical cores / 22 logical processors)
* **RAM:** 15.37 GB
* **C Compiler:** GCC 16.2
* **Java:** OpenJDK 17.0.20.1
* **Python:** CPython 3.14.6 (Anaconda environment)

### Python Packages Required

To run the Python script and the analysis notebook, install the required libraries:

```bash
pip install numpy pandas matplotlib psutil jupyter
```

## 3. Input Generation (No External Files Needed)

To avoid disk I/O overhead and differences between standard library random generators, all three implementations use the exact same built-in 64-bit Linear Congruential Generator (LCG):

$$
\text{state} = (\text{state} \times 6364136223846793005 + 1442695040888963407) \pmod{2^{64}}
$$

Given the same `seed` parameter, C, Java, and Python generate the exact same $n \times n$ matrices $A$ (`seed`) and $B$ (`seed + 1`) with 64-bit floating-point values in $[0, 1)$.

## 4. Build and Run Instructions

All three programs accept the exact same command-line arguments:

```text
<mode> <n> <warmup_runs> <measured_reps> <seed>
```

Where `<mode>` can be:

* `identity`: checks $A \times I = A$
* `zero`: checks $A \times 0 = 0$
* `dump`: prints the flattened result matrix $C$ to compare against NumPy
* `benchmark`: runs the warm-ups and measured repetitions, printing CSV lines: `language,n,numeric_type,repetition,time_s,memory_kb,checksum,threads`

### A. Compile C and Java

**On Windows (PowerShell / CMD):**

```bash
gcc -O2 src/matmul.c -o src/matmul_c.exe -lpsapi
javac src/MatMul.java
```

**On Linux / macOS:**

```bash
gcc -O2 src/matmul.c -o src/matmul_c -lm
javac src/MatMul.java
```

### B. Run Individual Benchmarks Manually

Example for $n = 200$, $2$ warm-up runs, $5$ measured repetitions, and `seed = 12345`:

* **C:**
  ```bash
  ./src/matmul_c.exe benchmark 200 2 5 12345
  ```
* **Java:**
  ```bash
  java -cp src MatMul benchmark 200 2 5 12345
  ```
* **Python:**
  ```bash
  python src/matmul.py benchmark 200 2 5 12345
  ```

## 5. Reproducing the Full Study and Plots

The easiest way to reproduce the entire experiment (compilation, correctness validation at $n=16$ against `numpy.matmul`, benchmarking across $n \in \{10, 50, 100, 200, 300, 500, 800, 1000\}$ with the 120s timeout budget, CSV generation, and plots) is by running the Jupyter Notebook:

```bash
jupyter notebook notebooks/experiments_and_analysis.ipynb
```

Execute all cells in order:

1. **Cell 1:** Compiles the C and Java code and prints the hardware/software environment table.
2. **Cell 2:** Validates correctness (`identity`, `zero`, and `dump` compared with NumPy using `rtol=1e-12`, `atol=1e-12`).
3. **Cell 3:** Runs the benchmarks for all matrix sizes and saves `data/raw_measurements.csv` and `data/execution_limits.csv`.
4. **Cell 4:** Computes medians and IQRs, saves `data/summary_results.csv`, and generates the plots in `figures/`.