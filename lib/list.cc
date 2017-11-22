#include <lib/list.h>

template <class T>
void List<T>::insert(Node<T>* node)
{
    if (!this->size) {
        this->head = node;
    } else {
        Node<T>* current = this->head;
        while (current->next) {
            current = current->next;
        }
        current->next = node;
    }
    this->size++;
}
