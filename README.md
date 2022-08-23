# COMP 40 hw07: Profiling & Optimizing

Eli Intriligator and Max Behrendt

Date: Dec 7 2021

TA help acknowledgements:
    Alex, Ann-Marie, Igor, Miles, Adhiraj, Kunal, Tina

Known problems/limitations:
    We believe we have implemented all features correctly.

In this homework, we used `kcachegrind` to dynamically analyze our Universal
Machine program and find ways to optimize its performance. Our final version,
um-main.c, runs 10x faster than the original program we designed. For details
about the changes we made, see `labnotes.pdf`.

According to our `kcachegrind` results, the UArray_at routine takes up the
most time. The assembly code could definitely be improved. The machine code
for UArray_at, for example, uses many registers to store values, which
certainly slows our program down. These extra registers are used because
the UArray class is more rigorous than we want for optimal speed,
performing extra checks such as assertions and exceptions that can slow our
program down. Since the UM is allowed to fail in cases where the user
performs an invalid instruction, many of these extra checks are helpful for
debugging, but not required if we are seeking to optimize performance.

---

Time spent analyzing problems posed in the assignment:
    2 hours
Time spent solving the problems after our analysis:
    7 hours
