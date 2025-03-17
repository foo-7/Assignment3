#include<linux/module.h>
#include<linux/sched/signal.h>
#include<linux/init.h>
#include<linux/kernel.h>
#include<linux/pid_namespace.h>
#include<asm/io.h>
#include<linux/mm.h> // this so that we have access to mm_struct

unsigned long virt2phys(struct mm_struct *mm, unsigned long vpage) {
    pgd_t *pgd;
    p4d_t *p4d;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;
    unsigned long physical_page_addr; //added this as it was cassing a warning. ISO C90 forbids mixed declarations and code
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
    physical_page_addr = page_to_phys(page);
    pte_unmap(pte);
    if (physical_page_addr == 70368744173568) {
        return 0;
    }
    return physical_page_addr;
}

//changed return value from void so that we can use it in proc_count and display our total pages.
unsigned long page_tables(struct task_struct *task) {
    struct vm_area_struct *vma = 0;
    unsigned long vpage;
    unsigned long total_pages = 0; //allocate our pages that need to be displayed

    /** 
            ?? ERROR ??
            mm has no mmap attribute
                -> It is unmmapped.

            Bill: we can do just do a check to see if mm is undefined
            That way  we don't need comment out the code
    */
   // we want to check if our variable is initialzed so we don't run into any unmapped atrributes
   if (task->mm == NULL){
    printk(KERN_INFO "unmapped values found. \n");
    return 0; // this means that we've reached unmapped values. and we return 0 since we're returning a long
   }

    if (task->mm && task->mm->mmap) {
        for (vma = task->mm->mmap; vma; vma = vma->vm_next) {
            for (vpage = vma->vm_start; vpage < vma->vm_end; vpage += PAGE_SIZE) {
                unsigned long physical_page_addr = virt2phys(task->mm, vpage);
                if (physical_page_addr != 0) { //check to see if the operation worked
                    total_pages += 1; //update
                }
            }   
        }
    }
    return total_pages; //return our total amount pages
}

int proc_count(void) {
    int i=0;
    struct task_struct *thechild;

    printk(KERN_INFO "PROCESS REPORT:\n");
    printk(KERN_INFO "proc_id,proc_name,total_pages\n");

    for_each_process(thechild) {
        i++;
        if (thechild->pid > 650) {
            unsigned long total_pages = page_tables(thechild);
            printk(KERN_INFO "%d, %s, %lu\n", thechild->pid, thechild->comm, total_pages);
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