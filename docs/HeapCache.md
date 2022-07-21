# HeapCache堆内存分配器

heap cache：小，中，大。

大块：就直接从page cache中分配，也是直接释放到page cache中去。

中块：从page cache分配span,然后返回span对象，释放时添加到span链表，如果链表太长，就直接释放到 page cache 中。

小块：从span中分配，将span拆分成小块，然后分配对象。释放时先释放到对象链表中，满足条 件的话，然后再把这个链表中的对象释放到span中（小对象必须在同一个span中）。
