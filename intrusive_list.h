#pragma once

#include <iostream>

namespace intrusive {
struct default_tag;

struct list_base
{
  list_base *prev = nullptr;
  list_base *next = nullptr;

  list_base() = default;
  list_base (const list_base &); //copy
  list_base (list_base &&); //move

  list_base &operator=(list_base const&old) = default; //copy

  bool operator==(list_base const&other) const;
  bool operator!=(list_base const&other) const;

  void link(list_base &);
  //void link_prev(list_base &cur);
  void unlink();
  ~list_base();
};

template <typename Tag = default_tag>
struct list_element : public list_base {};

template <typename T, typename Tag = default_tag>
struct list {
  list_element<Tag> sentinel;

  list()
  {
    close();
  }

  void close()
  {
    sentinel.next = &sentinel;
    sentinel.prev = &sentinel;
  }

  list(list<T> &&old)
  {
    if(old.empty())
      close();
    else
    {
      list_base &end = *old.sentinel.prev;
      old.sentinel.unlink();
      old.close();
      sentinel.link(end);
    }
  }

  //move constructor default

  static list_element<Tag> &make_ref(T &old_ref)
  {
    return static_cast<list_element<Tag> &>(old_ref);
  }

  static list_element<Tag> &make_ref(list_base &old_ref)
  {
    return static_cast<list_element<Tag> &>(old_ref);
  }

  static list_element<Tag> *make_p(list_base *old_p)
  {
    return static_cast<list_element<Tag> *>(old_p);
  }

  list &operator=(list<T> &&old)
  {
    if (this == &old)
      return *this;

    if(old.empty())
      close();
    else
    {
      list_base &end = *old.sentinel.prev;
      old.sentinel.unlink();
      old.close();
      sentinel.link(end);
    }

    return *this;
  }

  void push_back(T &data)
  {
    make_ref(data).link(make_ref(*sentinel.prev));
  }

  void pop_back()
  {
    sentinel.prev->unlink();
  }

  void push_front(T &data)
  {
    data.link(sentinel);
  }

  void pop_front()
  {
    sentinel.next->unlink();
  }

  T const&front() const
  {
    return static_cast<T &>(make_ref(*sentinel.next));
  }

  T const&back() const
  {
    return static_cast<T &>(make_ref(*sentinel.prev));
  }

  T &front()
  {
    return static_cast<T &>(make_ref(*sentinel.next));
  }

  T &back()
  {
    return static_cast<T &>(make_ref(*sentinel.prev));
  }

  bool empty()
  {
    //std::cout << sentinel.next << " " << &sentinel << std::endl;
    return sentinel.next == &sentinel;
  }

  ~list()
  {
    list_base *cur = (sentinel.next);
    while(cur != &sentinel)
    {
      list_base *next = cur->next;
      cur->prev = nullptr;
      cur->next = nullptr;
      cur = next;
    }
  }


  template <typename iT, bool isConst>
  struct list_iterator{
    using difference_type   = ptrdiff_t;
    using value_type        = std::conditional_t<isConst, const iT, iT>;
    using pointer           = value_type *;
    using reference         = value_type &;
    using iterator_category = std::bidirectional_iterator_tag;

    list_iterator() = default;

    list_element<Tag> *cur = nullptr;

    //list_iterator(list_iterator<iT, true> &other) : cur(other.cur)  { }
    list_iterator(list_iterator<iT, false> const&other) : cur(other.cur)  { }

    list_iterator(list_element<Tag> *cur) : cur(cur) {}

    template<bool isConst_1>
    bool operator==(list_iterator<iT, isConst_1> const&other) const
    {
      return *cur == *other.cur;
    }

    template<bool isConst_1>
    bool operator!=(list_iterator<iT, isConst_1> const&other) const
    {
      return *cur != *other.cur;
    }

    reference operator*() const
    {
      return static_cast<iT &>(*cur);
    }

    pointer operator->() const
    {
      return static_cast<iT *>(cur);
    }

    list_iterator &operator++()
    {
      cur = make_p(cur->next);
      return *this;
    }
    list_iterator &operator--()
    {
      cur = make_p(cur->prev);
      return *this;
    }
    list_iterator operator++(int)
    {
      list_iterator res (*this);
      cur = make_p(cur->next);
      return res;
    }
    list_iterator operator--(int)
    {
      list_iterator res (*this);
      cur = make_p(cur->prev);
      return res;
    }
  };

  using iterator = list_iterator<T, false>;
  using const_iterator = list_iterator<T, true>;

  iterator begin()
  {
    return {make_p(sentinel.next)};
  }

  const_iterator begin() const
  {
    return {make_p(sentinel.next)};
  }

  iterator end()
  {
    return {&sentinel};
  }

  const_iterator end() const
  {
    return {const_cast<list_element<Tag> *>(&sentinel)};
  }

  iterator insert(const_iterator const&it, T &data)
  {
    data.link(*(it.cur->prev));
    return iterator(make_p(it.cur->prev));
  }

  iterator erase(iterator const&it)
  {
    list_base *next = it.cur->next;
    it.cur->unlink();
    return iterator(make_p(next));
  }

  void splice(const_iterator pos, list &other,
              const_iterator first, const_iterator last)
  {
    if(this == &other && last == pos)
      return;

    if(first == last)
      return;

    if(first.cur->next == last.cur)
    {
      first.cur->unlink();
      insert(pos, static_cast<T &>(*first.cur));
      return;
    }

    last->prev->next = pos.cur;       //1
    pos.cur->prev->next = first.cur;  //2
    first.cur->prev->next = last.cur; //3

    auto pos_prev = pos.cur->prev;

    pos.cur->prev = last.cur->prev;   //4
    last.cur->prev = first.cur->prev; //5
    first.cur->prev = pos_prev;       //6
  }
};
} // namespace intrusive
