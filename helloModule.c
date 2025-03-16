#include<linux/module.h>
#include<linux/sched/signal.h>
#include<linux/init.h>
#include<linux/kernel.h>
#include<linux/pid_namespace.h>
#include<asm/io.h>
#include<linux/mm.h> // this so that we have access to mm_struct and so our functions that need access to mm don't run into errors

unsigned long virt2phys(struct mm_struct *mm, unsigned long vpage) {
    pgd_t *pgd;
    p4d_t *p4d;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;
    struct page *page;
    pgd = pgd_offset(mm, vpage);
    if (pgd_none(*pgd) || pgd_bad(*pgd)) {
        return 0;
    }
    p4d = p4d_offset(pgd, vpage);
    if (p4d_none(*p4d) || p4d_bad(*p4d)) {
        return 0;
    }
    pud = pud_offset(p4d, vpage);
    if (pud_none(*pud) || pud_bad(*pud)) {
        return 0;
    }
    pmd = pmd_offset(pud, vpage);
    if (pmd_none(*pmd) || pmd_bad(*pmd)) {
        return 0;
    }
    if (!(pte = pte_offset_map(pmd, vpage))) {
        return 0;
    }
    if (!(page = pte_page(*pte))) {
        return 0;
    }
    unsigned long physical_page_addr = page_to_phys(page);
    pte_unmap(pte);
    if (physical_page_addr == 70368744173568) {
        return 0;
    }
    return physical_page_addr;
}

void page_tables(struct task_struct *task) {
    struct vm_area_struct *vma = 0;
    unsigned long vpage;

    /** 
            ?? ERROR ??
            mm has no mmap attribute
                -> It is unmmapped.

            Bill: we can do just do a check to see if mm is undefined
            That way  we don't need comment out the code
    */
   // we want to check if our variables are initialzed so we don't run into any unmapped atrributes
   if (task->mm == NULL || task->mm->mmap == NULL){
    printf("unmapped values found \n")
    return; // this means that we've reached unmapped values
   }

    if (task->mm && task->mm->mmap) {
        for (vma = task->mm->mmap; vma; vma = vma->vm_next) {
            for (vpage = vma->vm_start; vpage < vma->vm_end; vpage += PAGE_SIZE) {
                unsigned long physical_page_addr = virt2phys(task->mm, vpage);
            }
        }
    }

    /**
        Might run, but check if u can have kernel ver 5 run above.
        (ERRORS EXIST HERE TOO FOR THE MAPPING)
        
    */
    // if (task->mm) {
    //     for (vma = task->mm->mmap; vma; vma = vma->vm_next) {
    //         for (vpage = vma->vm_start; vpage < vma->vm_end; vpage += PAGE_SIZE) {
    //             unsigned long physical_page_addr = virt2phys(task->mm, vpage);
    //         }
    //     }
    // }

    //isn't this the exact same?
}

int proc_count(void) {
    int i=0;
    struct task_struct *thechild;
    for_each_process(thechild) {
        i++;
        if (thechild->pid > 650) {
            page_tables(thechild);
        }
    }
    return i;
}

int proc_init (void) {
  printk(KERN_INFO "helloModule: kernel module initialized\n");
  printk(KERN_INFO "There are %d running processes. \n", proc_count());
  return 0;
}

void proc_cleanup(void) {
  printk(KERN_INFO "helloModule: performing cleanup of module\n");
}

MODULE_LICENSE("GPL");
module_init(proc_init);
module_exit(proc_cleanup);