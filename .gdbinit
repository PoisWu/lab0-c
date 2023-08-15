break merge_sort
break mergeTwoLists



define p_generic_list
  set var $n = $arg0->next
  while $n!=$arg0
    print $n
    print (element_t *) $n - 8
    print (('element_t' *)($n - 8))->value
    set var $n = $n->next
  end
end

document p_generic_list
        p_generic_list LIST_HEAD_POINTER
        Print all the fields of the nodes in the linked list pointed to by
        LIST_HEAD_POINTER. Assumes there is a next field in the struct.
end


