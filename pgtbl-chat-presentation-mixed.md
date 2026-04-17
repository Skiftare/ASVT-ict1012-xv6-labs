# pgtbl lab notes (chat style, mixed lang)

## Slide 1: ugetpid (English)

Idea in one line: map a tiny read-only per-proc page into user VA, so PID read does not pay syscall cost every time.

What to check by file:
- kernel/memlayout.h
What to do: define where USYSCALL lives in user virtual layout.
Why open it: this file is the memory map truth, so if address placement is wrong, everything else is wrong too.

- kernel/proc.h
What to do: add per-process pointer/field for the shared userspace metadata page.
Why open it: proc struct is where lifecycle ownership is tracked.

- kernel/proc.c
What to do: allocate that page in proc creation path, fill pid, map it with user read-only perms, then unmap/free on teardown.
Why open it: all process lifecycle hooks are here (alloc, map, free).

- user/ulib.c
What to do: implement ugetpid() as direct read from USYSCALL VA.
Why open it: this is where user-space helper funcs live.

- user/user.h
What to do: expose ugetpid() prototype.
Why open it: if declaration is missing, user tests/apps won’t compile cleanly.

## Slide 2: vmprint + debug visibility (English)

Idea in one line: build a recursive Sv39 walker so you can actually see real mappings and debug weird VM behavior fast.

What to check by file:
- kernel/vm.c
What to do: implement walker + pretty print by levels, print only valid entries.
Why open it: page-table traversal logic lives here; this is the core of the feature.

- kernel/sysproc.c
What to do: add syscall handlers used by pgtbl tests (print table, inspect pte).
Why open it: user-level test hooks enter kernel from here.

- kernel/syscall.c
What to do: wire new syscall numbers into syscall table.
Why open it: without this binding, handlers exist but are unreachable.

- kernel/syscall.h
What to do: define syscall IDs.
Why open it: number mismatches here break ABI between user and kernel.

- user/pgtbltest.c
What to do: use it as behavior spec, not just as test.
Why open it: it tells you expected output style and edge cases.

- user/user.h
What to do: expose user-visible wrappers (e.g. kpgtbl/pgpte declarations).
Why open it: missing declarations often hide integration mistakes.

## Slide 3: superpages (РУССКИЙ)

Идея в одну строку: добавить 2MB-страницы так, чтобы обычные 4KB-страницы не поломались и всё нормально жило в grow/free/fork.

По файлам:
- kernel/riscv.h
Что делать: добавить константы/макросы для SUPERPGSIZE и выравнивания.
Зачем смотреть: тут базовая математика адресов; если она кривая, дальше баги будут везде.

- kernel/kalloc.c
Что делать: сделать отдельный freelist под 2MB (superalloc/superfree), плюс аккуратно разрезать физическую память при init.
Зачем смотреть: именно тут источник физ. памяти под superpages.

- kernel/vm.c
Что делать: 
1) уметь маппить superpage-лист на нужном уровне таблицы,
2) в walk/walkaddr корректно учитывать leaf на L1,
3) в unmap уметь частично освобождать superpage через demote в обычные 4KB entries,
4) в uvmcopy на fork правильно копировать 2MB-регионы.
Зачем смотреть: это главный VM pipeline, тут решается будет ли всё работать после fork/sbrk/free.

- user/pgtbltest.c
Что делать: пройтись по superpg_fork/superpg_free и понять какие инварианты реально проверяются.
Зачем смотреть: тесты быстро показывают, где логика неполная (обычно в partial free или copy path).

## Slide 4: mini submit checklist

1. Run pgtbltest.
2. Run usertests -q.
3. Run grade-lab-pgtbl.
4. If it fails, re-check map/unmap/copy consistency first, not formatting.

One-liner for chat:
"ugetpid = shared RO page, vmprint = visibility/debug, superpages = careful lifecycle in alloc/map/unmap/fork."