#pragma once

#include <lib/functional.h>
#include <lib/utility.h>

namespace libcxx
{
enum class Color {
    RED,
    BLACK,
};

template <class T>
class rbnode
{
public:
    rbnode()
        : left(nullptr)
        , right(nullptr)
        , parent(nullptr)
        , prev(nullptr)
        , next(nullptr){};

    Color color;
    T *left, *right, *parent, *prev, *next;
};

template <class T, rbnode<T> T::*Link>
class rbtree
{
public:
    typedef libcxx::function<void(T*)> rb_callback_t;
    rbtree();
    ~rbtree();
    void insert(T& value);
    void insert(T& value, rb_callback_t callback);
    void print();

    // Functions for traversal
    const T* get_root();
    const T* left(const T* t)
    {
        return (t->*Link).left;
    }
    const T* right(const T* t)
    {
        return (t->*Link).right;
    }
    const T* parent(const T* t)
    {
        return (t->*Link).parent;
    }
    const T* prev(const T* t)
    {
        return (t->*Link).prev;
    }
    const T* next(const T* t)
    {
        return (t->*Link).next;
    }

private:
    // Internal tree operations
    void balance(T* n, rb_callback_t callback);
    void traverse(T* start, rb_callback_t callback);
    T* calculate_prev(T* val);
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
    T*& prev(T* t)
    {
        return (t->*Link).prev;
    }
    T*& next(T* t)
    {
        return (t->*Link).next;
    }
    Color color(T* rbnode)
    {
        if (!rbnode) {
            return Color::BLACK;
        }
        return (rbnode->*Link).color;
    }

    T* root;
};

template <class T, rbnode<T> T::*Link>
rbtree<T, Link>::rbtree()
{
    this->root = nullptr;
};

template <class T, rbnode<T> T::*Link>
rbtree<T, Link>::~rbtree()
{
    // TODO: Delete entire tree
}

template <class T, rbnode<T> T::*Link>
T* rbtree<T, Link>::calculate_prev(T* val)
{
    T* curr = this->root;
    T* prev = nullptr;
    while (curr) {
        if (*curr > *val) {
            curr = left(curr);
        } else {
            prev = curr;
            curr = right(curr);
        }
    }
    return prev;
}

template <class T, rbnode<T> T::*Link>
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

template <class T, rbnode<T> T::*Link>
void rbtree<T, Link>::insert(T& value, rb_callback_t callback)
{
    prev(&value) = this->calculate_prev(&value);
    T* curr      = &value;
    if (prev(curr)) {
        T* tnext    = next(prev(curr));
        T* tprev    = prev(curr);
        next(tprev) = curr;
        next(curr)  = tnext;
    } else {
        if (parent(curr)) {
            next(curr) = parent(curr);
        } else {
            next(curr) = nullptr;
        }
    }
    if (next(curr)) {
        prev(next(curr)) = curr;
    }
    if (!this->root) {
        this->root                = &value;
        (this->root->*Link).color = Color::BLACK;
        if (callback)
            callback(root);
    } else {
        insert(root, &value);
        if (callback) {
            for (T* curr = &value; curr != nullptr; curr = parent(curr)) {
                callback(curr);
            }
        }
        balance(&value, callback);
    }
} // namespace libcxx

template <class T, rbnode<T> T::*Link>
void rbtree<T, Link>::insert(T& value)
{
    // Empty
    rb_callback_t callback;
    return this->insert(value, callback);
}

template <class T, rbnode<T> T::*Link>
void rbtree<T, Link>::print()
{
    // TODO: In-order list
}

template <class T, rbnode<T> T::*Link>
T* rbtree<T, Link>::rotate_right(T* pivot)
{
    T* l          = left(pivot);
    left(pivot)   = right(l);
    parent(l)     = parent(pivot);
    parent(pivot) = l;
    right(l)      = pivot;
    return l;
}

template <class T, rbnode<T> T::*Link>
T* rbtree<T, Link>::rotate_left(T* pivot)
{
    T* r          = right(pivot);
    right(pivot)  = left(r);
    parent(r)     = parent(pivot);
    parent(pivot) = r;
    left(r)       = pivot;
    return r;
}

template <class T, rbnode<T> T::*Link>
T* rbtree<T, Link>::uncle(T* rbnode)
{
    // Quick sanity
    if (!parent(parent(rbnode))) {
        return nullptr;
    }
    if (parent(rbnode) == left(parent(parent(rbnode)))) {
        return right(parent(parent(rbnode)));
    } else {
        return left(parent(parent(rbnode)));
    }
}

template <class T, rbnode<T> T::*Link>
void rbtree<T, Link>::traverse(T* curr, rb_callback_t callback)
{
    if (!curr) {
        return;
    }
    if (this->left(curr)) {
        traverse(this->left(curr), callback);
    }
    if (this->right(curr)) {
        traverse(this->right(curr), callback);
    }
    callback(curr);
}

template <class T, rbnode<T> T::*Link>
void rbtree<T, Link>::balance(T* inserted, rb_callback_t callback)
{
    T* x = inserted;
    while (x != root && color(x) != Color::BLACK &&
           color(parent(x)) != Color::BLACK) {
        // Uncle is red
        if (color(uncle(x)) == Color::RED) {
            // Simple, just recolor the uncle, parent, and grandparent
            (parent(x)->*Link).color = (uncle(x)->*Link).color = Color::BLACK;
            (parent(parent(x))->*Link).color                   = Color::RED;
            x = parent(parent(x));
            // No need to recompute anything (TODO: Is this correct?)
        } else {
            // Hmmm
            if (parent(x) == left(parent(parent(x)))) {
                if (x == right(parent(x))) {
                    // Left-right
                    auto gp  = parent(parent(x));
                    left(gp) = rotate_left(parent(x));
                    x        = left(x);
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
                    auto gp   = parent(parent(x));
                    right(gp) = rotate_right(parent(x));
                    x         = right(x);
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
            if (callback) {
                traverse(x, callback);
                for (T* curr = x; curr != nullptr; curr = parent(curr)) {
                    callback(curr);
                }
            }
        }
    }
    (root->*Link).color = Color::BLACK;
}

template <class T, rbnode<T> T::*Link>
const T* rbtree<T, Link>::get_root()
{
    return root;
}
} // namespace libcxx