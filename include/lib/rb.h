#pragma once

#ifdef USERSPACE
#include <cassert>
#include <functional>
#include <utility>
#define libcxx std
#else
#include <lib/functional.h>
#include <lib/utility.h>
#endif

#ifndef USERSPACE
namespace libcxx
{
#endif
enum class Color {
    RED,
    BLACK,
};

template <class T>
struct rbnode {
    rbnode()
        : color(Color::BLACK)
        , left(nullptr)
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
    T* remove(T& value);
    T* remove(T& value, rb_callback_t callback);
    void print();

    // Functions for traversal
    T* get_root();
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

private:
    // Internal tree operations
    void balance(T* n, rb_callback_t callback);
    void traverse(T* start, rb_callback_t callback);
    void post_remove(T* n, rb_callback_t callback);
    T* calculate_prev(T* val);
    T* insert(T* curr, T* n);
    T* remove(T* curr, rb_callback_t callback);
    void rotate_left(T* root);
    void rotate_right(T* root);
    T* uncle(T* root);
    Color color(T* rbnode)
    {
        if (!rbnode) {
            return Color::BLACK;
        }
        return (rbnode->*Link).color;
    }
    void paint(T* rbnode, Color color)
    {
        if (!rbnode) {
            return;
        }
        (rbnode->*Link).color = color;
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
        paint(n, Color::RED);
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
        this->root = &value;
        paint(this->root, Color::BLACK);
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
}

template <class T, rbnode<T> T::*Link>
void rbtree<T, Link>::insert(T& value)
{
    // Empty
    rb_callback_t callback;
    return this->insert(value, callback);
}

template <class T, rbnode<T> T::*Link>
T* rbtree<T, Link>::remove(T& value, rb_callback_t callback)
{
    return remove(&value, callback);
}

template <class T, rbnode<T> T::*Link>
T* rbtree<T, Link>::remove(T& value)
{
    // Empty
    rb_callback_t callback;
    return remove(value, callback);
}

template <class T, rbnode<T> T::*Link>
T* rbtree<T, Link>::remove(T* value, rb_callback_t callback)
{
    T* node = value;
    if (left(node) && right(node)) {
        auto node_orig = node->*Link;
        *node          = *next(node);
        node->*Link    = node_orig;
        node           = next(node);
    }
    T* pullup = left(node) ? left(node) : right(node);
    if (pullup) {
        if (node == root) {
            this->root = pullup;
        } else {
            if (left(parent(node)) == node) {
                left(parent(node)) = pullup;
            } else {
                right(parent(node)) = pullup;
            }
            parent(pullup) = parent(node);
            if (color(node) == Color::BLACK) {
                post_remove(pullup, callback);
            }
        }
    } else if (node == root) {
        root = nullptr;
    } else {
        if (color(node) == Color::BLACK) {
            post_remove(node, callback);
        }
        // Remove from parent
        if (left(parent(node)) == node) {
            left(parent(node)) = nullptr;
        } else {
            right(parent(node)) = nullptr;
        }
    }
    // Fix up the previous and next nodes
    if (prev(node)) {
        next(prev(node)) = next(node);
    }
    if (next(node)) {
        prev(next(node)) = prev(node);
    }
    if (callback) {
        traverse(value, callback);
        for (T* curr = node; curr != nullptr; curr = parent(curr)) {
            callback(curr);
        }
    }
    return node;
}

template <class T, rbnode<T> T::*Link>
void rbtree<T, Link>::post_remove(T* n, rb_callback_t callback)
{
    while (n != root && color(n) == Color::BLACK) {
        if (n == left(parent(n))) {
            T* sibling = right(parent(n));
            if (color(sibling) == Color::RED) {
                paint(sibling, Color::BLACK);
                paint(parent(n), Color::RED);
                rotate_left(parent(n));
                sibling = right(parent(n));
            }
            if (color(left(sibling)) == Color::BLACK &&
                color(right(sibling)) == Color::BLACK) {
                paint(sibling, Color::RED);
                n = parent(n);
            } else {
                if (color(right(sibling)) == Color::BLACK) {
                    paint(left(sibling), Color::BLACK);
                    paint(sibling, Color::RED);
                    rotate_right(sibling);
                    sibling = right(parent(n));
                }
                paint(sibling, color(parent(n)));
                paint(parent(n), Color::BLACK);
                paint(right(sibling), Color::BLACK);
                rotate_left(parent(n));
                n = root;
            }
        } else {
            T* sibling = left(parent(n));
            if (color(sibling) == Color::RED) {
                paint(sibling, Color::BLACK);
                paint(parent(n), Color::RED);
                rotate_right(parent(n));
                sibling = left(parent(n));
            }
            if (color(left(sibling)) == Color::BLACK &&
                color(right(sibling)) == Color::BLACK) {
                paint(sibling, Color::RED);
                n = parent(n);
            } else {
                if (color(left(sibling)) == Color::BLACK) {
                    paint(right(sibling), Color::BLACK);
                    paint(sibling, Color::RED);
                    rotate_left(sibling);
                    sibling = left(parent(n));
                }
                paint(sibling, color(parent(n)));
                paint(parent(n), Color::BLACK);
                paint(left(sibling), Color::BLACK);
                rotate_right(parent(n));
                n = root;
            }
        }
    }
    paint(n, Color::BLACK);
}

template <class T, rbnode<T> T::*Link>
void rbtree<T, Link>::print()
{
    // TODO: In-order list
}

template <class T, rbnode<T> T::*Link>
void rbtree<T, Link>::rotate_right(T* pivot)
{
    T* l = left(pivot);
    // Swap the parent pointers to this
    if (parent(pivot)) {
        if (pivot == left(parent(pivot))) {
            left(parent(pivot)) = l;
        } else {
            right(parent(pivot)) = l;
        }
    } else {
        root = l;
    }
    left(pivot)   = right(l);
    parent(l)     = parent(pivot);
    parent(pivot) = l;
    if (right(l)) {
        parent(right(l)) = pivot;
    }
    right(l) = pivot;
}

template <class T, rbnode<T> T::*Link>
void rbtree<T, Link>::rotate_left(T* pivot)
{
    T* r = right(pivot);
    // Swap the parent pointers to this
    if (parent(pivot)) {
        if (pivot == left(parent(pivot))) {
            left(parent(pivot)) = r;
        } else {
            right(parent(pivot)) = r;
        }
    } else {
        root = r;
    }
    right(pivot)  = left(r);
    parent(r)     = parent(pivot);
    parent(pivot) = r;
    if (left(r)) {
        parent(left(r)) = pivot;
    }
    left(r) = pivot;
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
    while (x != root && color(parent(x)) != Color::BLACK) {
        // Uncle is red
        if (color(uncle(x)) == Color::RED) {
            // Simple, just recolor the uncle, parent, and grandparent
            paint(parent(x), Color::BLACK);
            paint(uncle(x), Color::BLACK);
            paint(parent(parent(x)), Color::RED);
            x = parent(parent(x));
            // No need to recompute anything (TODO: Is this correct?)
        } else {
            // Hmmm
            if (parent(x) == left(parent(parent(x)))) {
                if (x == right(parent(x))) {
                    // Left-right
                    rotate_left(parent(x));
                    x = left(x);
                }
                rotate_right(parent(parent(x)));
                libcxx::swap((parent(x)->*Link).color,
                             (right(parent(x))->*Link).color);
            } else if (parent(x) == right(parent(parent(x)))) {
                // Right oriented
                if (x == left(parent(x))) {
                    // Right-left
                    rotate_right(parent(x));
                    x = right(x);
                }
                rotate_left(parent(parent(x)));
                libcxx::swap((parent(x)->*Link).color,
                             (left(parent(x))->*Link).color);
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
    paint(root, Color::BLACK);
}

template <class T, rbnode<T> T::*Link>
T* rbtree<T, Link>::get_root()
{
    return root;
}
#ifndef USERSPACE
}
#endif
