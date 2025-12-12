# README

Assignment A4 in the course Computer Systems @ DIKU, UCPH

Asger Ussing - ctz435  
Sebastian Andreassen - bwj836  
Victor Panduro Andersen - xkf701

<https://github.com/Asgerbasker/A4>

---

**To use the RISC-V 32-bit emulator's disassembler:**

1. Download the full repository to your client.
2. Navigate to the *src* folder in your terminal.
3. Run the command:

    ~~~bash
    make
    ~~~

4. Then run the command:

    ~~~bash
    ./sim <path-to-.elf-file> -d
    ~~~

    Example usage: 
    ~~~bash
    ./sim ../predictor-benchmarks/fib.elf -d
    ~~~
---

**To run the RISC-V 32-bit emulator's simulator:**

1. Follow step 1-3 from above.

2. If you want to run the simulator with logging of every instruction and summary to an output file, run the command:

    ~~~bash
    ./sim <path-to-.elf-file> -l <name-of-output-file>
    ~~~

    Example usage:
    ~~~bash
    ./sim ../predictor-benchmarks/erat.elf -l log

3. If you want to run the simulator with logging of the summary to an output file, run the command:

    ~~~bash
    ./sim <path-to-.elf-file> -s <name-of-output-file>
    ~~~

    Example usage:
    ~~~bash
    ./sim ../predictor-benchmarks/erat.elf -s log

4. Without logging:

    ~~~bash 
    ./sim <path-to-.elf-file>
    ~~~

To run the simulator on the fib.elf file:

~~~bash
./sim ../predictor-benchmarks/fib.elf <optional-logging-parameters> -- <fib-num-to-calculate>
~~~

Example of usage when calculating fib(10) with logging:

~~~bash
./sim ../predictor-benchmarks/fib.elf -l log -- 10