# xv6 pgtbl lab - practical notes

## Slide 1 - ugetpid, no fluff

Goal:
- Remove syscall overhead for frequent PID reads.

USYSCALL meaning:
- USYSCALL is a fixed user virtual address.
- Kernel maps one read-only page there per process.
- That page stores tiny shared data (`struct usyscall`), mainly pid.

What to do by file:
- kernel/memlayout.h
Define USYSCALL address and struct usyscall layout.

- kernel/proc.h
Add field in process struct to own that page pointer.

- kernel/proc.c
Allocate page on proc setup, write pid, map with user read-only flags, then unmap and free in teardown.

- user/ulib.c
Implement ugetpid() as direct memory read from USYSCALL.

- user/user.h
Add prototype so user programs compile.

Success check:
- getpid() equals ugetpid() in pgtbltest parent/child loops.

## Slide 2 - vmprint and pte introspection

Goal:
- See actual mappings when debugging; stop guessing.

What to do by file:
- kernel/vm.c
Implement recursive Sv39 walk printer and keep level-aware behavior for mapping lookup.

- kernel/sysproc.c
Add syscall handlers used by tests (print page table, query pte).

- kernel/syscall.h
Reserve syscall IDs.

- kernel/syscall.c
Wire handlers into dispatch table.

- user/user.h
Expose wrappers for kpgtbl and pgpte.

- user/pgtbltest.c
Use as behavior spec for expected output and checks.

Success check:
- print_kpgtbl test passes with expected tree output.

## Slide 3 - superpages (РУССКИЙ)

Цель:
- Добавить 2MB superpages и не сломать обычный 4KB путь.

Что делать по файлам:
- kernel/riscv.h
Ввести SUPERPGSIZE и макросы выравнивания.

- kernel/kalloc.c
Сделать отдельный 2MB аллокатор (superalloc/superfree) и правильно выделить диапазон физпамяти.

- kernel/vm.c
Собрать целый жизненный цикл:
1. uvmalloc: если адрес/размер подходят, маппить superpage.
2. walkaddr: корректно считать оффсет для leaf на уровне L1.
3. uvmunmap: уметь частично освобождать superpage (демоут в L0).
4. uvmcopy: на fork корректно копировать 2MB регионы.

- user/pgtbltest.c
Проверить superpg_fork и superpg_free как главный критерий корректности.

Критерий успеха:
- Оба superpage теста проходят и обычные тесты не деградируют.

## Slide 4 - submit flow

1. Run pgtbltest.
2. Run usertests -q.
3. Run grade-lab-pgtbl.

Chat one-liner:
- ugetpid = USYSCALL read-only shared page.
- vmprint = real mapping visibility.
- superpages = consistent alloc/map/unmap/fork behavior.
