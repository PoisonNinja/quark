#pragma once

#include <lib/utility.h>

namespace libcxx
{

enum class Color {
    RED,
    BLACK,
};

template <class T>
class node
{
public:
    node()
        : left(nullptr)
        , right(nullptr)
        , parent(nullptr){};

    Color color;
    T *left, *right, *parent;
};

template <class T, node<T> T::*Link>
class rbtree
{
public:
    rbtree();
    ~rbtree();
    void insert(T& value);
    void print();
    const T* get_root();

private:
    // Internal tree operations
    void balance(T* n);
    T* insert(T* curr, T* n);
    T* rotate_left(T* root);
    T* rotate_right(T* root);
    T* uncle(T* root);
    T*& left(T* t)
    {
        return (t->*Link).left;
    }
    T*& right(T* t)
    {
        return (t->*Link).right;
    }
    T*& parent(T* t)
    {
        return (t->*Link).parent;
    }
    Color color(T* node)
    {
        if (!node) {
            return Color::BLACK;
        }
        return (node->*Link).color;
    }

    T* root;
};

template <class T, node<T> T::*Link>
rbtree<T, Link>::rbtree()
{
    this->root = nullptr;
};

template <class T, node<T> T::*Link>
rbtree<T, Link>::~rbtree()
{
    // TODO: Delete entire tree
}

template <class T, node<T> T::*Link>
T* rbtree<T, Link>::insert(T* curr, T* n)
{
    if (!curr) {
        (n->*Link).color = Color::RED;
        return n;
    }
    if (*n < *curr) {
        left(curr)         = insert(left(curr), n);
        parent(left(curr)) = curr;
    } else {
        right(curr)         = insert(right(curr), n);
        parent(right(curr)) = curr;
    }
    return curr;
}

template <class T, node<T> T::*Link>
void rbtree<T, Link>::insert(T& value)
{
    if (!this->root) {
        this->root                = &value;
        (this->root->*Link).color = Color::BLACK;
    } else {
        insert(root, &value);
        balance(&value);
    }
}

template <class T, node<T> T::*Link>
void rbtree<T, Link>::print()
{
    // TODO: In-order list
}

template <class T, node<T> T::*Link>
T* rbtree<T, Link>::rotate_right(T* pivot)
{
    T* l          = left(pivot);
    left(pivot)   = right(l);
    parent(l)     = parent(pivot);
    parent(pivot) = l;
    right(l)      = pivot;
    return l;
}

template <class T, node<T> T::*Link>
T* rbtree<T, Link>::rotate_left(T* pivot)
{
    T* r          = right(pivot);
    right(pivot)  = left(r);
    parent(r)     = parent(pivot);
    parent(pivot) = r;
    left(r)       = pivot;
    return r;
}

template <class T, node<T> T::*Link>
T* rbtree<T, Link>::uncle(T* node)
{
    // Quick sanity
    if (!parent(parent(node))) {
        return nullptr;
    }
    if (parent(node) == left(parent(parent(node)))) {
        return right(parent(parent(node)));
    } else {
        return left(parent(parent(node)));
    }
}

template <class T, node<T> T::*Link>
void rbtree<T, Link>::balance(T* inserted)
{
    T* x = inserted;
    while (x != root && color(x) != Color::BLACK &&
           color(parent(x)) != Color::BLACK) {
        // Uncle is red
        if (color(uncle(x)) == Color::RED) {
            (parent(x)->*Link).color = (uncle(x)->*Link).color = Color::BLACK;
            (parent(parent(x))->*Link).color                   = Color::RED;
            x = parent(parent(x));
        } else {
            // Hmmm
            if (parent(x) == left(parent(parent(x)))) {
                if (x == right(parent(x))) {
                    // Left-right
                    left(parent(parent(x))) = rotate_left(parent(x));
                    x                       = left(x);
                }
                rotate_right(parent(parent(x)));
                libcxx::swap((parent(x)->*Link).color,
                             (right(parent(x))->*Link).color);
                if (parent(parent(x)) == nullptr) {
                    root = parent(x);
                } else if (left(parent(parent(x))) == right(parent(x))) {
                    left(parent(parent(x))) = parent(x);
                } else {
                    right(parent(parent(x))) = parent(x);
                }
            } else if (parent(x) == right(parent(parent(x)))) {
                // Right oriented
                if (x == left(parent(x))) {
                    // Right-left
                    right(parent(parent(x))) = rotate_right(parent(x));
                    x                        = right(x);
                }
                rotate_left(parent(parent(x)));
                libcxx::swap((parent(x)->*Link).color,
                             (left(parent(x))->*Link).color);
                if (parent(parent(x)) == nullptr) {
                    root = parent(x);
                } else if (right(parent(parent(x))) == left(parent(x))) {
                    right(parent(parent(x))) = parent(x);
                } else {
                    left(parent(parent(x))) = parent(x);
                }
            }
            x = parent(x);
        }
    }
    (root->*Link).color = Color::BLACK;
}

template <class T, node<T> T::*Link>
const T* rbtree<T, Link>::get_root()
{
    return root;
}
} // namespace libcxx