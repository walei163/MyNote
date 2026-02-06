# mmap内核驱动优化

1、根据copilot给的建议，在ADC采样驱动中，应用mmap映射内核操作时，建议采用以下代码方式：

```c
static int esam_mmap(struct file *filep, struct vm_area_struct *vma)
{
    struct cpld_spi_cmd_priv *drvdata = filep->private_data;
    unsigned long size = vma->vm_end - vma->vm_start;
    unsigned long buf_size = RECORD_BUF_SIZE_MAX; /* 或把缓冲大小保存到 drvdata->size_of_buffer */
    unsigned long pfn;

    /* 不允许映射超出缓冲区 */
    if (size > buf_size)
        return -EINVAL;

    /* 计算基 PFN：更稳妥的方式是使用 page_to_pfn(virt_to_page()) */
    pfn = page_to_pfn(virt_to_page(drvdata->mmap_start));

    /* 把用户要求的 page offset 加上去 */
    pfn += vma->vm_pgoff;

    /* 如果需要非缓存映射，可对 vma->vm_page_prot 处理：
       vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
     */

    if (remap_pfn_range(vma, vma->vm_start, pfn, size, vma->vm_page_prot))
        return -EAGAIN;

    return 0;
}
```

其中的重点是以下两点：

1）不要用virt_to_page直接计算出PFN，而应该采用以下方式：

```c
/* 计算基 PFN：更稳妥的方式是使用 page_to_pfn(virt_to_page()) */
    pfn = page_to_pfn(virt_to_page(drvdata->mmap_start));
```

使用 page_to_pfn(virt_to_page(...)) 对于 kmalloc 分配的页面更稳妥；若内存是 vmalloc 分配（或者驱动后改成 vmalloc），必须用 remap_vmalloc_range 或者用 vmalloc_to_page/ vmalloc_to_pfn。

2）要考虑应用层在应用mmap映射时加入的offset偏移量：

```c
 /* 把用户要求的 page offset 加上去 */
    pfn += vma->vm_pgoff;
```

这样会把用户请求的 page 偏移（vma->vm_pgoff）加入到基址上，满足用户在 mmap() 时用 offset 的习惯。

用户层的mmap使用方式如下：

```c
void *map = mmap(NULL, bufsize, PROT_READ, MAP_SHARED, fd, offset);
```

其中的`offset`可能为0，也可能不为0，因此内核驱动中要考虑进去。

2、为了调试方便，可以考虑在`esam_mmap`函数里增加打印信息：

```c
printk(KERN_INFO "esam_mmap: mmap_start virt=%p phys=0x%lx pfn_base=0x%lx vma->vm_pgoff=%lu vma->vm_start=0x%lx size=%lu\n",
       drvdata->mmap_start, (unsigned long)virt_to_phys(drvdata->mmap_start),
       (unsigned long)page_to_pfn(virt_to_page(drvdata->mmap_start)),
       (unsigned long)vma->vm_pgoff, (unsigned long)vma->vm_start, (unsigned long)(vma->vm_end - vma->vm_start));
```

这样可以观察用户空间传进来的参数是否设置正确，以及内核映射是否正确。