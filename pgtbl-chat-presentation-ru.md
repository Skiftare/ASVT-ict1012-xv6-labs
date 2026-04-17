# pgtbl lab: очень коротко, по-человечески

## Слайд 1. Что тут вообще делалось

Короч, лаба по факту из 3 кусков:
1. Быстрый pid без лишнего syscall (ugetpid).
2. Печать/просмотр таблиц страниц (чтоб видеть, что реально замаплено).
3. Superpages (2MB), чтоб аллокация/форк/освобождение не ломались.

Идея простая: не "меняй строчку Х", а держать в голове цепочку
alloc -> map -> free -> copy.
Если где-то выпал кусок, тесты потом добьют.

## Слайд 2. Куда шариться по файлам

По ugetpid:
- kernel/memlayout.h
- kernel/proc.h
- kernel/proc.c
- user/ulib.c

По просмотру таблиц и debug-штукам:
- kernel/vm.c
- kernel/syscall.c
- kernel/sysproc.c
- user/pgtbltest.c

По superpages:
- kernel/riscv.h
- kernel/kalloc.c
- kernel/vm.c
- user/pgtbltest.c

## Слайд 3. Мини-чек перед сдачей

1. Прогнать pgtbltest.
2. Прогнать usertests -q.
3. Прогнать grade-lab-pgtbl и глянуть итог.

Если коротко для чата: 
"Сначала сделал ugetpid, потом vmprint/интроспекцию, потом superpages. Главное не забыть согласованность map/unmap/copy."