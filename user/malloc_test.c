#include "kernel/types.h"
#include "user/user.h"

typedef struct ListNode {
  struct ListNode *next;
  int value;
} ListNode;

typedef struct List {
  ListNode *first_element;
  ListNode *last_element;
  int size;
} List;

List *
create_new_list()
{
  List *new_list = malloc(sizeof(List));
  if (new_list == 0) {
    printf("Не выделилась память на новый список\n");
    return 0;
  }
  new_list->first_element = 0;
  new_list->size = 0;
  new_list->last_element = 0;
  return new_list;
}

void
add_new_element(List *list, int value)
{
  if (list == 0) {
    printf("Передан нулевой указатель на список\n");
    return;
  }

  ListNode *new_node = malloc(sizeof(ListNode));
  if (new_node == 0) {
    printf("Не выделилась память на новый элемент списка\n");
    return;
  }
  new_node->value = value;
  new_node->next = 0;

  if (list->first_element == 0 && (list->size == 0)) {
    list->first_element = new_node;
    list->last_element = new_node;

  }

  else {
    list->last_element->next = new_node;
    list->last_element = new_node;
  }
  list->size += 1;
}

void
delete_element_on_index(List *list, int index)
{

  if (list == 0) {
    printf("Передан нулевой указатель на список\n");
    return;
  }

  if (list->size == 0) {
    printf("Передан пустой список\n");
    return;
  }

  if (index < 0 || index >= list->size) {
    printf("Индекс выходит за рамки списка\n");
    return;
  }
  ListNode *node_to_delete;

  if (index == 0) {
    node_to_delete = list->first_element;
    list->first_element = node_to_delete->next;

    if (list->size == 1) {
      list->last_element = 0;
    }

  } else {

    ListNode *current = list->first_element;
    for (int i = 0; i < index - 1; i++) {
      current = current->next;
    }
    node_to_delete = current->next;
    current->next = current->next->next;
    if (index == list->size - 1) {
      list->last_element = current;
    }
  }

  free(node_to_delete);
  list->size -= 1;
}

void
delete_new_list(List *list)
{
  if (list == 0) {
    printf("Передан нулевой указатель на список\n");
    return;
  }

  if (list->size == 0) {
    free(list);
    return;
  }

  while (list->size > 0)
    delete_element_on_index(list, 0);

  free(list);
}

void
main()
{
  List *new_list_1 = create_new_list();
  add_new_element(new_list_1, 1);
  add_new_element(new_list_1, 2);
  add_new_element(new_list_1, 3);
  add_new_element(new_list_1, 4);
  add_new_element(new_list_1, 5);
  add_new_element(new_list_1, 6);
  add_new_element(new_list_1, 7);
  add_new_element(new_list_1, 8);
  add_new_element(new_list_1, 9);
  add_new_element(new_list_1, 10);

  delete_element_on_index(new_list_1, 4);
  delete_element_on_index(new_list_1, 1);
  delete_element_on_index(new_list_1, 7);

  delete_new_list(new_list_1);
  new_list_1 = 0;
}